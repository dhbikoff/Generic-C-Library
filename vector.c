#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <assert.h>

static const int defaultVectorAlloc = 4; // default allocation value
static const int kNotFound = -1; // not found sentinel

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
  if (initialAllocation == 0) {
    initialAllocation = defaultVectorAlloc;
  }

  assert(elemSize > 0);
  assert(initialAllocation > 0);

  v->elemSize = elemSize;
  v->allocLength = initialAllocation;
  v->logLength = 0;

  v->elems = malloc(initialAllocation * elemSize);
  assert(v->elems != NULL);

  v->freefn = freeFn;
}

void VectorDispose(vector *v)
{
  if (v->freefn != NULL) {
    for (int i = 0; i < v->logLength; i++) {
      v->freefn(VectorNth(v,i));
    }
  }

  free(v->elems);
}

static void DoubleCapacity(vector *v)
{
  v->allocLength *= 2;
  v->elems = realloc(v->elems, v->allocLength * v->elemSize);
  assert(v->elems != NULL);
}

int VectorLength(const vector *v)
{ 
  return v->logLength; 
}

void *VectorNth(const vector *v, int position)
{
  assert(position < v->logLength && position >= 0);
  
  void *destAddr = (char*)v->elems + (position * v->elemSize);
  return destAddr; 
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
  assert(position >= 0 && position < v->logLength);

  void *destAddr = (char*)v->elems + (position * v->elemSize); 

  if (v->freefn != NULL) {
    v->freefn(destAddr); // free old elem
  }

  memcpy(destAddr,elemAddr,v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
  assert(position >= 0 && position <= v->logLength);

  if (position == v->logLength) {
    VectorAppend(v, elemAddr);
    return;
  }

  if (v->logLength == v->allocLength) {
    DoubleCapacity(v);
  }

  void *startShiftAddr = (char*)v->elems + (position * v->elemSize);
  // shift one elem over to insert
  void *endShiftAddr = (char*)v->elems + ((position+1) * v->elemSize);
  int blockSize = (v->logLength - position) * v->elemSize;

  memmove(endShiftAddr,startShiftAddr,blockSize); // shift memory over
  memcpy(startShiftAddr,elemAddr,v->elemSize); // insert elem

  v->logLength++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
  if (v->logLength == v->allocLength) {
    DoubleCapacity(v);
  }
  
  void *destAddr = (char*)v->elems + v->logLength * v->elemSize;
  memcpy(destAddr, elemAddr, v->elemSize); // add to end of vector
  v->logLength++;
}

void VectorDelete(vector *v, int position)
{
  assert(position >= 0 && position < v->logLength);

  if (position == v->logLength) { 
    v->logLength--;
    return;
  }

  // one elem to the right
  void *startShiftAddr = (char*)v->elems + ((position+1) * v->elemSize);
  // elem address to delete
  void *endShiftAddr = (char*)v->elems + (position * v->elemSize);
  // size of memory block to move
  int blockSize = (v->logLength - position - 1) * v->elemSize;

  if (v->freefn != NULL) {
    v->freefn(endShiftAddr); // free deleted elem
  }

  memmove(endShiftAddr,startShiftAddr,blockSize);
  
  v->logLength--;  
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
  assert(compare != NULL);
  qsort(v->elems,v->logLength,v->elemSize,compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
  assert(mapFn != NULL);
  for (int i = 0; i < v->logLength; i++) {
    void *currAddr = (char*)v->elems + (i * v->elemSize);
    mapFn(currAddr,auxData);
  }
}

/**
* Calculates index from the difference between found address and start address.
**/
static int GetIndex(const vector *v, void *found, int startIndex)
{
  void *startAddr = (char*)v->elems + (startIndex * v->elemSize);

  int start = (int)startAddr;
  int foundAddr = (int)found;
  int diff = (foundAddr - start) / v->elemSize;
  return diff;
}

static void *BinarySearchVector(const vector *v, const void *key,
                                VectorCompareFunction searchFn, int startIndex,
                                bool isSorted, size_t size)
{
  return bsearch(key,(char*)v->elems + (startIndex * v->elemSize),
                 size,v->elemSize,searchFn);
}

static void *LinearSearchVector(const vector *v, const void *key,
                                VectorCompareFunction searchFn, int startIndex,
                                bool isSorted, size_t size)
{
  return lfind(key,(char*)v->elems + (startIndex * v->elemSize),
               &size,v->elemSize,searchFn);
}

int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, 
                 int startIndex, bool isSorted)
{
  assert(searchFn != NULL || key != NULL);

  void *found; // address of found element
  size_t size = v->logLength - startIndex; // size of block to test

  if (isSorted) {
    found = BinarySearchVector(v,key,searchFn,startIndex,isSorted,size);
  } else {
    found = LinearSearchVector(v,key,searchFn,startIndex,isSorted,size);
  }

  if (found == NULL) {
    return kNotFound;
  } else {
    return GetIndex(v, found, startIndex);
  }
} 

