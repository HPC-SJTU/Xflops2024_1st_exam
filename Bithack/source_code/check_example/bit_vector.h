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

#ifndef bit_vector_H
#define bit_vector_H

#include <sys/types.h>
#include <stdbool.h>

// ********************************* Types **********************************

// Abstract data type representing an array of bits.
typedef struct bit_vector bit_vector_t;

// Concrete data type representing an array of bits.
struct bit_vector {
  // The number of bits represented by this bit array.
  // Need not be divisible by 8.
  size_t bit_sz;

  // The underlying memory buffer that stores the bits in
  // packed form (8 per byte).
  char* buf;
};

// ******************************* Prototypes *******************************

// Allocates space for a new bit array.
// bit_sz is the number of bits storable in the resultant bit array
bit_vector_t* bit_vector_new(const size_t bit_sz);

// Frees a bit array allocated by bit_vector_new.
void bit_vector_free(bit_vector_t* const bit_vector);

// Returns the number of bits stored in a bit array.
// Note the invariant bit_vector_get_bit_sz(bit_vector_new(n)) = n.
size_t bit_vector_get_bit_sz(const bit_vector_t* const bit_vector);

// Does a random fill of all the bits in the bit array.
void bit_vector_randfill(bit_vector_t* const bit_vector);

// Indexes into a bit array, retreiving the bit at the specified zero-based
// index.
bool bit_vector_get(const bit_vector_t* const bit_vector, const size_t bit_index);

// Indexes into a bit array, setting the bit at the specified zero-based index.
void bit_vector_set(bit_vector_t* const bit_vector,
                  const size_t bit_index,
                  const bool value);



// Rotates a subarray.
//
// bit_offset is the index of the start of the subarray
// bit_length is the length of the subarray, in bits
// bit_right_amount is the number of places to rotate the subarray right
//
// The subarray spans the half-open interval
// [bit_offset, bit_offset + bit_length)
// That is, the start is inclusive, but the end is exclusive.
//
// Note: bit_right_amount can be negative, in which case a left rotation is
// performed.
//
// Example:
// Let ba be a bit array containing the byte 0b10010110; then,
// rotate_the_bit_vector(ba, 0, bit_vector_get_bit_sz(ba), -1)
// left-rotates the entire bit array in place.  After the rotation, ba
// contains the byte 0b00101101.
//
// Example:
// Let ba be a bit array containing the byte 0b10010110; then,
// rotate_the_bit_vector(ba, 2, 5, 2) rotates the third through seventh
// (inclusive) bits right two places.  After the rotation, ba contains the
// byte 0b10110100.
void rotate_the_bit_vector(bit_vector_t* const bit_vector,
                     const size_t bit_offset,
                     const size_t bit_length,
                     const ssize_t bit_right_amount);

#endif  // bit_vector_H
