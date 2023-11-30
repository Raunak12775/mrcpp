/* created by Raunak Farhaz
 * 16 October 2023
 * UiT - HU
 */
#pragma once

#include "fstream"
#include "iostream"
#include "vector"


namespace mrcpp {
template <int D> class MWData {
public:
    // member variables
        int Level;
        std::array<int,D> Translation;
        bool HasChild;
        std::vector<double> FnValue;


    // constructor
        MWData() {} // empty constructor

        MWData(int _n, std::array<int,D> _l, bool _child, std::vector<double> _fn_value) {
            Level = _n;
            Translation = _l;
            HasChild = _child;
            FnValue = _fn_value;
        }
        
    // member functions
        void print();

};

} // namespace mrcpp