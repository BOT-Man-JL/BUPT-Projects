
// Bloom Filter
//   by BOT Man & ZhangHan, 2018
//
// reference:
// https://drewdevault.com/2016/04/12/How-to-write-a-better-bloom-filter-in-C.html

#include "bloom_filter.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// hash functions

unsigned RSHash(const char* str, unsigned length) {
  unsigned b = 378551;
  unsigned a = 63689;
  unsigned hash = 0;
  unsigned i = 0;

  for (i = 0; i < length; ++str, ++i) {
    hash = hash * a + (*str);
    a = a * b;
  }

  return hash;
}

unsigned JSHash(const char* str, unsigned length) {
  unsigned hash = 1315423911;
  unsigned i = 0;

  for (i = 0; i < length; ++str, ++i) {
    hash ^= ((hash << 5) + (*str) + (hash >> 2));
  }

  return hash;
}

unsigned PJWHash(const char* str, unsigned length) {
  const unsigned BitsInUnsignedInt = (unsigned)(sizeof(unsigned) * 8);
  const unsigned ThreeQuarters = (unsigned)((BitsInUnsignedInt * 3) / 4);
  const unsigned OneEighth = (unsigned)(BitsInUnsignedInt / 8);
  const unsigned HighBits = (unsigned)(0xFFFFFFFF)
                            << (BitsInUnsignedInt - OneEighth);
  unsigned hash = 0;
  unsigned test = 0;
  unsigned i = 0;

  for (i = 0; i < length; ++str, ++i) {
    hash = (hash << OneEighth) + (*str);

    if ((test = hash & HighBits) != 0) {
      hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
    }
  }

  return hash;
}

unsigned ELFHash(const char* str, unsigned length) {
  unsigned hash = 0;
  unsigned x = 0;
  unsigned i = 0;

  for (i = 0; i < length; ++str, ++i) {
    hash = (hash << 4) + (*str);

    if ((x = hash & 0xF0000000L) != 0) {
      hash ^= (x >> 24);
    }

    hash &= ~x;
  }

  return hash;
}

unsigned BKDRHash(const char* str, unsigned length) {
  unsigned seed = 131; /* 31 131 1313 13131 131313 etc.. */
  unsigned hash = 0;
  unsigned i = 0;

  for (i = 0; i < length; ++str, ++i) {
    hash = (hash * seed) + (*str);
  }

  return hash;
}

unsigned DJBHash(const char* str, unsigned length) {
  unsigned hash = 5381;
  unsigned i = 0;

  for (i = 0; i < length; ++str, ++i) {
    hash = ((hash << 5) + hash) + (*str);
  }

  return hash;
}

unsigned DEKHash(const char* str, unsigned length) {
  unsigned hash = length;
  unsigned i = 0;

  for (i = 0; i < length; ++str, ++i) {
    hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
  }

  return hash;
}

unsigned APHash(const char* str, unsigned length) {
  unsigned hash = 0xAAAAAAAA;
  unsigned i = 0;

  for (i = 0; i < length; ++str, ++i) {
    hash ^= ((i & 1) == 0) ? ((hash << 7) ^ (*str) * (hash >> 3))
                           : (~((hash << 11) + ((*str) ^ (hash >> 5))));
  }

  return hash;
}

unsigned (*g_hash_funcs[])(const char* str, unsigned length) = {
    RSHash, JSHash, PJWHash, ELFHash, BKDRHash, DJBHash, DEKHash, APHash};

// bloom filter definitions

struct _BloomFilter {
  void* bits;
  size_t size;
};

size_t g_bloom_filter_count;

typedef unsigned char Unit;
#define UNIT_BIT (sizeof(Unit) * 8)
#define N_HASH_FUNC (sizeof(g_hash_funcs) / sizeof(g_hash_funcs[0]))

BloomFilter* CreateBloomFilter(size_t size) {
  BloomFilter* ret = (BloomFilter*)malloc(sizeof(BloomFilter));
  assert(ret);
  if (!ret)
    return NULL;

  size_t bits_size = size / UNIT_BIT + 1;
  ret->bits = malloc(bits_size);
  assert(ret->bits);
  if (!ret->bits) {
    free((void*)ret);
    return NULL;
  }
  memset(ret->bits, 0, bits_size);

  ret->size = size;

  ++g_bloom_filter_count;
  return ret;
}

void FreeBloomFilter(BloomFilter* filter) {
  if (!filter)
    return;

  if (filter->bits)
    free(filter->bits);

  free((void*)filter);
  --g_bloom_filter_count;
}

void AssertBloomFilterNoLeak() {
  assert(g_bloom_filter_count == 0);
}

void BloomFilterAdd(BloomFilter* filter, const char* str) {
  if (!filter || !filter->bits || !str)
    return;

  Unit* bits = (Unit*)filter->bits;
  for (size_t i = 0; i < N_HASH_FUNC; i++) {
    unsigned hash =
        (unsigned)(g_hash_funcs[i](str, (unsigned)strlen(str)) % filter->size);

    Unit unit = (Unit)(bits[hash / UNIT_BIT] | ((Unit)1 << (hash % UNIT_BIT)));
    bits[hash / UNIT_BIT] = unit;
  }
}

unsigned char BloomFilterTest(BloomFilter* filter, const char* str) {
  if (!filter || !filter->bits || !str)
    return 0;

  Unit* bits = (Unit*)filter->bits;
  for (size_t i = 0; i < N_HASH_FUNC; i++) {
    unsigned hash =
        (unsigned)(g_hash_funcs[i](str, (unsigned)strlen(str)) % filter->size);

    Unit unit = (Unit)(bits[hash / UNIT_BIT] & ((Unit)1 << (hash % UNIT_BIT)));
    if (!unit)
      return 0;
  }
  return 1;
}
