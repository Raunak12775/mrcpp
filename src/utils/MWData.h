/* created by Raunak Farhaz
 * 16 October 2023
 * UiT - HU
 */
#pragma once

#include "fstream"
#include "iostream"
#include "map"
#include "vector"

// mrcpp header files

namespace mrcpp {
template <int D> class MWData {
private:
    //int Dimension;
    //int Polynomial_Order;
    //int Scaling_Fn_Size;
    int Level;
    // std::vector<int> Translation;
    int Translation;
    bool HasChild;
    std::vector<double> FnValue;

public:
    // constructor
    MWData() {} // empty constructor

    MWData(int _n, int _l, bool _child, std::vector<double> _fn_value) {
        Level = _n;
        Translation = _l;
        HasChild = _child;
        FnValue = _fn_value;
    }

    // getters
    int getLevel() { return Level; }
    int getTranslation() { return Translation; }
    bool getHasChild() { return HasChild; }
    std::vector<double> getScalingFunctions() { return FnValue; }

    // member functions
    void print();


    // friend functions
    friend std::ostream &operator<<(std::ostream &output, const MWData<D> &data) {
        output << data.Level << " " << data.Translation << " " << data.HasChild << "\t";
        if (data.HasChild == 1) {
            output << "no data" << std::endl;
        } else {
            for (auto values : data.FnValue) { output << values << "\t"; }
            output << std::endl;
        }
        return output;
    }

    // friend std::ifstream &operator>>(std::ifstream &ifs0, const MWData<D> &data){
    //     ifs0.precision(12);
    //     // ifs0 >> data.Level << data.Translation << data.HasChild;
    //     return ifs0;
    // }
};

// template<int D>
// std::vector<MWData<D>> deserialize(const std::string &filename)
//     {
//         std::vector<MWData<D> > dataset;
//         double start, stop;
//         int ndim, order, scal_fn_size;
//         int n, l;
//         bool child;
//         std::vector<double> function_values;
//         std::ifstream ifs;
//         std::cout.precision(12);
//         ifs.precision(12);
//         ifs.open(filename);
//         if (!ifs)
//         {
//             std::cout << "file does not exist";
//         }
//         else
//         {
//             ifs >> ndim;
//             ifs >> start >> stop;
//             ifs >> order;
//             ifs >> scal_fn_size;
//             std::cout << ndim << "\n"
//                       << stop << " " << start << std::endl;
//             std::cout << order << std::endl;
//             std::cout << scal_fn_size << std::endl;
//             // continuous data starts
//             // int iterator_index = 0;
//             while (ifs >> n >> l >> child)
//             {
//                 // std::cout << "n: " << n << " l: " << l << " c: " << child << " ";
//                 double val;
//                 if (child == 1)
//                 {
//                     function_values.push_back(0.0);
//                     // std::cout << "no function values" << std::endl;
//                     MWData<D> data1(n, l, child, function_values);
//                     dataset.push_back(data1);
//                     function_values.clear();
//                 }
//                 else if (child == 0)
//                 {
//                     for (int i = 0; i < scal_fn_size; ++i)
//                     {
//                         ifs >> val;
//                         function_values.push_back(val);
//                     }
//                     MWData<D> data0(n, l, child, function_values);
//                     dataset.push_back(data0);
//                     function_values.clear(); // clear the vector
//                 }
//                 // iterator_index++;
//             }
//         }
//         ifs.close();
//         return dataset;
//     }

} // namespace mrcpp