#include "bit_vector.h"
#include "parameter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
    
    bit_vector_t* test_vector = bit_vector_new(length);
    bit_vector_randfill(test_vector);
    char* char_buf = malloc(2*length);
    FILE* file = fopen("origin_bits.txt","w");
    for(u_int64_t i=0; i< length;i++){
        if(bit_vector_get(test_vector, i))
        char_buf[2*i] = '1';
        else char_buf[2*i] = '0';
        char_buf[2*i+1] = '\n';
    }
    fwrite(char_buf, 1, 2*length, file);
    fclose(file);

    rotate_the_bit_vector(test_vector, bit_offset, bit_length, rotate_num);
    file = fopen("out_bits.txt","w");
    for(u_int64_t i=0; i< length;i++){
        if(bit_vector_get(test_vector, i))
        char_buf[2*i] = '1';
        else char_buf[2*i] = '0';
        char_buf[2*i+1] = '\n';
    }
    fwrite(char_buf, 1, 2*length, file);
    fclose(file);
}