#include <iostream>
#include <fstream>
#include <cmath>

class roots {
public:
    double x1;
    double x2;
    roots(double v1, double v2) : x1(v1), x2(v2) {}
};

roots find_solution(double a, double b, double d);
double read_coeff(const char* pk);
double discriminant(double a, double b, double c);
double read_coeff(const char* pk);
double discriminant(double a, double b, double c);
double one_root(double a, double b);
