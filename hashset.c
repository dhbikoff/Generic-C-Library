#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const int defaultVectorAlloc = 4; // vector allocation size
static const int kNotFound = -1; // elem not found sentinel

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn,
		HashSetFreeFunction freefn)
{
  assert(elemSize > 0 && numBuckets > 0 && hashfn != NULL && 
         comparefn != NULL);

  h->elemSize = elemSize;
  h->numBuckets = numBuckets;
  h->elemCount = 0;
  h->hashfn = hashfn;
  h->compfn = comparefn;
  h->freefn = freefn;

  h->buckets = malloc(sizeof(vector) * numBuckets);

  for (int i = 0; i < numBuckets; i++) {
    void *srcAddr = (char*)h->buckets + i * sizeof(vector);
    VectorNew(srcAddr, elemSize, freefn, defaultVectorAlloc); // init vectors
  }
}

void HashSetDispose(hashset *h)
{
  for (int i = 0; i < h->numBuckets; i++) {
    VectorDispose(&h->buckets[i]);
  }

  free(h->buckets);
}

int HashSetCount(const hashset *h)
{
  return h->elemCount;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
  assert(mapfn != NULL);

  for (int i = 0; i < h->numBuckets; i++) {
    VectorMap(&h->buckets[i],mapfn,auxData);
  }
}

/**
* Returns an integer computed by the hash function.
**/
static int GetHashNum(const hashset *h, const void *elemAddr)
{
  assert(elemAddr != NULL);

  int hashNum = h->hashfn(elemAddr,h->numBuckets);
  assert(hashNum >= 0 && hashNum < h->numBuckets);
  return hashNum;
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
  int hashNum = GetHashNum(h,elemAddr);

  int index = VectorSearch(&h->buckets[hashNum],elemAddr,h->compfn,0,false);

  if (index == kNotFound) {
    VectorAppend(&h->buckets[hashNum],elemAddr); // add elem
    h->elemCount++;
  } else {
    // overwrite existing element
    VectorReplace(&h->buckets[hashNum], elemAddr, index);
  }
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
  int hashNum = GetHashNum(h,elemAddr);

  VectorSort(&h->buckets[hashNum], h->compfn);
  int index = VectorSearch(&h->buckets[hashNum],elemAddr,h->compfn,0,true);

  if (index == kNotFound) {
    return NULL;
  } else {
    return VectorNth(&h->buckets[hashNum],index); // elem address
  }
}

