#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>          // for retrieving the error number.
#include <string.h>         // for strerror function.
#include <signal.h>         // for the signal handler registration.
#include <unistd.h>

#define SERV_UDP_PORT 51091 //PORT NUMBER 
#define MAX_NUM_PACKETS 25  // maximum number of packets

// constants for data packet
const static int MAX_BUFFER_SIZE_DP = 516;
const static unsigned short OP_CODE_DATA = 3;
const static int DATA_OFFSET = 4; //

//constants for RRQ and WRQ paket
const static unsigned short OP_CODE_RRQ = 1;
const static int OPCODE_OFFSET = 2; //
const static char MODE[] = "octet";
const static unsigned short OP_CODE_WRQ = 2;

// costants for ACK packet
const static unsigned short OP_CODE_ACK = 4;

// constants for Error packet
const static unsigned short OP_CODE_ERR = 5;

char* create_ACK_packet(int block)
{
    char *buffer = malloc(4);
    bzero(buffer, sizeof(buffer));
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = htons(OP_CODE_ACK); // add opcode data at empty buffer
    opCodePtr++; 
    unsigned short blockNum = (unsigned short)block; //?
    unsigned short *blockNumPtr = opCodePtr;
    *blockNumPtr = htons(blockNum); 
    return buffer;
}

char* create_WRQ_packet(char* input_file)
{
    char *buffer = malloc (4 + strlen(input_file)+ strlen(MODE));
    bzero(buffer, sizeof(buffer));
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = htons(OP_CODE_WRQ); // add opcode data at empty buffer
 
    char* file_string = buffer + OPCODE_OFFSET; // point at 3rd byte

    // concatinate input file, zero, and mode.
    strcat(file_string, input_file);
    strcat(file_string, "0");
    strcat(file_string, MODE);
    return buffer;
}


char* create_RRQ_packet(char* input_file)
{
    char *buffer = malloc(9 + strlen(input_file));
    bzero(buffer, sizeof(buffer));
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = htons(OP_CODE_RRQ); // add opcode data at empty buffer

    char* file_string = buffer + OPCODE_OFFSET; // point at 3rd byte

    // concatinate input file, zero, and mode.
    strcat(file_string, input_file);
    strcat(file_string, "0");
    strcat(file_string, MODE);
    return buffer; 
}

// create packet from one data blcok size file
// input is one data block size file <=512 bytes
char* create_data_packet(int block, char* one_data_file)
{
     char *buffer = malloc(MAX_BUFFER_SIZE_DP);
    bzero(buffer, sizeof(buffer));
    
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = htons(OP_CODE_DATA); // add opcode data at empty buffer
    opCodePtr++; //pointing to 3rd byte.
    
    unsigned short blockNum = (unsigned short)block;
    unsigned short *blockNumPtr = opCodePtr;
    *blockNumPtr = htons(blockNum); // add block number to buffer
     
    char *file_data = buffer + DATA_OFFSET; //point to the 5th offset
    char *file = one_data_file;
    strncpy (file_data, file, strlen(file)); // bcopy , memcpy

    return buffer;
}
// for data file
    //store the input file in some buffer
    // read the next 512 byte(untill last ) and return one data packet size file <= 512
char* get_one_packet_data(char* input_file) // need to be tested
{
    char* one_data_buffer = malloc(512);

    
    if(sizeof(input_file) > 512)
    {
        // copy 512 bytes
        bzero(one_data_buffer, sizeof(one_data_buffer));
        memcpy (one_data_buffer, input_file, 512);
    }
    else if(sizeof(input_file) < 512)
    {
        // copy size of large buffer
        bzero(one_data_buffer, sizeof(one_data_buffer));
        memcpy(one_data_buffer, input_file, sizeof(input_file));
    }
    // return 512 bytes
    return one_data_buffer;
}
char* create_ERR_packet(char* err_code, char* err_msg)
{
    char *buffer = malloc (5 + strlen(err_msg));
    bzero(buffer, sizeof(buffer));
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = htons(OP_CODE_ERR); // add opcode data at empty buffer
    
    char* file_string = buffer + OPCODE_OFFSET; // point at 3rd byte
    strcat(file_string, err_code);
 strcat(file_string, err_msg);
    return buffer;
}

// helper functions at reciever side
// function to get opcode from recieved packet
unsigned short get_opcode(char* packet)
{
    unsigned short *opCodePtrRcv = (unsigned short*) packet;
    unsigned short opCodeRcv = ntohs(*opCodePtrRcv);
    return opCodeRcv; // return opcode value
}

char* get_file_data(char* data_packet) 
{
    char* buffer = data_packet;
    char* file_ptr = buffer + DATA_OFFSET; // point at 5th offset
    char *file = malloc(sizeof(file_ptr));
    memcpy (file, file_ptr, sizeof(file_ptr));
    return file; // return the actual file data
}

// for data and acknoledegment packet
unsigned short get_block_number(char* packet)
{
    unsigned short *block_num_ptr = (unsigned short*) packet + 1; 
    unsigned short block_num = ntohs(*block_num_ptr);
    return block_num; // return block number
}
// implement
// get error code 
// get error mesg