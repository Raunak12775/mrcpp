#pragma once

#include "TreeCalculator.h"

namespace mrcpp {

class ABGVCalculator final : public TreeCalculator<2> {
public:
    ABGVCalculator(const ScalingBasis &basis, double a, double b);

protected:
    const double A;	 ///< Left boundary conditions, ref. Alpert et al.
    const double B;	 ///< Right boundary conditions, ref. Alpert et al.
    Eigen::MatrixXd K;
    Eigen::VectorXd valueZero;
    Eigen::VectorXd valueOne;

    virtual void calcNode(MWNode<2> &node);

    void calcKMatrix(const ScalingBasis &basis);
    void calcValueVectors(const ScalingBasis &basis);
};

}
