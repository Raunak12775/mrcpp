/*
 * MRCPP, a numerical library based on multiresolution analysis and
 * the multiwavelet basis which provide low-scaling algorithms as well as
 * rigorous error control in numerical computations.
 * Copyright (C) 2021 Stig Rune Jensen, Jonas Juselius, Luca Frediani and contributors.
 *
 * This file is part of MRCPP.
 *
 * MRCPP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MRCPP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MRCPP.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For information on the complete list of contributors to MRCPP, see:
 * <https://mrcpp.readthedocs.io/>
 */

#include "MWTree.h"

#include "MWNode.h"
#include "MultiResolutionAnalysis.h"
#include "NodeAllocator.h"
#include "utils/Bank.h"
#include "NodeIndex.h"
#include "TreeIterator.h"
#include "fstream" // rafa
#include "utils/Printer.h"
#include "utils/math_utils.h"
#include "utils/periodic_utils.h"
#include "utils/tree_utils.h"
// #include "utils/io_utils.h" // rafa
using namespace Eigen;

namespace mrcpp {

/** @brief MWTree constructor.
 *
 * @param[in] mra: the multiresolution analysis object
 * @param[in] n: the name of the tree (only for printing purposes)
 *
 * @details Creates an empty tree object, containing only the set of
 * root nodes. The information for the root node configuration to use
 * is in the mra object which is passed to the constructor.
 */
template <int D>
MWTree<D>::MWTree(const MultiResolutionAnalysis<D> &mra, const std::string &n)
        : MRA(mra)
        , order(mra.getOrder()) /// polynomial order
        , kp1_d(math_utils::ipow(mra.getOrder() + 1, D)) ///nr of scaling coefficients \f$ (k+1)^D \f$
        , name(n)
        , squareNorm(-1.0)
        , rootBox(mra.getWorldBox()) {
    this->nodesAtDepth.push_back(0);
}

/** @brief MWTree destructor. */
template <int D> MWTree<D>::~MWTree() {
    this->endNodeTable.clear();
    if (this->nodesAtDepth.size() != 1) MSG_ERROR("Nodes at depth != 1 -> " << this->nodesAtDepth.size());
    if (this->nodesAtDepth[0] != 0) MSG_ERROR("Nodes at depth 0 != 0 -> " << this->nodesAtDepth[0]);
}

/** @brief Deletes all the nodes in the tree
  *
  * @details This method will recursively delete all the nodes,
  * including the root nodes. Derived classes will call this method
  * when the object is deleted.
  */
template <int D> void MWTree<D>::deleteRootNodes() {
    for (int i = 0; i < this->rootBox.size(); i++) {
        MWNode<D> &root = this->getRootMWNode(i);
        root.deleteChildren();
        root.dealloc();
        this->rootBox.clearNode(i);
    }
}

/** @brief Serialize a tree metadata to a file :: RAFA
 *
 * @details Store the dimension of the problem, scaling function space size,
 * order of the polynomial, level index, translation index,
 * a node having children or not, function values corresponding to the node into a file.
 *
 * @param[in] filename: Desired filename as a `std::string`
 *
 * @returns: nothing
 */
template <int D> void MWTree<D>::Serialize(const std::string filename) {

    const auto lowerbound = this->getMRA().getWorldBox().getLowerBound(0);
    const auto upperbound = this->getMRA().getWorldBox().getUpperBound(0);
    const auto order0 = this->getMRA().getOrder();
    const int size = pow(2 * (order0 + 1),D);
    std::cout << "number of scaling functions: " << size << std::endl; 
    std::ofstream ofs;
    ofs.open(filename);
    ofs.precision(12);
    std::cout.precision(12);
    
    ofs << lowerbound << " " << upperbound << std::endl; // cell size
    //std::cout << lowerbound << " " << upperbound << std::endl; // cell size
    ofs << order0 << std::endl;                          // order of the polynomial
    //std::cout << order0 << std::endl;                          // order of the polynomial
    ofs << size << std::endl;                            // size of the scaling function space
    //std::cout << size << std::endl;                            // size of the scaling function space
    
    TreeIterator<D> it(*this, TopDown);
    while (it.next()) {
    // for(auto *node : *this-> getEndNodeTable()){
        MWNode<D> &node = it.getNode();
        const NodeIndex<D> &index = node.getNodeIndex();
        const int ncoefs = node.getNCoefs();
        node.mwTransform(Reconstruction); // I need to reconstruct them bcoz it is assumed that it should be in this form
        node.cvTransform(Forward);
        double *coefs = node.getCoefs();
        int n = index.getScale();
        std::array<int, D> l = index.getTranslation();
        bool hasChildren = node.isBranchNode();
        // ofs << n + 1 << " " << 2*l[0] << " " << hasChildren << "\t";
        // for (int ndim = 0; ndim < D; ndim++){
        ofs << n << " ";
        //std::cout << n << " ";
        for(int ndim = 0; ndim < D; ndim++){
            ofs << l[ndim] << " ";
          //  std::cout<< l[ndim] << " ";
        }
        
        ofs << hasChildren << "\t";
        //std::cout << hasChildren << "\t";
        
        // for now we print everything
        for (int i = 0; i < ncoefs; ++i) {
                ofs << coefs[i] << "\t";
          //      std::cout << coefs[i] << "\t";
            }

        // condition whether the only the child values are important
        // if (hasChildren == true) {
        //     ofs << " ";
        //     std::cout << " ";
        // } else {
        //     for (int i = 0; i < ncoefs; ++i) {
        //         ofs << coefs[i] << "\t";
        //         std::cout << coefs[i] << "\t";
        //     }
        // }
        // // for (int i = 0; i < ncoefs/2 ; ++i){
        //     ofs << coefs[i] << "\t";
        // }
        // ofs << std::endl;
        // ofs << n + 1 << " " << 2*l[0] + 1 << " " << hasChildren << "\t";
        // for (int j = ncoefs/2; j < ncoefs; ++j){
        //     ofs << coefs[j] << "\t";
        // }
        //std::cout << std::endl;
        ofs << std::endl;

        node.cvTransform(Backward);
        node.mwTransform(Compression);
    }
    ofs.close();
    std::cout << "The tree has been saved in the file named : " << filename << std::endl; 
}

/** @brief Convert the vector data of nodes to a map :: RAFA
 *
 * @details Coming soon
 *
 */
// template <int D> std::map<NodeIndex<D>, std::vector<double>> MWTree<D>::treeVector2Map(const std::vector<MWData<D>> &DataSet) {

//     std::map<NodeIndex<D>, std::vector<double>> treemap;
//     // std::array<int,D> Translation_Array{0}; 

//     for (int i = 0; i < DataSet.size(); ++i) {

//         NodeIndex<D> idx(DataSet[i].Level,DataSet[i].Translation_Array); // define Translation_Array
//         treemap[idx] = DataSet[i].FnValue; 
//         }
//     // {n,lx,ly,lz} -> {Func_val}

//     std::map<NodeIndex<D>, std::vector<double>>::iterator iter;

//     for (iter = treemap.begin(); iter != treemap.end(); ++iter) {
//         for (auto key : iter->first) { std::cout << key << " "; }

//         for (auto value : iter->second) { std::cout << value << " "; }
//         std::cout << std::endl;
//     }
//     return treemap;
// }

template<std::size_t D>
std::istream& operator>>(std::istream& is,std::array<int,D>& l){
    for(int i = 0; i < D; i++)
        is >> l[i];
    return is;
}

/** @brief Deserialize tree from a file :: RAFA
 *
 * @details Coming soon
 *
 */
template <int D> std::map<NodeIndex<D>, std::vector<double>> MWTree<D>::extractTreeData(const std::string filename) {
    // variables
  //  std::map<NodeIndex<D>, std::vector<double>> datasetmap;
    std::vector<MWData<D>> dataset;
    double lowerbound, upperbound;
    int order_poly_max, scal_fn_size;
    int n;
    constexpr std::size_t NDIM=D;
    std::array<int,NDIM> l;
    bool child;
    std::vector<double> function_values;

    std::ifstream ifs;
    ifs.open(filename);
    std::cout.precision(12);
    ifs.precision(12);

    if (!ifs) {
        std::cout << "file does not exist";
    } else {
        std::cout << "File exists and reading started" << std::endl;
        ifs >> lowerbound >> upperbound;
        ifs >> order_poly_max;
        ifs >> scal_fn_size;
        std::cout << lowerbound << " " << upperbound << std::endl;
        std::cout << order_poly_max << std::endl;
        std::cout << scal_fn_size << std::endl;
        //bool decision = true;
        // continuous data starts
        while (ifs >> n >> l >> child){
        // while(decision) {
            //decision &= ifs >> n;
            
            
            //for(int d=0; d<D;d++)
              //  decision &= ifs >> l[d];
            
            //decision &= ifs >> child;

            /// ifs >> n >> l >> child

            std::cout << "n: " << n << " l: " << l << " c: " << child << " ";
            double tmp_value;
            for (int i = 0; i < scal_fn_size; ++i)
            {
                ifs >> tmp_val;
                function_values.push_back(tmp_val);
            }
            MWData<D> data0(n, l, child, function_values);
            dataset.push_back(data0);
            function_values.clear(); // clear the vector
            
            // if (child == 1) {
            //     function_values.push_back(0.0);
            //     // std::cout << "no function values" << std::endl;
            //     MWData<D> data1(n, l, child, function_values);
            //     dataset.push_back(data1);
            //     function_values.clear();
            // } else if (child == 0) {
            //     for (int i = 0; i < scal_fn_size; ++i) {
            //         ifs >> tmp_value;
            //         function_values.push_back(tmp_value);
            //     }
            //     MWData<D> data0(n, l, child, function_values);
            //     dataset.push_back(data0);
            //     function_values.clear(); // clear the vector
            // }
        }
    }

    ifs.close();
//    datasetmap = treeVector2Map(dataset);
    return dataset;
}

/** @brief Remove all nodes in the tree
 *
 * @details Leaves the tree in the same state as after construction,
 * i.e.  undefined tree structure containing only root nodes without
 * coefficients.  The assigned memory, including branch and leaf
 * nodes, (nodeChunks in NodeAllocator) is NOT released, but is
 * immediately available to the new function.
 */
template <int D> void MWTree<D>::clear() {
    for (int i = 0; i < this->rootBox.size(); i++) {
        MWNode<D> &root = this->getRootMWNode(i);
        root.deleteChildren();
        root.clearHasCoefs();
        root.clearNorms();
    }
    this->resetEndNodeTable();
    this->clearSquareNorm();
}

/** @brief Calculate the squared norm \f$ ||f||^2_{\ldots} \f$ of a function represented as a tree.
 *
 * @details The norm is calculated using endNodes only. The specific
 * type of norm which is computed will depend on the derived class
 */
template <int D> void MWTree<D>::calcSquareNorm() {
    double treeNorm = 0.0;
    for (int n = 0; n < this->getNEndNodes(); n++) {
        const MWNode<D> &node = getEndMWNode(n);
        assert(node.hasCoefs());
        treeNorm += node.getSquareNorm();
    }
    this->squareNorm = treeNorm;
}

/** @brief Full Multiwavelet transform of the tree in either directions
 *
 * @param[in] type: TopDown (from roots to leaves) or BottomUp (from
 * leaves to roots) which specifies the direction of the MW transform
 * @param[in] overwrite: if true, the result will overwrite
 * preexisting coefficients.
 *
 * @details It performs a Multiwavlet transform of the whole tree. The
 * input parameters will specify the direction (upwards or downwards)
 * and whether the result is added to the coefficients or it
 * overwrites them. See the documentation for the #mwTransformUp 
 * and #mwTransformDown for details.
 * \f[ 
 * \pmatrix{
 * s_{nl}\\
 * d_{nl}
 * }
 * \rightleftarrows \pmatrix{
 * s_{n+1,2l}\\
 * s_{n+1,2l+1}
 * }
 * \f]
 */
template <int D> void MWTree<D>::mwTransform(int type, bool overwrite) {
    switch (type) {
        case TopDown:
            mwTransformDown(overwrite);
            break;
        case BottomUp:
            if (not overwrite) NOT_IMPLEMENTED_ABORT;
            mwTransformUp();
            break;
        default:
            MSG_ABORT("Invalid wavelet transform");
    }
}

/** @brief Regenerates all s/d-coeffs by backtransformation
 *
 * @details It starts at the bottom of the tree (scaling coefficients
 * of the leaf nodes) and it generates the scaling and wavelet
 * coefficients if the parent node. It then proceeds recursively all the
 * way up to the root nodes. This is generally used after a function
 * projection to purify the coefficients obtained by quadrature at
 * coarser scales which are therefore not precise enough.
 */
template <int D> void MWTree<D>::mwTransformUp() {
    std::vector<MWNodeVector<D>> nodeTable;
    tree_utils::make_node_table(*this, nodeTable);
#pragma omp parallel shared(nodeTable) num_threads(mrcpp_get_num_threads())
    {
        int start = nodeTable.size() - 2;
        for (int n = start; n >= 0; n--) {
            int nNodes = nodeTable[n].size();
#pragma omp for schedule(guided)
            for (int i = 0; i < nNodes; i++) {
                MWNode<D> &node = *nodeTable[n][i];
                if (node.isBranchNode()) { node.reCompress(); }
            }
        }
    }
}

/** @brief Regenerates all scaling coeffs by MW transformation of existing s/w-coeffs
 * on coarser scales
 *
 * @param[in] overwrite: if true the preexisting coefficients are overwritten
 *
 * @details The transformation starts at the rootNodes and proceeds
 * recursively all the way to the leaf nodes. The existing scaling
 * coefficeints will either be overwritten or added to. The latter
 * operation is generally used after the operator application.
 *
 */
template <int D> void MWTree<D>::mwTransformDown(bool overwrite) {
    std::vector<MWNodeVector<D>> nodeTable;
    tree_utils::make_node_table(*this, nodeTable);
#pragma omp parallel shared(nodeTable) num_threads(mrcpp_get_num_threads())
    {
        for (int n = 0; n < nodeTable.size(); n++) {
            int n_nodes = nodeTable[n].size();
#pragma omp for schedule(guided)
            for (int i = 0; i < n_nodes; i++) {
                MWNode<D> &node = *nodeTable[n][i];
                if (node.isBranchNode()) {
                    if (this->getRootScale() > node.getScale()) {
                        int reverse = n_nodes - 1;
                        int cIdx = this->getRootBox().getBoxIndex(node.getNodeIndex());
                        node.giveChildCoefs(reverse - cIdx, overwrite);
                    } else {
                        node.giveChildrenCoefs(overwrite);
                    }
                }
            }
        }
    }
}

/** @brief Set the MW coefficients to zero, keeping the same tree structure
 *   
 * @details Keeps the node structure of the tree, even though the zero
 * function is representable at depth zero. One should then use \ref cropTree to remove
 * unnecessary nodes.
 */
template <int D> void MWTree<D>::setZero() {
    TreeIterator<D> it(*this);
    while (it.next()) {
        MWNode<D> &node = it.getNode();
        node.zeroCoefs();
    }
    this->squareNorm = 0.0;
}

/** @brief Increments node counter by one for non-GenNodes.
 *
 * @details TO BE DOCUMENTED
 * \warning: This routine is not thread
 * safe, and must NEVER be called outside a critical region in parallel.
 * It's way. way too expensive to lock the tree, so don't even think
 * about it. */
template <int D> void MWTree<D>::incrementNodeCount(int scale) {
    int depth = scale - getRootScale();
    if (depth < 0) {
        int n = this->nodesAtNegativeDepth.size();
        if (-depth > n) {
            for (int i = 0; i < -depth - n; i++) { this->nodesAtNegativeDepth.push_back(0); }
        }
        this->nodesAtNegativeDepth[-depth - 1]++;
    } else {
        int n = this->nodesAtDepth.size() - 1;
        if (depth > n) {
            for (int i = 0; i < depth - n; i++) { this->nodesAtDepth.push_back(0); }
        }
        this->nodesAtDepth[depth]++;
    }
}

/** @brief Decrements node counter by one for non-GenNodes.
 *
 * @details TO BE DOCUMENTED
 * \warning: This routine is not thread
 * safe, and must NEVER be called outside a critical region in parallel.
 * It's way. way too expensive to lock the tree, so don't even think
 * about it.
 */
template <int D> void MWTree<D>::decrementNodeCount(int scale) {
    int depth = scale - getRootScale();
    if (depth < 0) {
        assert(-depth - 1 < this->nodesAtNegativeDepth.size());
        this->nodesAtNegativeDepth[-depth - 1]--;
        assert(this->nodesAtNegativeDepth[-depth - 1] >= 0);
        if (this->nodesAtNegativeDepth[-depth - 1] == 0 and this->nodesAtNegativeDepth.size() > 0) this->nodesAtNegativeDepth.pop_back();
    } else {
        assert(depth < this->nodesAtDepth.size());
        this->nodesAtDepth[depth]--;
        assert(this->nodesAtDepth[depth] >= 0);
        if (this->nodesAtDepth[depth] == 0 and this->nodesAtDepth.size() > 1) this->nodesAtDepth.pop_back();
    }
}

/** @returns Total number of nodes in the tree, at given depth (not in use)
 *
 * @param[in] depth: Tree depth (0 depth is the coarsest scale) to count.
 */
template <int D> int MWTree<D>::getNNodesAtDepth(int depth) const {
    int N = 0;
    if (depth < 0) {
        if (this->nodesAtNegativeDepth.size() >= -depth) N = this->nodesAtNegativeDepth[-depth];
    } else {
        if (this->nodesAtDepth.size() > depth) N = this->nodesAtDepth[depth];
    }
    return N;
}

/** @returns Size of all MW coefs in the tree, in kB */
template <int D> int MWTree<D>::getSizeNodes() const {
    auto nCoefs = 1ll * getNNodes() * getTDim() * getKp1_d();
    return sizeof(double) * nCoefs / 1024;
}

/** @brief Finds and returns the node pointer with the given \ref NodeIndex, const version.
 *
 * @details Recursive routine to find and return the node with a given
 * NodeIndex.  This routine returns the appropriate Node, or a NULL
 * pointer if the node does not exist, or if it is a
 * GenNode. Recursion starts at the appropriate rootNode.
 */
template <int D> const MWNode<D> *MWTree<D>::findNode(NodeIndex<D> idx) const {
    if (getRootBox().isPeriodic()) { periodic::index_manipulation<D>(idx, getRootBox().getPeriodic()); }
    int rIdx = getRootBox().getBoxIndex(idx);
    if (rIdx < 0) return nullptr;
    const MWNode<D> &root = this->rootBox.getNode(rIdx);
    assert(root.isAncestor(idx));
    return root.retrieveNodeNoGen(idx);
}

/** @brief Finds and returns the node pointer with the given \ref NodeIndex.
 *
 * @details Recursive routine to find and return the node with a given
 * NodeIndex.  This routine returns the appropriate Node, or a NULL
 * pointer if the node does not exist, or if it is a
 * GenNode. Recursion starts at the appropriate rootNode.
 */
template <int D> MWNode<D> *MWTree<D>::findNode(NodeIndex<D> idx) {
    if (getRootBox().isPeriodic()) { periodic::index_manipulation<D>(idx, getRootBox().getPeriodic()); }
    int rIdx = getRootBox().getBoxIndex(idx);
    if (rIdx < 0) return nullptr;
    MWNode<D> &root = this->rootBox.getNode(rIdx);
    assert(root.isAncestor(idx));
    return root.retrieveNodeNoGen(idx);
}

/** @brief Finds and returns the node reference with the given NodeIndex.
 *
 * @details This routine ALWAYS returns the node you ask for. If the
 * node does not exist, it will be generated by MW
 * transform. Recursion starts at the appropriate rootNode and descends
 * from this.
 */
template <int D> MWNode<D> &MWTree<D>::getNode(NodeIndex<D> idx) {
    if (getRootBox().isPeriodic()) periodic::index_manipulation<D>(idx, getRootBox().getPeriodic());

    MWNode<D> *out = nullptr;
    MWNode<D> &root = getRootBox().getNode(idx);
    if (idx.getScale() < getRootScale()) {
#pragma omp critical(gen_parent)
        out = root.retrieveParent(idx);
    } else {
        out = root.retrieveNode(idx);
    }
    return *out;
}

/** @brief Finds and returns the node with the given NodeIndex.
 *
 * @details This routine returns the Node you ask for, or the EndNode
 * on the path to the requested node, if the requested one is deeper
 * than the leaf node ancestor. It will never create or return
 * GenNodes.  Recursion starts at the appropriate rootNode and decends
 * from this.
 */
template <int D> MWNode<D> &MWTree<D>::getNodeOrEndNode(NodeIndex<D> idx) {
    if (getRootBox().isPeriodic()) { periodic::index_manipulation<D>(idx, getRootBox().getPeriodic()); }
    MWNode<D> &root = getRootBox().getNode(idx);
    assert(root.isAncestor(idx));
    return *root.retrieveNodeOrEndNode(idx);
}

/** @brief Finds and returns the node reference with the given NodeIndex. Const version.
 *
 * @details This routine ALWAYS returns the node you ask for. If the
 * node does not exist, it will be generated by MW
 * transform. Recursion starts at the appropriate rootNode and decends
 * from this.
 */
template <int D> const MWNode<D> &MWTree<D>::getNodeOrEndNode(NodeIndex<D> idx) const {
    if (getRootBox().isPeriodic()) { periodic::index_manipulation<D>(idx, getRootBox().getPeriodic()); }
    const MWNode<D> &root = getRootBox().getNode(idx);
    assert(root.isAncestor(idx));
    return *root.retrieveNodeOrEndNode(idx);
}

/** @brief Finds and returns the node at a given depth that contains a given coordinate.
 *
 * @param[in] depth: requested node depth from root scale.
 * @param[in] r: coordinates of an arbitrary point in space
 *
 * @details This routine ALWAYS returns the node you ask for, and will
 * generate nodes that do not exist. Recursion starts at the
 * appropriate rootNode and decends from this.
 */
template <int D> MWNode<D> &MWTree<D>::getNode(Coord<D> r, int depth) {
    MWNode<D> &root = getRootBox().getNode(r);
    if (depth >= 0) {
        return *root.retrieveNode(r, depth);
    } else {
        return *root.retrieveNodeOrEndNode(r, depth);
    }
}

/** @brief Finds and returns the node at a given depth that contains a given coordinate.
 *
 * @param[in] depth: requested node depth from root scale.
 * @param[in] r: coordinates of an arbitrary point in space
 *
 * @details This routine returns the Node you ask for, or the EndNode on
 * the path to the requested node, and will never create or return GenNodes.
 * Recursion starts at the appropriate rootNode and decends from this.
 */
template <int D> MWNode<D> &MWTree<D>::getNodeOrEndNode(Coord<D> r, int depth) {

    if (getRootBox().isPeriodic()) { periodic::coord_manipulation<D>(r, getRootBox().getPeriodic()); }

    MWNode<D> &root = getRootBox().getNode(r);
    return *root.retrieveNodeOrEndNode(r, depth);
}

/** @brief Finds and returns the node at a given depth that contains a given coordinate. Const version
 *
 * @param[in] depth: requested node depth from root scale.
 * @param[in] r: coordinates of an arbitrary point in space
 *
 * @details This routine returns the Node you ask for, or the EndNode on
 * the path to the requested node, and will never create or return GenNodes.
 * Recursion starts at the appropriate rootNode and decends from this.
 */
template <int D> const MWNode<D> &MWTree<D>::getNodeOrEndNode(Coord<D> r, int depth) const {

    if (getRootBox().isPeriodic()) { periodic::coord_manipulation<D>(r, getRootBox().getPeriodic()); }
    const MWNode<D> &root = getRootBox().getNode(r);
    return *root.retrieveNodeOrEndNode(r, depth);
}

/** @brief Returns the list of all EndNodes
 *
 * @details copies the list of all EndNode pointers into a new vector
 * and retunrs it.
 */
template <int D> MWNodeVector<D> *MWTree<D>::copyEndNodeTable() {
    auto *nVec = new MWNodeVector<D>;
    for (int n = 0; n < getNEndNodes(); n++) {
        MWNode<D> &node = getEndMWNode(n);
        nVec->push_back(&node);
    }
    return nVec;
}

/** @brief Recreate the endNodeTable
 *
 * @details the endNodeTable is first deleted and then rebuilt from
 * scratch. It makes use of the TreeIterator to traverse the tree.
 * 
 */
template <int D> void MWTree<D>::resetEndNodeTable() {
    clearEndNodeTable();
    TreeIterator<D> it(*this, TopDown, Hilbert);
    it.setReturnGenNodes(false);
    while (it.next()) {
        MWNode<D> &node = it.getNode();
        if (node.isEndNode()) { this->endNodeTable.push_back(&node); }
    }
}


template <int D> int MWTree<D>::countBranchNodes(int depth) {
    NOT_IMPLEMENTED_ABORT;
}

template <int D> int MWTree<D>::countLeafNodes(int depth) {
    NOT_IMPLEMENTED_ABORT;
    //    int nNodes = 0;
    //    TreeIterator<D> it(*this);
    //    while (it.next()) {
    //        MWNode<D> &node = it.getNode();
    //        if (node.getDepth() == depth or depth < 0) {
    //            if (node.isLeafNode()) {
    //                nNodes++;
    //            }
    //        }
    //    }
    //    return nNodes;
}

/* Traverse tree and count nodes belonging to this rank. */
template <int D> int MWTree<D>::countNodes(int depth) {
    NOT_IMPLEMENTED_ABORT;
    //    TreeIterator<D> it(*this);
    //    int count = 0;
    //    while (it.next()) {
    //        MWNode<D> &node = it.getNode();
    //        if (node.isGenNode()) {
    //            continue;
    //        }
    //        if (not node.isForeign()) {
    //            count++;
    //        }
    //    }
    //    return count;
}

/* Traverse tree and count nodes with allocated coefficients. */
template <int D> int MWTree<D>::countAllocNodes(int depth) {
    NOT_IMPLEMENTED_ABORT;
    //    TreeIterator<D> it(*this);
    //    int count = 0;
    //    while (it.next()) {
    //        MWNode<D> &node = it.getNode();
    //        if (node.isGenNode()) {
    //            continue;
    //        }
    //        if (node.hasCoefs()) {
    //            count++;
    //        }
    //    }
    //    return count;
}

/** @brief Prints a summary of the tree structure on the output file
 */
template <int D> std::ostream &MWTree<D>::print(std::ostream &o) const {
    o << "  square norm: " << this->squareNorm << std::endl;
    o << "  root scale: " << this->getRootScale() << std::endl;
    o << "  order: " << this->order << std::endl;
    o << "  nodes: " << this->getNNodes() << std::endl;
    o << "  endNodes: " << this->endNodeTable.size() << std::endl;
    o << "  nodes per scale: " << std::endl;
    for (int i = this->nodesAtNegativeDepth.size() - 1; i >= 0; i--) { o << "    scale=" << -(i + this->getRootScale() + 1) << "  nodes=" << this->nodesAtNegativeDepth[i] << std::endl; }
    for (int i = 0; i < this->nodesAtDepth.size(); i++) { o << "    scale=" << i + this->getRootScale() << "  nodes=" << this->nodesAtDepth[i] << std::endl; }
    return o;
}

/** @brief sets values for maxSquareNorm in all nodes
 *
 * @details it defines the upper bound of the squared norm \f$
 * ||f||^2_{\ldots} \f$ in this node or its descendents
 */
template <int D> void MWTree<D>::makeMaxSquareNorms() {
    NodeBox<D> &rBox = this->getRootBox();
    MWNode<D> **roots = rBox.getNodes();
    for (int rIdx = 0; rIdx < rBox.size(); rIdx++) {
        // recursively set value of children and descendants
        roots[rIdx]->setMaxSquareNorm();
    }
}

/** @brief gives serialIx of a node from its NodeIndex
 *
 * @details Peter will document this!
 */
template <int D> int MWTree<D>::getIx(NodeIndex<D> nIdx) {
    if (this->isLocal == false) MSG_ERROR("getIx only implemented in local representation");
    if (NodeIndex2serialIx.count(nIdx) == 0)
        return -1;
    else
        return NodeIndex2serialIx[nIdx];
}

template <int D> void MWTree<D>::getNodeCoeff(NodeIndex<D> nIdx, double *data) {
    assert(this->isLocal);
    int size = (1 << D) * kp1_d;
    int id = 0;
    for (int i = 0; i < D; i++) id += std::abs(nIdx.getTranslation(i));
    this->NodesCoeff->get_data(id, size, data);
}

template <int D> void MWTree<D>::getNodeCoeff(NodeIndex<D> nIdx, double *data) {
    assert(this->isLocal);
    int size = (1 << D) * kp1_d;
    int id = 0;
    for (int i = 0; i < D; i++) id += std::abs(nIdx.getTranslation(i));
    this->NodesCoeff->get_data(id, size, data);
}

template class MWTree<1>;
template class MWTree<2>;
template class MWTree<3>;

} // namespace mrcpp
