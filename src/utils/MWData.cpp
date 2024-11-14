/* created by Raunak Farhaz
 * 16 October 2023
 * UiT - HU
 */
#include "MWData.h"

namespace mrcpp {

template <int D> void MWData<D>::print() {
    std::cout << "File starts reading checked!" << std::endl;
}

template class MWData<1>;
template class MWData<2>;
template class MWData<3>;
} // namespace mrcpp