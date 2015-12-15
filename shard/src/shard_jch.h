#ifndef __SHARD_MOD_H__
#define __SHARD_MOD_H__

#include <cstdint>

#include <shard.h>

class ShardJumConsistentHash : public IShard
{
  public:
    virtual ~ShardJumConsistentHash() {};

    virtual int32_t hash(const uint64_t key, const uint32_t bucket);
};

#endif
