/**
 * Copyright (c) 2012 MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 **/

// Implements the ADT specified in bit_vector.h as a packed array of bits; a bit
// array containing bit_sz bits will consume roughly bit_sz/8 bytes of
// memory.


#include "./bit_vector.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sys/types.h>

// Many programming languages define modulo in a manner incompatible with its
// widely-accepted mathematical definition.
// http://stackoverflow.com/questions/1907565/c-python-different-behaviour-of-the-modulo-operation
// provides details; in particular, C's modulo
// operator (which the standard calls a "remainder" operator) yields a result
// signed identically to the dividend e.g., -1 % 10 yields -1.
// This is obviously unacceptable for a function which returns size_t, so we
// define our own.
//
// n is the dividend and m is the divisor
//
// Returns a positive integer r = n (mod m), in the range
// 0 <= r < m.
static size_t modulo(const ssize_t n, const size_t m);

// Produces a mask which, when ANDed with a byte, retains only the
// bit_index th byte.
//
// Example: bitmask(5) produces the byte 0b00100000.
//
// (Note that here the index is counted from right
// to left, which is different from how we represent bit_vectors in the
// tests.  This function is only used by bit_vector_get and bit_vector_set,
// however, so as long as you always use bit_vector_get and bit_vector_set
// to access bits in your bit_vector, this reverse representation should
// not matter.
static char bitmask(const size_t bit_index);

// ******************************* Functions ********************************

bit_vector_t* bit_vector_new(const size_t bit_sz) {
  // Allocate an underlying buffer of ceil(bit_sz/8) bytes.
  char* const buf = calloc(1, (bit_sz+7) / 8);
  if (buf == NULL) {
    return NULL;
  }

  // Allocate space for the struct.
  bit_vector_t* const bit_vector = malloc(sizeof(struct bit_vector));
  if (bit_vector == NULL) {
    free(buf);
    return NULL;
  }

  bit_vector->buf = buf;
  bit_vector->bit_sz = bit_sz;
  return bit_vector;
}

void bit_vector_free(bit_vector_t* const bit_vector) {
  if (bit_vector == NULL) {
    return;
  }
  free(bit_vector->buf);
  bit_vector->buf = NULL;
  free(bit_vector);
}

size_t bit_vector_get_bit_sz(const bit_vector_t* const bit_vector) {
  return bit_vector->bit_sz;
}

bool bit_vector_get(const bit_vector_t* const bit_vector, const size_t bit_index) {
  assert(bit_index < bit_vector->bit_sz);

  // We're storing bits in packed form, 8 per byte.  So to get the nth
  // bit, we want to look at the (n mod 8)th bit of the (floor(n/8)th)
  // byte.
  //
  // In C, integer division is floored explicitly, so we can just do it to
  // get the byte; we then bitwise-and the byte with an appropriate mask
  // to produce either a zero byte (if the bit was 0) or a nonzero byte
  // (if it wasn't).  Finally, we convert that to a boolean.
  return (bit_vector->buf[bit_index / 8] & bitmask(bit_index)) ?
         true : false;
}

void bit_vector_set(bit_vector_t* const bit_vector,
                  const size_t bit_index,
                  const bool value) {
  assert(bit_index < bit_vector->bit_sz);

  // We're storing bits in packed form, 8 per byte.  So to set the nth
  // bit, we want to set the (n mod 8)th bit of the (floor(n/8)th) byte.
  //
  // In C, integer division is floored explicitly, so we can just do it to
  // get the byte; we then bitwise-and the byte with an appropriate mask
  // to clear out the bit we're about to set.  We bitwise-or the result
  // with a byte that has either a 1 or a 0 in the correct place.
  bit_vector->buf[bit_index / 8] =
    (bit_vector->buf[bit_index / 8] & ~bitmask(bit_index)) |
    (value ? bitmask(bit_index) : 0);
}

void bit_vector_randfill(bit_vector_t* const bit_vector){
  int32_t *ptr = (int32_t *)bit_vector->buf;
  for (int64_t i=0; i<bit_vector->bit_sz/32 + 1; i++){
    ptr[i] = rand();
  }
}

static size_t modulo(const ssize_t n, const size_t m) {
  const ssize_t signed_m = (ssize_t)m;
  assert(signed_m > 0);
  const ssize_t result = ((n % signed_m) + signed_m) % signed_m;
  assert(result >= 0);
  return (size_t)result;
}

static char bitmask(const size_t bit_index) {
  return 1 << (bit_index % 8);
}

