#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
////////////////////////////this example is used as an inspiration & Appitizer for the project//////////////////

//IEEE 802.3 hex def of generation polynom -> 10011
#define POLY 0x13


int CRC4_C_helper(int data_length,char* data );
int char_counter (char* data);

//get sys current monotonic time 
static inline double curtime(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}


int CRC4_C(){
    // the text message : "SayHellotoMyLittleFriend"
    //encoded to hex : 53 61 79 48 65 6c 6c 6f 74 6f 4d 79 4c 69 74 74 6c 65 46 72 69 65 6e 64
    // char* data = "SayHellotoMyLittleFriend\n";
    char* data = "Say\n";
    printf("%s \n",data);

   return CRC4_C_helper(char_counter(data),data);
}

int char_counter (char* data)
{
    int out = 0 ;
    while (data[out] != '\n'){
        out++;
    }
    return out;
} 

int CRC4_C_helper(int data_length,char* data )
{
    unsigned char reg = 0x0000; 
    int pos = 0;
    unsigned char padded_data[data_length+1];
     //padding 0 * 4
    memset(padded_data,0,data_length+1);
    memcpy(padded_data,data,data_length);
    printf("Data: %s \ndata length(Bytes) : %lu\n",padded_data, sizeof (padded_data));
    data_length +=1;
    while (pos < data_length){
        if(pos == data_length-1){
             for(int i = 0 ; i < 4 ; i++){
                reg <<= 1;
                // printf("reg : %c\n" , reg);
            //test whether the first bit in reg is equal to 1?
                if(((reg >> (4) & 0x0001) == 0x01)){
                    reg = reg ^ POLY;
                }
             }
        }else {
            for(int cur_bit = 7; cur_bit >= 0 ; cur_bit--){
            //test whether the first bit in reg is equal to 1?
                if(((reg >> (4) & 0x0001) == 0x01)){
                    reg = reg ^ POLY;
                }
                //lsl
                reg <<= 1;
                //load data from buffer
                //push up the next bit from the buffer into reg
                reg |= (padded_data[pos] >> cur_bit) & 0x0001;
            }
            if(((reg >> (4) & 0x0001) == 0x01)){
                reg = reg ^ POLY;
            }
            // printf("pos : %d\n" , pos);
            //padding part only padding last 4 bit
            
        }
        // if(((reg >> (4) & 0x0001) == 0x01)){
        //     reg = reg ^ POLY;
        // }
    
        pos++;
    }
    return reg;
}

int main(int argc, char** argv) {
    int result =CRC4_C();;
    double start = curtime();
    // printf("start time  : %f \n" , start);
    for(int i = 0;i < 10;i++)
    {
        // printf("gg");
       CRC4_C();
    }
    
    double end = curtime();

        printf("%d \n",result);
    printf("time elapsed : %f \n" , end - start);

    return 0;
}  