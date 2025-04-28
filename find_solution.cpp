#include "header.h"

roots find_solution(double a, double b, double d) {
    double y1 = (-b + sqrt(d)) / (2 * a);
    double y2 = (-b - sqrt(d)) / (2 * a);
    return roots(y1, y2);
}
