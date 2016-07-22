#include <benchmark/benchemark_api.h>

#include <iostream>

using namespace std;

void long() {
	for (int i = 0; i < 1000000; ++i) {
		for (int j = 0; j < 1000; ++j) {
			char a = 'x';
		}
	}
}

void short() {
	long();
}

static void BM_TEST(benchmark::State& state) {
	while (state.KeepRunning())
		short();
}
BENCHMARK(BM_TEST);

BENCHMARK_MAIN();
