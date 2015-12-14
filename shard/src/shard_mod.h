#ifndef __SHARD_MOD_H__
#define __SHARD_MOD_H__

#include <cstdint>

#include <shard.h>

class ShardMod : public IShard
{
  public:
    virtual ~ShardMod() {};

    virtual int32_t hash(const uint64_t key, const uint32_t bucket);
};

#endif
