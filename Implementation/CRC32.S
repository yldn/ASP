.global __CRC32_ASM2__
.global __CRC32_ASM__ 


//ignored table generation
.data
CRCtable:
    //DB 0x0 0x4c11db7 0x9823b6e 0xd4326d9 0x130476dc 0x17c56b6b 0x1a864db2 0x1e475005 0x2608edb8 0x22c9f00f 0x2f8ad6d6
    //.asciz "hello world\n"
len = .-CRCtable

.text

//extern int __CRC32_ASM2__(char* data);
//extern int CRC32_ASM__(char* data);
//regs refs.
/*
    x0=data_length
    x1=sp
    x2=POLY : 0x04C11DB7
    x3=
*/

//first approach by implementing the CRC standard-method ,realizing that already in c file XD
//__CRC32_ASM__: deprecated 
__CRC32_ASM__:
    //load POLY 0x04C11DB7
    //move 16 bit keep unchanged .
    //smovk x2,0x04C1,lsl 16
    //movk x2,0x1DB7

    //sub x0,x0,1
    //cbz x0,.Lret    //brunch to return if 
    //b   __CRC32_ASM__
/*
    x0=sp
    x1 = table  
    x2=current data === data [pos]
    x3= stop flag for 
    x4= reg
    x5= reg >> 24 
    x6= table [reg >> 24 && 0xff]
    x7= 4 for looping in the end
    ...
    
*/
__CRC32_ASM2__:
//16 byte aligned 
    sub sp,sp,16
    str x19,[sp]
//clear work space
    mov x4 , 0 
    mov x5 , 0
    //load table data-pointer to x19 (callee - save)
    //store the x19 value to x1 for later on use 

    adr x1 , Table_CRC32
// SIMD
   // b __CRC32_ASM_SIMD__

//version 1 ,plain asm
.mainloop:
    ldrb w2, [x0],1  // Post-increment: adds 1 to x0 after loading
    cbz w2, .Lpad  // Branch to return if char is '\0'

    
    mov x5, x4 ,lsr 24
    and x5, x5 ,0xff
    //load table value to w6
    //convert byte to int ( need *4 )
    ldr w6,[x1,x5,lsl 2]
    //adding next byte data from the pointer
    orr x4,x2,x4,lsl 8 
    eor x4,x4,x6 

    b .mainloop
    
.Lpad:
    mov w7 , 4
    mov w2 , 0
.padloop:
    sub w7 ,w7, 1

    mov x5, x4 ,lsr 24
    and x5, x5 ,0xff
    //load table value to x6
    //convert byte to int ( need *4 )
    ldr w6,[x1,x5,lsl 2]
    //adding next byte data from the pointer
    orr x4,x2,x4,lsl 8
    eor x4,x4,x6

    cbnz w7 , .padloop

.Lret:
    mov x0,x4
    ldr x19,[sp],16
    ret




///we figured out it's impossible to optimize the program in SIMD because the method we figured sofar
//need the previous result of the datablock ,we think firstly that load the buffer into the q-register 
//then calc the result by checking the previous result ,and we figured out that this measurement doesn't 
//have more efficient result than plain assembly as described above , so we concluded that i't not possible 
//to make it in SIMD structure.
//
/*
__CRC32_ASM_SIMD__:
    //q0 = data[x0]
    //q1 = flags to check wether the data breaks

    //q2 = table value of v1.b[15]/v1.b[11]/v1.b[7]/v1.b[3]
    //q3 = 
    //q4 = 
    //q5 = 
    //q6 = reg1/reg2/reg3/reg4
    //q7 = reg1>>
//we add 16 later on 
/////
    ldr q0,[x0] 
    //compare and memorise byte in v0 equals 0 into v1
    cmeq v1.16b, v0.16b, 0
    //check where 0xff is and mov to general porpose
    umaxv b1, v1.16b
    umov w3 ,v1.b[0]
    //stop cond.
    cbnz w3 , .mainloop
    
.simd_CRC:

//load table value from the table pointer x1.
    ldr v2.s[3],[x1,v0.b[15],lsl 2]
    ldr v2.s[2],[x1,v0.b[11],lsl 2]
    ldr v2.s[1],[x1,v0.b[7],lsl 2]
    ldr v2.s[0],[x1,v0.b[3],lsl 2]

.load_nextbyte
//q7 for next byte
    mov v7.b[12], v0.b[15]
    mov v7.b[8], v0.b[11]
    mov v7.b[4], v0.b[7]
    mov v7.b[0], v0.b[3]

//q0 shift 1 byte to the front
    shl v0.4s,v0.4s,8

    umov w3 ,v0.b[15]
    cbnz w3 ,.load_nextbyte

ret

*/


