#include "bit_vector.h"
#include "parameter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
    bit_vector_t* test_vector = bit_vector_new(length);
    char* char_buf = malloc(length*2);
    FILE* file = fopen("origin_bits.txt","r");
    fread(char_buf, 1, 2*length, file);
    for(u_int64_t i=0; i< length;i++){
        if(char_buf[2*i] == '1') bit_vector_set(test_vector, i, 1);
        else bit_vector_set(test_vector, i, 0);
    }
    fclose(file);

    rotate_the_bit_vector(test_vector, bit_offset, bit_length, rotate_num);
    file = fopen("check_out_bits.txt","w");
    for(u_int64_t i=0; i< length;i++){
        if(bit_vector_get(test_vector, i))
        char_buf[2*i] = '1';
        else char_buf[2*i] = '0';
        char_buf[2*i+1]='\n';
    }
    fwrite(char_buf, 1, 2*length, file);
    fclose(file);
}