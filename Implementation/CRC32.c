#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
//                                            01011000111011101110111000111000 
//IEEE 802.3 hex def of generation polynom -> 00000100110000010001110110110111
//                                            
#define POLY 0x04C11DB7

unsigned int Table_CRC32[256];

//header start
int CRC4_C_helper(char* data );
int CRC4_C(char* data);
void generateCRC32_Table();
char* decimal_to_binary(int n);
int char_counter (char* data);
void printarray (unsigned int* data);
unsigned int CRC32_C_helper(int data_length , char* data);
unsigned int CRC32_C_helper2(char* data);
char* padding (int data_length , char* data);
char* padding2(char* data);
unsigned int CRC32_ASM(char* message);
unsigned int CRC32_C(char* message);
unsigned int CRC32_C_table(char* data);
unsigned int CRC32_C_TAB(char * message);

//header end 

extern int __CRC32_ASM2__(char* data);
extern int __CRC32_ASM__(char* data);
//get sys current monotonic time 
static inline double curtime(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

////Data padded(32 bits)
unsigned int CRC32_ASM(char* message)
{
//version 1
   // int data_length = char_counter(message);
   // char *padded_data = padding(data_length,message);
   
   //data_length extended by 4 --------
   // data_length += 4;
   // return __CRC32_ASM__(data_length,padded_data);

//version 2
   char *padded_data = padding2(message);
   return __CRC32_ASM2__(padded_data);
}


///////////////////////////////////////////////CRC4//////////////////////////////////////////////////

//IEEE 802.3 hex def of generation polynom -> 10011
#define POLY4 0x13

int CRC4_C(char* data)
{
    // the text message : "SayHellotoMyLittleFriend"
    //encoded to hex : 53 61 79 48 65 6c 6c 6f 74 6f 4d 79 4c 69 74 74 6c 65 46 72 69 65 6e 64
    // char* data = "SayHellotoMyLittleFriend\n";
   return CRC4_C_helper(data);
}
int CRC4_C_helper(char* data )
{
    unsigned char reg = 0; 
    int pos = 0;
    
    while (pos != strlen(data)){

            for(int cur_bit = 7; cur_bit >= 0 ; cur_bit--){
            //test whether the first bit in reg is equal to 1?
                if(((reg >> (4) & 0x0001) == 0x01)){
                    reg = (reg) ^ POLY4;
                    reg &=0x0f;
                }
                //lsl
                reg <<= 1;
                //load data from buffer
                //push up the next bit from the buffer into reg
                reg |= (data[pos] >> cur_bit) & 0x0001;
            }
            //test whether the first bit ==1?
            if(((reg >> (4) & 0x0001) == 0x01)){
                reg = (reg) ^ POLY4;
                reg &=0x0f;
            }
            //padding part only padding last 4 bit
        pos++;
    }

      for(int i = 0 ; i < 4 ; i++){
            reg <<= 1;
      //test whether the first bit in reg is equal to 1?
            if(((reg >> (4) & 0x0001) == 0x01)){
               reg = (reg) ^ POLY4;
               reg &=0x0f;
            }
      }
    return reg;
}



///////////////////////////////////////////////CRC4//////////////////////////////////////////////////

unsigned int CRC32_C(char* message)
{
   //version 1
   // return CRC32_C_helper(char_counter(message),message);
   //version 2
   return CRC32_C_helper2(message);
   //version 3 
   // return CRC32_C_table(message);
}

unsigned int CRC32_C_TAB(char * message)
{
    return CRC32_C_table(message);
}

//padding for 4 Byte "0" 
char* padding (int data_length , char* data)
{
   char* padded_data = (char*) malloc(data_length+4);
    memset(padded_data,0,data_length);
    memcpy(padded_data,data,data_length);
   //  printf("Padded-data length(Bytes) : %lu\n", strnlen(padded_data,strlen(padded_data)+5));
     return padded_data;
}

//trickier than CRC4
unsigned int CRC32_C_helper(int data_length , char* data)
{
    
    char *padded_data = padding(data_length,data);

    //initialize the Register & Byte-position index
    unsigned int reg = 0 ;
    unsigned int pos = 0 ;
    //
    data_length +=4;
    //main loop
    while (pos < data_length){
        for(int cur_bit = 7; cur_bit >= 0 ; cur_bit--){ 
                
                int high = (reg >> 31) & 0x01;
                //lsl
                reg <<= 1;
                //load data from buffer
                //push up the next bit from the buffer into reg
                reg |= ((padded_data[pos]>>cur_bit)&0x00000001);
                // printf("current reg : %s \n ", decimal_to_binary(reg)); 
                if(high == 1){
                    reg = reg ^ POLY;
                    //  printf("Xor reg     : %s \n ", decimal_to_binary(reg));
                }
        }
        // printf("\n");
        pos++;
    }
    // printf("result reg : %s \n ", decimal_to_binary(reg)); 
    return reg;
}

//////implementing the 2.version ///

char* padding2(char* data)
{
   char* padded_data = padding(char_counter(data),data);
   return padded_data;
}

//2. version of CRC without counting the data length...
unsigned int CRC32_C_helper2(char* data)
{
    char* padded_data = padding2(data);
   // printf("Data: %s \nPadded-data length(Bytes) : %lu\n",padded_data ,strlen(padded_data));
    //initialize the Register & Byte-position index
    unsigned int reg = 0 ;
    unsigned int pos = 0 ;
    //
    //main loop
    //stopthe loop when there's no char in the stack
    while (pos != strlen(padded_data)+4){
        for(int cur_bit = 7; cur_bit >= 0 ; cur_bit--){ 
                //highest bit 
                int high = (reg >> 31) & 0x01;
                //lsl
                reg <<= 1;
                //load data from buffer
                //push up the next bit from the buffer into reg
                reg |= ((padded_data[pos]>>cur_bit)&0x00000001);
                // printf("current reg : %s \n ", decimal_to_binary(reg)); 
                if(high == 1){
                    reg = reg ^ POLY;
                    //  printf("Xor reg     : %s \n ", decimal_to_binary(reg));
                }
        }
        // printf("\n");
        pos++;
    }
    // printf("result reg : %s \n ", decimal_to_binary(reg)); 
    return reg;
}


//3.version of CRC using look-up table methord 
unsigned int CRC32_C_table(char* data)
{
   //generation finished 
   unsigned int reg = 0;
   unsigned int pos = 0;
   while (pos != strlen(data)+4){
      unsigned int tablevalue = Table_CRC32[(reg>>24)&0xff];
      reg = (reg<<8)|data[pos];
      reg = reg ^ tablevalue ;
      pos++;
   }
   return reg;
}
//Utility
void  generateCRC32_Table()
{
   // generation of CRC-32 Lookup table
   unsigned int i, j;  
   unsigned long nData32;  
   unsigned int nAccum32;  
   //iteration 00000000 - > 11111111
   for ( i = 0; i < 256; i++ )  
   {
   nData32 = ( unsigned long )( i << 24 );  
   nAccum32 = 0;  
   //for each Byte , bit-wise calc of CRC value 
   for ( j = 0; j < 8; j++ )  
   {
      // int high = sum_poly & 0x80000000;
      // sum_poly <<= 1;
      // if(high) sum_poly =sum_poly^POLY;
      //
      int high = ( nData32 ^ nAccum32 ) & 0x80000000;
      if ( high )  
         nAccum32 = ( nAccum32 << 1 ) ^ POLY;  
      else  
         nAccum32 <<= 1;  
      nData32 <<= 1; 
   }  
   Table_CRC32[i] = nAccum32;  
   }
}
char* decimal_to_binary(int n){
   int c, d, count;
   char *pointer;
   count = 0;
   pointer = (char*)malloc(32+1);
   if (pointer == NULL)
      exit(EXIT_FAILURE); 
   for (c = 31 ; c >= 0 ; c--){
      d = n >> c;
     
      if (d & 1)
         *(pointer+count) = 1 + '0';
      else
         *(pointer+count) = 0 + '0';
     
      count++;
   }
   *(pointer+count) = '\0';
   return  pointer;
}
int char_counter (char* data){
    int out = 0 ;
    while (data[out] != '\0'){
        out++;
    }
    return out;
} 
void printarray (unsigned int* data)
{
   for(int i = 0;i < 256;i++)
   {
       printf("%x ", data[i]);
   }
   printf("\n");
}

int main(int argc, char** argv) {

   if(argc != 3){
      fprintf(stderr,"usage : %s <input string> <iteration time> \n",argv[0]);
      return 1;
   }
    // the text message : "SayHellotoMyLittleFriend"
    //encoded to hex : 53 61 79 48 65 6C 6C 6F 74 6F 4D 79 4C 69 74 74 6C 65 46 72 69 65 6E 64
   //  char* data = "SayHellotoMyLittleFriend\0";
   //  int mals = 10;


////////std io/////////////
   char* data = argv[1];
   int mals = atoi(argv[2]);

//////////////////////////

   // char* data =  "SayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriendSayHellotoMyLittleFriend";
   // printf("data : %s\n", data);
   // printf("data length : %i\n", char_counter(data));
   // for(int i = 0;i < 1;i++)
   // {
   //     strcat(data, data);
   // }

//vars for  time 
float time_crc4 ;
float time_crc32 ;
float time_crc32_tab ;
float time_asm ;

//vars for result
int crc4;
int crc_c;
int crc_c_tab;
int crc_asm;

//vars for time
double start;
double end;
int i;
   printf("program runs %i time(s)\n", mals);
//c

//CRC4 as heuristic 

   start = curtime();
       crc4 = CRC4_C(data);
   end = curtime();
   time_crc4 = end-start;


//CRC32 
   
    start = curtime();
    for(i = 0;i < mals;i++){
        crc_c = CRC32_C(data);
    }
    end = curtime();
    time_crc32 = end-start;

//CRC32 table 
    generateCRC32_Table(); 
   //  printarray(Table_CRC32);

    start = curtime();
    for(i = 0;i < mals;i++)
    {
        crc_c_tab = CRC32_C_TAB(data);
    }
    
    end = curtime();
    time_crc32_tab = end-start;
//timm.knoerle@tum.de
//asm    
   
    start = curtime();
   //  printf("asm time start: %f \n",start);
   // printf("data : %s\n", data);
      for(i = 0;i < mals;i++)
    {
       crc_asm = __CRC32_ASM2__(data);
    }
    end  = curtime();
   //  printf("asm time end: %f \n",end);
    time_asm = end-start;

    printf("CRC4 as heuristic run 1 time\n");
    printf("CRC4_C result : %u \n",crc4);
    printf("CRC4_C time elapsed : %f \n" ,time_crc4);
    printf("CRC4 -end \n");


    printf("CRC32-C result : %u \n",crc_c);
    printf("CRC32-C time elapsed : %f \n" ,time_crc32);

    printf("CRC32-C-table result : %u \n",crc_c_tab);
    printf("CRC32-C-table time elapsed : %f \n" ,time_crc32_tab);

    printf("CRC32-ASM result : %u \n",crc_asm);
    printf("ASM time elapsed : %f \n" ,time_asm);
    

    return 0;
}  



