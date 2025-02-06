#include "header.h"

double read_coeff(const char* pk) {
    double k;
    std::ifstream reading(pk);
    reading >> k;
    return k;
}