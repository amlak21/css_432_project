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
 
char* create_ACK_packet_reciever(int block)
{
    char *buffer = malloc(4);
    bzero(buffer, sizeof(buffer));
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = ntohs(OP_CODE_ACK); // add opcode data at empty buffer
    opCodePtr++; 
    unsigned short blockNum = (unsigned short)block; //?
    unsigned short *blockNumPtr = opCodePtr;
    *blockNumPtr = ntohs(blockNum); 
    return buffer;
}


char* create_WRQ_packet(char* input_file)
{
   char mode[] = "octet";
    char *buffer = malloc (4 + strlen(input_file)+ strlen(mode));
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
    
    //char mode[] = "octet";
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


char* create_data_packet(int block, char* input_file)
{
     char *buffer = malloc(MAX_BUFFER_SIZE_DP);
    bzero(buffer, sizeof(buffer));
    
   // unsigned short opCode = OP_CODE_DATA;
   // cout<< "size of unsigned short: " << sizeof(opCode) << " bytes." << endl;
    
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = htons(OP_CODE_DATA); // add opcode data at empty buffer
    opCodePtr++; //pointing to 3rd byte.
    
    unsigned short blockNum = (unsigned short)block;
    unsigned short *blockNumPtr = opCodePtr;
    *blockNumPtr = htons(blockNum); // add block number to buffer
    
    
    char *file_data = buffer + DATA_OFFSET; //point to the 5th offset
    char *file = input_file;
    strncpy (file_data, file, strlen(file)); // bcopy , memcpy

    return buffer;
}


char* create_data_packet_reciever(int block, char* input_file)
{
     char *buffer = malloc(MAX_BUFFER_SIZE_DP);
    bzero(buffer, sizeof(buffer));
    
   // unsigned short opCode = OP_CODE_DATA;
   // cout<< "size of unsigned short: " << sizeof(opCode) << " bytes." << endl;
    
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = ntohs(OP_CODE_DATA); // add opcode data at empty buffer
    opCodePtr++; //pointing to 3rd byte.
    
    unsigned short blockNum = (unsigned short)block;
    unsigned short *blockNumPtr = opCodePtr;
    *blockNumPtr = ntohs(blockNum); // add block number to buffer
    
    
    char *file_data = buffer + DATA_OFFSET; //point to the 5th offset
    char *file = input_file;
    strncpy (file_data, file, strlen(file)); // bcopy , memcpy

    return buffer;
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