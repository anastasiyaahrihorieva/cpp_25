#include "header.h"

int main() {
	const char* file[] = { "a.txt", "b.txt", "c.txt" };
	double a = read_coeff(file[0]);
	double b = read_coeff(file[1]);
	double c = read_coeff(file[2]);
	double D = discriminant(a, b, c);

	if (D > 0) {
		roots result = find_solution(a, b, D);
		std::cout << "x1: " << result.x1 << ", x2: " << result.x2;
	}
	else if (D == 0) {
		double x = one_root(a, b);
		std::cout << "x1: ";
	}
	else {
		std::cout << "no real roots";
	}

	return 0;
}