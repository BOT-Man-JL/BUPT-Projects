
// Bloom Filter
//   by BOT Man & ZhangHan, 2018

#ifndef BLOOM_FILTER
#define BLOOM_FILTER

#include <stddef.h>

struct _BloomFilter;
typedef struct _BloomFilter BloomFilter;

BloomFilter* CreateBloomFilter(size_t size);
void FreeBloomFilter(BloomFilter* filter);

void AssertBloomFilterNoLeak();

void BloomFilterAdd(BloomFilter* filter, const char* str);
unsigned char BloomFilterTest(BloomFilter* filter, const char* str);

#endif  // BLOOM_FILTER
