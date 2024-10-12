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


// ******************** Prototypes for static functions *********************

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

// Rotates a subarray left by an arbitrary number of bits.
//
// bit_offset is the index of the start of the subarray
// bit_length is the length of the subarray, in bits
// bit_left_amount is the number of places to rotate the
//                    subarray left
//
// The subarray spans the half-open interval
// [bit_offset, bit_offset + bit_length)
// That is, the start is inclusive, but the end is exclusive.
static void rotate_the_bit_vector_left(bit_vector_t* const bit_vector,
                                 const size_t bit_offset,
                                 const size_t bit_length,
                                 const size_t bit_left_amount);

// Rotates a subarray left by one bit.
//
// bit_offset is the index of the start of the subarray
// bit_length is the length of the subarray, in bits
//
// The subarray spans the half-open interval
// [bit_offset, bit_offset + bit_length)
// That is, the start is inclusive, but the end is exclusive.
static void rotate_the_bit_vector_left_one(bit_vector_t* const bit_vector,
                                     const size_t bit_offset,
                                     const size_t bit_length);

// ******************************* Functions ********************************



void rotate_the_bit_vector(bit_vector_t* const bit_vector,
                     const size_t bit_offset,
                     const size_t bit_length,
                     const ssize_t bit_right_amount) {
  assert(bit_offset + bit_length <= bit_vector->bit_sz);

  if (bit_length == 0) {
    return;
  }

  // Convert a rotate left or right to a left rotate only, and eliminate
  // multiple full rotations.
  rotate_the_bit_vector_left(bit_vector, bit_offset, bit_length,
                       modulo(-bit_right_amount, bit_length));
}

static void rotate_the_bit_vector_left(bit_vector_t* const bit_vector,
                                 const size_t bit_offset,
                                 const size_t bit_length,
                                 const size_t bit_left_amount) {
  for (size_t i = 0; i < bit_left_amount; i++) {
    rotate_the_bit_vector_left_one(bit_vector, bit_offset, bit_length);
  }
}

static void rotate_the_bit_vector_left_one(bit_vector_t* const bit_vector,
                                     const size_t bit_offset,
                                     const size_t bit_length) {
  // Grab the first bit in the range, shift everything left by one, and
  // then stick the first bit at the end.
  const bool first_bit = bit_vector_get(bit_vector, bit_offset);
  size_t i;
  for (i = bit_offset; i + 1 < bit_offset + bit_length; i++) {
    bit_vector_set(bit_vector, i, bit_vector_get(bit_vector, i + 1));
  }
  bit_vector_set(bit_vector, i, first_bit);
}
