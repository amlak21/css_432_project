#include "tftp.h"

char *prog_name; // pointer to program name
#define MAX_BUFFER_SIZE 516
const size_t MAX_FILE_LEN = 60000 ; // upt tp  12 MB file to read/write
                   

int main(int argc, char* argv[])
{
    int sockfd;
    int packet_bytes; // store sent/receive packet bytes
    char buffer[MAX_BUFFER_SIZE];  // store recived data packet
    //char file_buffer[MAX_FILE_SIZE]; // store all received data file to be written on a file
    int port_number = SERV_UDP_PORT; //default port number

    unsigned short  opcode; // store opcode
    unsigned short block_number; // store block number - data and ack packet
    unsigned short error_code;	//store error code
    char error_msg[]= ""; //store error message
 
    // setting up server and client addresses                                                    
    struct sockaddr_in  cli_addr, serv_addr, *addr_ptr;

    // argumnets order: prog_name, RQ, file, server_address
     if((argc == 4) || (argc == 6)) // check number of arguments
    {
	  	if (argc == 6)
		{
	 		if(strcmp(argv[4], "-P") == 0 || strcmp(argv[4], "-p") == 0)
		 	{
			  port_number = atoi(argv[5]);
		 	}
		}
	}

	else
	{
		fprintf(stderr,"Server erorr: Invalid number of argumnets\n");
         exit(1);
	}

    prog_name = argv[0]; //store program name for later use
    char* serv_host_addr = argv[3]; // store server IP address
    char *input_file = argv[2];  // store input file

    //Initialize first the server's data with the well-known numbers. 
    bzero((char *) &serv_addr, sizeof(serv_addr)); // clear up server address
    serv_addr.sin_family      = AF_INET; //set address family to internet address
 
    serv_addr.sin_addr.s_addr = inet_addr(serv_host_addr); 
    serv_addr.sin_port        = htons(port_number); // set port

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
 
    //Let the system choose one of its addresses for it        
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
        char* file_name = input_file;
        char* RRQ_packet = create_RRQ_packet(file_name); //create RRQ packet

        bzero(buffer,sizeof(buffer));
        memcpy(buffer,RRQ_packet, sizeof(buffer)); //copy RRQ_packet into buffer
        
        free(RRQ_packet); //deallocate RRQ packet

        //send out RRQ - once
        if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
        {
            printf("%s: RRQ send to error on socket\n",prog_name);
			exit(1);
        }

        printf("%s: sent RRQ to %s\n", prog_name, serv_host_addr);
          
        while(1) //infinite while loop to receive data and send ack unilt last data
        {           //will break when last data block is received
            bzero(buffer,sizeof(buffer));
            if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
            {
                printf("%s: data block recvfrom error\n",prog_name);
			    exit(4);
            }

            printf("%s: recived packet \n",prog_name);
            opcode = get_opcode(buffer);
            block_number = get_block_number(buffer);
            
            if(opcode == 3) //received data packet
            {
                char* buffer_rcvd = buffer;
                char* data_file = get_file_data(buffer_rcvd); //get the actual data from packet
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
            }


            else if(opcode == 5) // received error packet
            {
                error_code = get_error_code(buffer); // get error msg
               
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
            }

        }
        // after break from the loop on last data block
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
    }



    //if it is WRQ
    else if(strcmp(argv[1], "-W") == 0 || strcmp(argv[1], "-w") == 0)
    {
        // check if the file exist and copy to array
        //const size_t MAX_LEN = 1600; 
    	FILE * fp;
    	char f_array[ MAX_FILE_LEN +1];
    	int c;
    	size_t i = -1; //
    	f_array[ MAX_FILE_LEN +1] = 0;

        if ( access(argv[2], F_OK) == -1) // check if file exists 
		{
			printf("%s: Erorr: File Does Not Exist\n",prog_name);
			exit(1);
		}

		// check for file read access permission
		else if(access(argv[2], R_OK) == -1)
		{
			printf("%s: ERROR: No permission to access a file\n",prog_name);
			exit(1);	
		}

    	else 
		{
    	    fp = fopen(argv[2],"r");   // argv[3] is input file

        	while ( EOF != (c = fgetc( fp )) && ++i < MAX_FILE_LEN )
            	f_array[ i ] = c;

        	fclose (fp);
    	}
    	f_array[ i ] = 0;
   
       printf("\n \n"); // used for issue with malloc assertion

        char* file_name = input_file;

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

        if(opcode == 4) // received ack packet
        {
            printf("    packet contains ack block: %d\n", block_number);

            // starts writing once it get ack
            int block_counter = 1; //block counter
            char* large_file = f_array;

            while(strlen(large_file) > 512) //when file is larger than 512 bytes
            {
                char* current_bytes = large_file; 

                // get one data block size data (only 512 bytes)
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

                block_counter++; //increment block number
                char* increment = large_file + 512; //increment by 512 size
                large_file = increment;
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


        else if(opcode == 5) //received error packet
        {
            error_code = get_error_code(buffer); // get error code
            if(error_code == 6) // if error code is 6
            {
                printf("    packet contains error packet with error code: %d: File Already Exists\n", error_code);
                exit(1);
            }

            else
            {
                printf("    packet contains error packet with unknown error code");
                exit(1);
            }
        }
            
        else // send request again after sometime ?
        {
            printf("no ack from server");
            exit(1);
        }    
    }

    close(sockfd);
	exit(0);

    return 0;
}