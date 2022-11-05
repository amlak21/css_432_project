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

// costants for ACK packet
const static unsigned short OP_CODE_ACK = 4;

// constants for Error packet
const static unsigned short OP_CODE_ERR = 5;
/*

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
*/
// create packet from one data blcok size file
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
    
    memcpy(file_data, file, strlen(file)); // bcopy , memcpy
    return buffer;
}

// for data file
    //store the input file in some buffer
    // read the next 512 byte(untill last ) and return one data packet size file <= 512
char* get_one_packet_data(char* input_file) // tested
{
    char* file = input_file;
   char *buffer = malloc(MAX_DATA_SIZE + 1);
    if(strlen(file) > MAX_DATA_SIZE) 
    {
        bzero(buffer, MAX_DATA_SIZE + 1);
        memcpy (buffer, file, MAX_DATA_SIZE);   // copy only 512 bytes
        printf("\nthe buffer inside is: \n %s\n", buffer);
    }

    else if(strlen(file) <= MAX_DATA_SIZE)
    {
        bzero(buffer, MAX_DATA_SIZE + 1);
        memcpy(buffer, file, strlen(file)+ 1);  // copy all of bytes
    }
    return buffer; // return one packet size data

}

/*
char* create_ERR_packet(char* err_code, char* err_msg)
{
    char *buffer = malloc (5 + strlen(err_msg));
    bzero(buffer, strlen(buffer));
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
*/
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

// for data and acknoledegment packet
unsigned short get_block_number(char* packet)
{
    unsigned short *block_num_ptr = (unsigned short*) packet + 1; 
    unsigned short block_num = ntohs(*block_num_ptr);
    return block_num; // return block number
}

//get file_name from RRQ/WRQ packet

// implement
// get error code 
// get error mesg
char* open_file(char* file_name)
{
    
}







int main()
{




// get input file, parse it
    const size_t MAX_LEN = 100;
    FILE * fp;
    char f_array[ MAX_LEN +1];
    int c;
    size_t i = -1;
    f_array[ MAX_LEN +1] = 0;

    char file_name[] = "hi.txt";
    char*f = file_name;

    fp = fopen(f,"r");

    if ( NULL == fp )
        perror("Error opening file");
    else {
        while ( EOF != (c = fgetc( fp )) && ++i < MAX_LEN )
            f_array[ i ] = c;

        fclose (fp);
    }
    f_array[ i ] = 0;

   char* file34 = f_array;
   //char* in_file = file1;
   printf("\nThe content: \n%s\n", f_array);
char* bFile = get_one_packet_data(file34);
printf("\nThe file is: \n %s", bFile);
printf("\nThe size str is bfi: \n %d", strlen(bFile));
printf("\nThe size szof is bfi: \n %d", sizeof(bFile));
char* buf6 = create_data_packet(5, bFile);
//bzero(bft, sizeof(bft));
//memcpy(bft,buf6, strlen(buf6) );

printf("\nThe size str is bf6: \n %d", strlen(buf6));
printf("\nThe size szof is bf6: \n %d", sizeof(buf6));

for ( int i = 0; i < 516; i++ ) {
        printf("0x%X,", buf6[i]);
    }

char* gFile = get_file_data(buf6);

printf("\nThe size is str gf: \n %d", strlen(gFile));
printf("\nThe size szo is gf: \n %d", sizeof(gFile));
printf("\nThe file is: \n %s\n", gFile);



    
    

   




/*


char file2[]= "post.txt";
char file1[] = "Hello world! NIce 112 to see";
char err_code[] = "45";

char* in_file = file1;
char* bFile = get_one_packet_data(in_file);
printf("\nThe file is: \n %s", bFile);
printf("\nThe size str is bfi: \n %d", strlen(bFile));
printf("\nThe size szof is bfi: \n %d", sizeof(bFile));
char* buf6 = create_data_packet(5, bFile);
//bzero(bft, sizeof(bft));
//memcpy(bft,buf6, strlen(buf6) );

printf("\nThe size str is bf6: \n %d", strlen(buf6));
printf("\nThe size szof is bf6: \n %d", sizeof(buf6));

for ( int i = 0; i < 50; i++ ) {
        printf("0x%X,", buf6[i]);
    }

char* gFile = get_file_data(buf6);

printf("\nThe size is str gf: \n %d", strlen(gFile));
printf("\nThe size szo is gf: \n %d", sizeof(gFile));
printf("\nThe file is: \n %s", gFile);


    
char* ff = bft;
memcpy(ff,buf6, strlen(buf6));

printf("pointer\n");
for ( int i = 0; i < 20; i++ ) {
        printf("0x%X,", ff[i]);
    }
printf("buf6\n");

    for ( int i = 0; i < 20; i++ ) {
        printf("0x%X,", buf6[i]);
    }


//char* data = get_file_data(buf6);

//printf("%s\n", data);



//unsigned short opcode = get_opcode(buf6);
//printf("%s%d\n","opcode = ",opcode);

//unsigned short block = get_block_number(buf6);
//printf("%s%d\n", "block = ", block);


/*
char *buf = create_RRQ_packet(file2);
printf("%s\n", "RRQ packet");
for ( int i = 0; i < 20; i++ ) {
        printf("0x%X,", buf[i]);
    }

printf( "\n%s\n", "WRQ packet");
char *buf1 = create_WRQ_packet(file2);
for ( int i = 0; i < 20; i++ ) {
        printf("0x%X,", buf1[i]);
    } 


printf( "\n%s\n", "data packet");
char *buf2 = create_data_packet(1,file2);
for ( int i = 0; i < 30; i++ ) {
        printf("0x%X,", buf2[i]);
    } 

printf( "\n%s\n", "ack packet");
char *buf3 = create_ACK_packet(2);
for ( int i = 0; i < 4; i++ ) {
        printf("0x%X,", buf3[i]);
    } 


printf( "\n%s\n", "err packet");
char *buf4 = create_ERR_packet(err_code, file2);
for ( int i = 0; i < 15; i++ ) {
        printf("0x%X,", buf4[i]);
    } 
    
*/
return 0;
}

