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

#include "ReadCalculator.h"
#include "trees/MWNode.h"

using Eigen::MatrixXd;

namespace mrcpp {

template <int D> void ReadCalculator<D>::calcNode(MWNode<D> &node) {
    MatrixXd exp_pts;
    node.getExpandedChildPts(exp_pts);

    assert(exp_pts.cols() == node.getNCoefs());

    Coord<D> r;
    double *coefs = node.getCoefs();
    for (int i = 0; i < node.getNCoefs(); i++) {
        for (int d = 0; d < D; d++) { r[d] = scaling_factor[d] * exp_pts(d, i); }
        coefs[i] = this->func->evalf(r);
    }
    node.cvTransform(Backward);
    node.mwTransform(Compression);
    node.setHasCoefs();
    node.calcNorms();
}

template class ReadCalculator<1>;
template class ReadCalculator<2>;
template class ReadCalculator<3>;

} // namespace mrcpp
