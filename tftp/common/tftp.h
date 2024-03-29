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
#include <sys/wait.h>



#define SERV_UDP_PORT 51091223 //PORT NUMBER

//#define MAX_FILE_SIZE 12000000 //12 MB 


// constants for data packet
const static int MAX_BUFFER_SIZE_DP = 516;
const static unsigned short OP_CODE_DATA = 3;
const static int DATA_OFFSET = 4; //
const static int MAX_DATA_SIZE = 512;

//constants for RRQ and WRQ paket
const static unsigned short OP_CODE_RRQ = 1;
const static int OPCODE_OFFSET = 2; //
const static char MODE[] = "octet";
const static unsigned short OP_CODE_WRQ = 2;
const static int FILE_NAME_BYTES = 50; //file name up to 50 bytes of length

// costants for ACK packet
const static unsigned short OP_CODE_ACK = 4;

// constants for Error packet
const static unsigned short OP_CODE_ERR = 5;




// functions

//to create acknowledge packet
char* create_ACK_packet(int block)
{
    char *buffer = malloc(4);
    bzero(buffer, strlen(buffer)); 
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = htons(OP_CODE_ACK); // add opcode data at empty buffer
    opCodePtr++; 
    unsigned short blockNum = (unsigned short)block; //?
    unsigned short *blockNumPtr = opCodePtr;
    *blockNumPtr = htons(blockNum); 
    return buffer;
}

//to create write request packet
char* create_WRQ_packet(char* input_file)
{
    char *buffer = malloc (4 + strlen(input_file)+ strlen(MODE));
    bzero(buffer, strlen(buffer));
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = htons(OP_CODE_WRQ); // add opcode data at empty buffer
	
    char* file_string = buffer + OPCODE_OFFSET; // point at 3rd byte

    // concatinate input file, zero, and mode.
    strcat(file_string, input_file);
    strcat(file_string, "0");
    strcat(file_string, MODE);
    return buffer;
}

//to create read request packet
char* create_RRQ_packet(char* input_file)
{
    char *buffer = malloc(9 + strlen(input_file));
    bzero(buffer, strlen(buffer));
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = htons(OP_CODE_RRQ); // add opcode data at empty buffer

    char* file_string = buffer + OPCODE_OFFSET; // point at 3rd byte

    // concatinate input file, zero, and mode.
    strcat(file_string, input_file);
    strcat(file_string, "0");
    strcat(file_string, MODE);
    return buffer; 
}

// to create data packet
// input is one data block size file <=512 bytes
char* create_data_packet(int block, char* one_data_file)
{
    char *buffer = malloc(MAX_BUFFER_SIZE_DP);
    bzero(buffer, MAX_BUFFER_SIZE_DP);

    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = htons(OP_CODE_DATA); // add opcode data at empty buffer
    opCodePtr++; //pointing to 3rd byte.
    
    unsigned short blockNum = (unsigned short)block;
    unsigned short *blockNumPtr = opCodePtr;
    *blockNumPtr = htons(blockNum); // add block number to buffer
     
    char *file_data = buffer + DATA_OFFSET; //point to the 5th offset
    char *file = one_data_file;
    
    memcpy(file_data, file, strlen(file)); 
    return buffer;
}

// to get only up to 512 bytes of data from the large file
// helper function to create a data packet
char* get_one_packet_data(char* input_file) // tested
{
    char* file = input_file;
   char *buffer = malloc(MAX_DATA_SIZE + 1);
    if(strlen(file) > MAX_DATA_SIZE) 
    {
        bzero(buffer, MAX_DATA_SIZE + 1);
        memcpy (buffer, file, MAX_DATA_SIZE);   // copy only 512 bytes
    }

    else if(strlen(file) <= MAX_DATA_SIZE)
    {
        bzero(buffer, MAX_DATA_SIZE + 1);
        memcpy(buffer, file, strlen(file)+ 1);  // copy all of bytes
    }
    return buffer; // return one packet size data

}

// to create error packet
//to be used by server when need to send error packet
char* create_ERR_packet(int err_code, char* err_msg)
{
    char *buffer = malloc (5 + strlen(err_msg));
    bzero(buffer, strlen(buffer));
    unsigned short *opCodePtr = (unsigned short*) buffer; //point at empty buffer
    *opCodePtr = htons(OP_CODE_ERR); // add opcode data at empty buffer
     opCodePtr++; //pointing to 3rd byte.

    unsigned short err_code_num = (unsigned short)err_code;
    unsigned short *err_code_ptr = opCodePtr;
    *err_code_ptr = htons(err_code_num); // add error code to buffer
    
    char* file_string = buffer + DATA_OFFSET; // point at 5th byte
    //strcat(file_string, err_code);
	strcat(file_string, err_msg);
    return buffer;
}

// helper functions at reciever side
// to get the opcode from the recived packet
//help to determine what type of packet is being received
unsigned short get_opcode(char* packet)
{
    unsigned short *opCodePtrRcv = (unsigned short*) packet;
    unsigned short opCodeRcv = ntohs(*opCodePtrRcv);
    return opCodeRcv; // return opcode value
}

//to get only the actual file data from the recived data packet
char* get_file_data(char* data_packet) 
{
    char* buffer = data_packet;
    char* file_ptr = buffer + DATA_OFFSET; // point at 5th offset
    char *file = malloc(MAX_DATA_SIZE);
    bzero(file, MAX_DATA_SIZE);
    char *data = file;
    memcpy (data, file_ptr,MAX_DATA_SIZE );
    return file;
 
}

// to get block number from ACK and Data packet
unsigned short get_block_number(char* packet)
{
    unsigned short *block_num_ptr = (unsigned short*) packet + 1; 
    unsigned short block_num = ntohs(*block_num_ptr);
    return block_num; // return block number
}

//to get error code from recived error packet
// help to determine what type of error is
unsigned short get_error_code(char* packet)
{
    unsigned short *err_num_ptr = (unsigned short*) packet + 1; 
    unsigned short err_code = ntohs(*err_num_ptr);
    return err_code; // return block number
}

//to get the file name from the recived RRQ or WRQ packet
// to be used by server
char* get_file_name( char* rq_packet)
{
    char *buffer = malloc(FILE_NAME_BYTES);
    bzero(buffer, FILE_NAME_BYTES);
    char *file_name_ptr = rq_packet+ OPCODE_OFFSET; //point to third offeset
    int file_size = strlen(file_name_ptr) - 6; // subtract mmode and 1 0's bytes
    memcpy (buffer, file_name_ptr, file_size); 
    return buffer;
}


void handle_timeout( int signum )
{
        printf("timeout!\n");
        printf("retransmiting again\n");
}

// This function handles the timeout by incrementing
// the corresponding global variables.
//Input:  signal number.
// Return: None.
 
int register_handler( ){
    int rt_value = 0;

    //register the handler function. 
    rt_value = ( long ) signal( SIGALRM, handle_timeout );
    if( rt_value == ( long ) SIG_ERR ){
        printf("can't register function handle_timeout.\n" );
        printf("signal() error: %s.\n", strerror(errno) );
        return -1;
    }
    
    //disable the restart of system call on signal
    //so that the OS will not stuck in the system call
    rt_value = siginterrupt( SIGALRM, 1 );
    if( rt_value == -1 ){
        printf( "invalid sig number.\n" );
        return -1;
    }
    return 0;
}