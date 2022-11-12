#include "tftp.h"

char *prog_name; // pointer to program name
//#define MAXLINE 512 // size of data to be sent
#define MAX_BUFFER_SIZE 516
const size_t MAX_FILE_LEN = 60000 ; // upt tp  12 MB file to read/write
                   

int main(int argc, char* argv[])
{
    int sockfd;
    int packet_bytes; // store sent/receive packet bytes
    char buffer[MAX_BUFFER_SIZE];  // store recived data packet
    //char file_buffer[MAX_FILE_SIZE]; // store all received data file to be written on a file


    unsigned short  opcode; // store opcode
    unsigned short block_number; // store block number - data and ack packet
    unsigned short error_code;	//store error code
    char error_msg[]= ""; //store error message
 
    // setting up server and client addresses                                                    
    struct sockaddr_in  cli_addr, serv_addr, *addr_ptr;

    // prog_name, RQ, srv_addr, file
    if(argc != 4) // check number of arguments
    {
        fprintf(stderr,"Client rorr: Invalid number of argumnets\n");
        exit(1);
    }

    // command line input order - program name, request, server IP, data file
    prog_name = argv[0];
    char* serv_host_addr = argv[2];

    // store the input file in large buffer
    // parse the next 512 bytes and create data packet untill end of file
    char *input_file = argv[3];  // need to be changed - uses input file directly

    //Initialize first the server's data with the well-known numbers. 

    bzero((char *) &serv_addr, sizeof(serv_addr)); // clear up server address
    serv_addr.sin_family      = AF_INET; //set address family to internet address
 
    serv_addr.sin_addr.s_addr = inet_addr(serv_host_addr); 
    serv_addr.sin_port        = htons(SERV_UDP_PORT); // set port

    //Create the socket for the client side.                          
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        printf("%s: can't open datagram socket\n",prog_name);
        exit(1);
    }

    //Initialize the structure holding the local address data to      
    //bind to the socket.                                             

    bzero((char *) &cli_addr, sizeof(cli_addr)); // clear up client address
    cli_addr.sin_family      = AF_INET;
 
    //Let the system choose one of its addresses for you          
       
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
 
    cli_addr.sin_port        = htons(0);
         
    // bind to assign address to the created socket
    if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0)
    {
        printf("%s: can't bind local address\n",prog_name);
        exit(2);
    }

    // ready to send request to server
    // if RRQ
    if(strcmp(argv[1], "-R") == 0 || strcmp(argv[1], "-r") == 0){ 
        // create_RRQ_packet
        char* file_name = input_file;
        
        char* RRQ_packet = create_RRQ_packet(file_name); //create RRQ packet

        bzero(buffer,sizeof(buffer));
        memcpy(buffer,RRQ_packet, sizeof(buffer));
        
        free(RRQ_packet); //deallocate RRQ packet


        //send out RRQ - once
        if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
        {
            printf("%s: RRQ send to error on socket\n",prog_name);
			exit(1);
        }

        printf("%s: sent RRQ to %s\n", prog_name, serv_host_addr);
         
        
        //int n = 0;
        while(1) //infinite while loop
        {
            bzero(buffer,sizeof(buffer));
            if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
            {
                printf("%s: data block recvfrom error\n",prog_name);
			    exit(4);
            }

            printf("%s: recived packet \n",prog_name);
            opcode = get_opcode(buffer);
            block_number = get_block_number(buffer);
            
            if(opcode == 3) //data packet
            {
                char* buffer_rcvd = buffer;
                char* data_file = get_file_data(buffer_rcvd);
                printf("    packet contains data block: %d with %d bytes\n", block_number,strlen(data_file) );

                // open the file
                // check if not empty
                // parse data block
                // copy the file - copy it at large buffer and then write that buffer to file

                //deallocate after done writing

                if(strlen(data_file) < 512) // reciving last data file
                {
                    free(data_file);
                    break;
                }
                free(data_file);

                char* ACK_packet = create_ACK_packet(block_number); // creat ack packet
                bzero(buffer,sizeof(buffer));
                memcpy(buffer,ACK_packet, sizeof(buffer));

                free(ACK_packet); // deallocate ACK_packet

                if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
                {
                    printf("%s: ack sendto error on socket\n",prog_name);
			        exit(1);
                }
                printf("%s: sent ACK block: %d to %s\n", prog_name, block_number, serv_host_addr);
                //n++;
            }


            else if(opcode == 5) // error packet
            {
                error_code = get_error_code(buffer); // get error msg
                //error_msg = get_error_msg(buffer);// get error code - may not be necessary
                if(error_code == 1)
                {
                    printf("    packet contains error packet with error code: %d: File Does Not Exist\n", error_code);
                    exit(1);
                }
                else if(error_code == 2)
                {
                    printf("    packet contains error packet with error code: %d: No Read Permission\n", error_code);
                    exit(1);
                }
                printf("    packet contains error packet with error code: %d\n", error_code);
                exit(1);
                //what to do when recieving error packet
            }

        }
        // after break from the loop
        printf("%s: has already recived last data block \n",prog_name);

        // send last acknoledge
        char* ACK_packet = create_ACK_packet(block_number); // creat ack packet
        bzero(buffer,sizeof(buffer));
        memcpy(buffer,ACK_packet, sizeof(buffer));

        free(ACK_packet); // deallocate ACK_packet

        if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
        {
            printf("%s: ack sendto error on socket\n",prog_name);
			exit(1);
        }
        printf("%s: sent last ACK block: %d to %s\n", prog_name, block_number, serv_host_addr);

        // deallocate ack packet
            
        
    }

    //if WRQ
    else if(strcmp(argv[1], "-W") == 0 || strcmp(argv[1], "-w") == 0)
    {
        char* file_name = input_file;

        // check if the file exist and copy to array
        //const size_t MAX_LEN = 1600; 
    	FILE * fp;
    	char f_array[ MAX_FILE_LEN +1];
    	int c;
    	size_t i = -1; //
    	f_array[ MAX_FILE_LEN +1] = 0;
    	fp = fopen(argv[3],"r");   // argv[3] - input file

    	if ( NULL == fp ) // check if file exists
		{
            printf("%s: ERROR: File does not exist\n",prog_name);
			exit(1);
		}

    	else 
		{
        	while ( EOF != (c = fgetc( fp )) && ++i < MAX_FILE_LEN )
            	f_array[ i ] = c;

        	fclose (fp);
    	}
    	f_array[ i ] = 0;


        
        // create WRQ packet
       printf("\n \n"); // used for issue with malloc assertion

        char* WRQ_packet = create_WRQ_packet(file_name); //create RRQ packet
        bzero(buffer,sizeof(buffer));
        memcpy(buffer,WRQ_packet, sizeof(buffer));

        free(WRQ_packet);//deallocate WRQ packet

        if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
        {
            printf("%s: WRQ send to error on socket\n",prog_name);
			exit(1);
        }
        printf("%s: sent WRQ to %s\n", prog_name, serv_host_addr);

        //recievefrom ack #0 
        bzero(buffer,sizeof(buffer));
        if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
        {
            printf("%s: ack recvfrom error\n",prog_name);
			exit(4);
        }

        printf("%s: recived packet\n", prog_name);
        opcode = get_opcode(buffer);
        block_number = get_block_number(buffer);

        if(opcode == 4) // ack packet
        {
            printf("    packet contains ack block: %d\n", block_number);

            // starts writing once it get ack
           

            int block_counter = 1; //block counter
            char* large_file = f_array;

            while(strlen(large_file) > 512) // 
            {
                char* current_bytes = large_file; 

                // get one data block size data <=512
                char* one_data = get_one_packet_data(current_bytes);
                char* data_packet = create_data_packet(block_counter, one_data);

                bzero(buffer,sizeof(buffer));
                memcpy(buffer,data_packet, sizeof(buffer));

                free(data_packet);//deallocate data packet

                if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
                {
                    printf("%s: data block send to error on socket\n",prog_name);
			        exit(1);
                }
            
                printf("%s: sent data block: %d with %d bytes to %s\n", prog_name,block_counter, strlen(one_data),serv_host_addr);
                free(one_data);

                // rceive ack
                bzero(buffer,sizeof(buffer));
                if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
                {
                    printf("%s: ack recvfrom error\n",prog_name);
			        exit(4);
                }
                printf("%s: recived packet\n", prog_name);
                printf("    packet contains ack block: %d\n", block_counter);
                block_counter++;
                char* increment = large_file + 512; //increment by 512 size
                large_file = increment;
                //n++;
            }
            // send last data block
            char* one_data = get_one_packet_data(large_file);
            char* data_packet = create_data_packet(block_counter, one_data);

            bzero(buffer,sizeof(buffer));
            memcpy(buffer,data_packet, sizeof(buffer));

            free(data_packet);//deallocate data packet

            if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
            {
                printf("%s: data block send to error on socket\n",prog_name);
			    exit(1);
            }
            printf("%s: sent last data block: %d with %d bytes to %s\n", prog_name,block_counter, strlen(one_data),serv_host_addr);
            free(one_data);

            bzero(buffer,sizeof(buffer));

            //receive last ack
            if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
            {
                printf("%s: ack recvfrom error\n",prog_name);
			    exit(4);
            }
            printf("%s: recived packet\n", prog_name);
            printf("    packet contains last ack block: %d\n", block_counter);

            
        }

        else if(opcode == 5) //error packet
        {
            error_code = get_error_code(buffer); // get error code
            if(error_code == 6) // if error code is 6
            {
                printf("    packet contains error packet with error code: %d: File Already Exists\n", error_code);
               // printf("Do you want to overwrite? Y/N: ");
                //get user response
                //if(strcmp == y) ....etc
                    //send response
                exit(1);
            }
                    //ask for overwrite
        }
            

        else // send request again after sometime ?
        {
            printf("no ack from server");
        }

       
          
    }

    close(sockfd);
	exit(0);
}