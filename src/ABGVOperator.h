#pragma once

#include "DerivativeOperator.h"

template<int D>
class ABGVOperator : public DerivativeOperator<D> {
public:
    ABGVOperator(const MultiResolutionAnalysis<D> &mra, double a, double b);
    virtual ~ABGVOperator() { }

protected:
    void initializeOperator(double a, double b);
};

