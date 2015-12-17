#include <shard_jch.h>

#include <cstdint>

int32_t ShardJumpConsistentHash::hash(const uint64_t key, const uint32_t bucket) {
	int64_t b = -1;
	int64_t j = 0;
	uint64_t k = key;
	while (j < bucket) {
		b = j;
		k = k * 2862933555777941757ULL + 1;
		j = (b + 1) * (double(1LL << 31) / double((key >> 33) + 1));
	}

	return b;
}
