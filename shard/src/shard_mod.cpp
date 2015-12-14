#include <shard_mod.h>

#include <cstdint>

int32_t ShardMod::hash(const uint64_t key, const uint32_t bucket) {
	return (key % bucket);
}
