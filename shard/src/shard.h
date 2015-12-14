#ifndef __SHARD_H__
#define __SHARD_H__

#include <cstdint>

class IShard
{
  public:
    virtual ~IShard() {};

    virtual int32_t hash(const uint64_t key, const uint32_t buckets) = 0;
};

#endif
