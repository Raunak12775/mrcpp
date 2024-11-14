#include "MRCPP/MWFunctions"
#include "MRCPP/Printer"
#include "MRCPP/Timer"
#include "fstream"
#include "trees/MWNode.h"
#include "trees/MWTree.h"
#include "trees/MultiResolutionAnalysis.h"
#include "trees/NodeAllocator.h"
#include "trees/NodeIndex.h"
#include "trees/TreeIterator.h"
#include "utils/math_utils.h"
#include "utils/periodic_utils.h"
#include "utils/tree_utils.h"
#include "utils/MWData.h"
#include "MRCPP/Plotter"
//#include "treebuilders/ReadCalculator.h"
#include "treebuilders/project.h"

using namespace mrcpp;

const auto min_scale = 0; // ?
const auto max_depth = 10; // ?

const auto order = 5;     // order of the polynomial, size of the scaling func = 6
const auto prec = 1.0e-5; // precision of d

const auto D = 3; // dimension of the problem

double test_function(const Coord<D> &r) {
    double x = r[0]; 
    double y = r[1];
    double z = r[2];
    auto res = std::pow((x*x + y*y + z*z),0.5);
    return std::sin(res);
}

int main(int argc, char **argv) {
    Timer timer;
    // Initialize printing
     int printlevel = 0;
     Printer::init(printlevel);
     print::environment(0);
     print::header(0, "MRCPP IO manipulation code w madness");
    
    // Constructing world box
     auto corner = std::array<int, D>{};
     auto boxes = std::array<int, D>{1}; // no. of boxes
    //  auto world = BoundingBox<D>(min_scale, corner, boxes, std::array<double, D>{10.0});
    //  auto world = BoundingBox<D>(min_scale, corner, boxes, std::array<double, D>{10.0,10.0}); 
     auto world = BoundingBox<D>(min_scale, corner, boxes, std::array<double, D>{10.0,10.0,10.0});
    
    // working on [0,L]
    // Constructing basis and MRA
     auto basis = LegendreBasis(order);
     auto MRA = MultiResolutionAnalysis<D>(world, basis); // value 3 is manually given to fake the representation
     MRA.print();
    
    // Projecting function
     FunctionTree<D> f_tree(MRA);
     project<D>(prec, f_tree, test_function, -1);
    
    //  auto integral = f_tree.integrate();
    
     // Plot
        // Plotter<D> plotter;
        // plotter.setRange({1.0});
        // plotter.linePlot({1000},f_tree,"plotfile");

    // serialize tree
    const std::string filename = "output_full.dat";
    f_tree.Serialize(filename);
    
    //std::vector<MWData<D>> dataset0;
    
    //auto datasetmap0=f_tree.extractTreeData(filename);
    // auto MRAout=MultiResolutionAnalysis<D>(world,Legendre);
    // FunctionTree<D> f_treeout(MRAout);
    // read(arguments);

    // for(auto i : dataset0)
    //     std::cout << i;


    print::header(0, "Projecting analytic function");
    print::tree(-1, "Projected function", f_tree, timer);
    // print::value(0, "Integrated function", integral, "(au)");
    print::footer(0, timer, 2);

    return 0;
}

// int main(int argc, char **argv){
    
    
//     std::vector<MWData<D>> dataset0;
    
    
//     //std::vector<MWData<D> > dataset; 
//     //dataset = deserialize("output");


//     return 0;
// }