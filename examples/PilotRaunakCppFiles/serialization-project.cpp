#include "MRCPP/MWFunctions"
#include "MRCPP/Printer"
#include "MRCPP/Timer"

#include "MRCPP/trees/TreeIterator.h"
#include "MRCPP/trees/MWNode.h"
#include "fstream"

const auto min_scale = 0;
//const auto max_depth = 25;

const auto order = 6;
const auto prec = 1.0e-5;
const auto D = 1;

// double guess_function(mrcpp::Coord<D> &r){
//     double x = r[0];
//     return std::sin(x);
// }

int main(int argc, char **argv) {
    auto timer = mrcpp::Timer();

    // Initialize printing
    auto printlevel = 0;
    mrcpp::Printer::init(printlevel);
    mrcpp::print::environment(0);

    // Constructing world box
    auto corner = std::array<int, D>{};                                         //
    auto boxes = std::array<int, D>{1};                                         // no. of boxes
    auto world = mrcpp::BoundingBox<D>(min_scale, corner, boxes, std::array<double, 1>{10.0}); // working on [0,L]
    
    // Constructing basis and MRA
    auto basis = mrcpp::LegendreBasis(order);
    auto MRA = mrcpp::MultiResolutionAnalysis<D>(world, basis);

    // Defining analytic function
    auto f = [](const mrcpp::Coord<1> &r) -> double { return std::sin(r[0]); };

    // Projecting function
    mrcpp::FunctionTree<D> f_tree(MRA);
    mrcpp::project<1>(prec, f_tree, f);
    println(0, f_tree);
    // iteration
    std::ofstream ofs;
    ofs.open("output");
    mrcpp::TreeIterator<1> it(f_tree, mrcpp::TopDown);
    while (it.next()) {
        const mrcpp::MWNode<1> &node = it.getNode(); // here you get the node
        const mrcpp::NodeIndex<1> &idx = node.getNodeIndex(); // index the node for iteration
        const double *coefs = node.getCoefs(); // get the coefficients
        int n = idx.getScale(); // n values
        std::array<int, D> l = idx.getTranslation(); // l values
        std::string hasChildren = node.isBranchNode() ? "yes" : "no"; // whether node has children or not?
        // streaming stuff 
        ofs << "n :" << n << "  l : " << l[0] << "  hasChildren : " << hasChildren << "\t";
        for (int i = 0 ; i < n; i++)
            ofs << coefs[i] << "\t";
        ofs << std::endl;
    }
    ofs.close();

    


    mrcpp::print::footer(0, timer, 2);

    return 0;
}

