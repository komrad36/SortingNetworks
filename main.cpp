#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

#include "sorts.h"

using namespace std::chrono;

extern "C" {
	void sort2a(int* x);
	void sort3a(int* x);
	void sort4a(int* x);
	void sort5a(int* x);
	void sort6a(int* x);
	void simdsort4a(int* x);
	void simdsort4a_noconstants(int* x);
	void simdsort4a_nofloat(int* x);
}

#include "KIterTools.h"

constexpr auto f = simdsort4a;

int main() {
	std::vector<int> ref = { rand()*(INT_MAX/RAND_MAX*2), rand()*(INT_MAX / RAND_MAX*2), rand()*(INT_MAX / RAND_MAX*2), rand()*(INT_MAX / RAND_MAX*2) };
	ref = { 1, 2, 3, 4 };
	std::sort(ref.begin(), ref.end());
	Permutations<int> ps(ref);
	std::vector<int> p;
	bool all_good = true;
	while (ps.next(p)) {
		f(p.data());
		for (int i = 0; i < p.size(); ++i) {
			if (p[i] != ref[i]) {
				std::cout << "NOPE!" << std::endl;
				all_good = false;
			}
		}
	}

	if (all_good) std::cout << "All good!" << std::endl;

	constexpr int runs = 1000000000;
	int x[4];
	auto start = high_resolution_clock::now();
	for (int i = 0; i < runs; ++i) {
		x[0] = 1;
		x[1] = 2;
		x[2] = 3;
		x[3] = 4;
		f(x);
	}
	auto end = high_resolution_clock::now();
	const double ns = static_cast<double>(duration_cast<nanoseconds>(end - start).count()) / static_cast<double>(runs);
	std::cout << ns << " ns per sort." << std::endl;
}
