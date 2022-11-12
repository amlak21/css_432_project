

#include "tftp.h"

char *prog_name;
//#define MAXMESG 2048
//#define REQUEST_BUFFER_SIZE 50
#define MAX_BUFFER_SIZE 516
const size_t MAX_FILE_LEN = 60000 ; // need to be up to 12 MB file to read/write ?????????????????


int main(int argc, char* argv[])
{
	int  sockfd;
	char buffer[MAX_BUFFER_SIZE]; // store recived packet
	//char file_buffer[MAX_FILE_SIZE]; // store all received data file to be written on a file

	
	int request_bytes; // store RRQ/WRQ packet bytes
	int packet_bytes; // store sent/receive ack, data packet bytes
	//char recv_buffer[MAX_BUFFER_SIZE]; // store recieved ack, data packet

	unsigned short opcode; // store opcode
    unsigned short block_number; // store block number - data and ack packet
	unsigned short error_code;	//store error code
	char error_msg[]= ""; // store error message


	struct sockaddr_in serv_addr;

	struct sockaddr pcli_addr; //ptr to client address 
	int cli_addr_len = sizeof(struct sockaddr);

	// program name and server side input file
	 if(argc != 2) // check number of arguments
    {
        fprintf(stderr,"Server erorr: Invalid number of argumnets\n");
        exit(1);
    }
	prog_name = argv[0];
	//char* serv_input_file = argv[1]; // only used when RRQ
	//create server socket
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("%s: can't open datagram socket\n",prog_name);
		exit(1); 
	}

	// clear up server address
	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family      = AF_INET; // set address familt to interenet address

	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	serv_addr.sin_port        = htons(SERV_UDP_PORT); // set port

	// bind server address to the created socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{ 
		printf("%s: can't bind local address\n",prog_name);
		exit(2);
	}

	printf("%s: waiting to receive request\n",prog_name);
	
	for(; ; )
	{

	//recive request packet from client
	// save client address for future sendto
	bzero(buffer, sizeof(buffer));
	if(request_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, &cli_addr_len) < 0)
	{
		printf("%s: WRQ/RRQ recvfrom error\n",prog_name);
		exit(3);
	}

	printf("%s: receievd a request \n",prog_name);
	//use opcode to determine RRQ ot WRQ
	
	//char* request_ptr = buffer;
	opcode = get_opcode(buffer);

	
	if(opcode == 1) //RRQ
	{
		printf("%s: receievd RRQ \n",prog_name);
		
		// check if file exist,or permission to access to access
		//const size_t MAX_LEN = 700; 
    	FILE * fp;
    	char f_array[ MAX_FILE_LEN +1];
    	int c;
    	size_t i = -1;
    	f_array[ MAX_FILE_LEN +1] = 0;

    	//char file_name[] = "hi.txt";
    	//char*f = file_name;

    	fp = fopen(argv[1],"r");

    	if ( access(argv[1], F_OK) == -1) // check if file exists      //NULL == fp
		{
        	
			printf("%s: Erorr: File Does Not Exist\n",prog_name);
			error_code = 1 ; // set error code to 1
			strcat(error_msg, "File Does Not Exist"); // set error message

		}

		// check for file read access permission
		else if(access(argv[1], R_OK) == -1)
		{
			printf("%s: ERROR: No permission to access a file\n",prog_name);
			error_code = 2; // set error code to 2
			strcat(error_msg, "No Read Permission"); // set error message
			
		}



		// send error packet if doesnt exist or no permission to read
		if((access(argv[1], F_OK) == -1 ) || (access(argv[1], R_OK) == -1))
		{
			// create error packet
			char* error_packet = create_ERR_packet(error_code, error_msg);

			bzero(buffer, sizeof(buffer));
			memcpy(buffer,error_packet, sizeof(buffer));

			free(error_packet); // deallocate error_packet

			// send error packet
			if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
            {
                printf("%s: error packet send to error on socket\n",prog_name);
			    exit(1);
            }
          
            printf("%s: sent error packet with error code: %d\n", prog_name, error_code);
			opcode = 5; 

		}
			

    	else 
		{
        	while ( EOF != (c = fgetc( fp )) && ++i < MAX_FILE_LEN )
            	f_array[ i ] = c;

        	fclose (fp);
    	}
    	f_array[ i ] = 0;

        int block_counter = 1; //block counter
		char* serv_large_file = f_array;
		
		// while loop - for number of packets
		while(strlen(serv_large_file) > 512)
		{
			char* current_bytes = serv_large_file; //
			char* serv_one_data = get_one_packet_data(current_bytes); // get first 512 bytes
        	char* data_packet = create_data_packet(block_counter, serv_one_data); // create data packet
			
			// send data block 1
			bzero(buffer, sizeof(buffer));
			memcpy(buffer,data_packet, sizeof(buffer));

			free(data_packet); // deallocate data packet

			if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
            {
                printf("%s: data block send to error on socket\n",prog_name);
			    exit(1);
            }
          
            printf("%s: sent data block: %d with %d bytes\n", prog_name,block_counter,strlen(serv_one_data) );
			free(serv_one_data); 

			// recv ack #1
			bzero(buffer, sizeof(buffer));
			if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
            {
            	printf("%s: ack recvfrom error\n",prog_name);
				exit(4);
            }

			printf("%s: recived packet\n", prog_name);
            printf("    packet contains ack block: %d\n", block_counter);
			block_counter++; //increment block number
           // n++;
			char* increment = serv_large_file + 512; // increment by 512 bytes
			serv_large_file = increment;

		}

		if(opcode != 5) //if error packt was sent do nothing
		{
			char* serv_one_data = get_one_packet_data(serv_large_file); // get first 512 bytes
			char* data_packet = create_data_packet(block_counter, serv_one_data); //create data packet
			bzero(buffer, sizeof(buffer));
			memcpy(buffer,data_packet, sizeof(buffer)); //copy data packet to buffer
			free(data_packet); // deallocate data packet

			if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
        	{
            	printf("%s: data block send to error on socket\n",prog_name);
				exit(1);
        	}
	
        	printf("%s: sent last data block: %d with %d bytes\n", prog_name,block_counter,strlen(serv_one_data) );
			free(serv_one_data);

			// recieve acknoledege
			bzero(buffer, sizeof(buffer));
			if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
        	{
            	printf("%s: ack recvfrom error\n",prog_name);
				exit(4);
        	}
			printf("%s: recived packet\n", prog_name);
        	printf("    packet contains last ack block: %d\n", block_counter);


		}


	}

	else if(opcode == 2) // WRQ
	{
		printf("%s: receievd WRQ\n",prog_name);
		
		if ( access(argv[1], F_OK) == 0) //if file already exists
		{
			printf("%s: ERROR: File Already Exists\n",prog_name);
			error_code = 6; // set error code to 6
			strcat(error_msg, "File Already Exists"); // set error message

			// create error packet
			char* error_packet = create_ERR_packet(error_code, error_msg);

			bzero(buffer, sizeof(buffer));
			memcpy(buffer,error_packet, sizeof(buffer));

			free(error_packet); // deallocate error_packet

			// send error packet
			if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
            {
                printf("%s: error packet send to error on socket\n",prog_name);
			    exit(1);
            }
          
            printf("%s: sent error packet with error code: %d\n", prog_name, error_code);
			opcode = 5;
		} 

			
		if(opcode != 5)
		{
			//create ack packet
			char* ACK_packet = create_ACK_packet(0);

			// send ack #0
			bzero(buffer, sizeof(buffer));
			memcpy(buffer,ACK_packet, sizeof(buffer));
			free(ACK_packet); // deallocate ack packet 0

			if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
        	{
            	printf("%s: ack block send to error on socket\n",prog_name);
				exit(1);
        	}
			printf("%s: sent ack block: %d \n", prog_name,0);

		
			while(1) 
			{
				// recv data packet
				bzero(buffer, sizeof(buffer));
				if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
        		{
        			printf("%s: data block recvfrom error\n",prog_name);
					exit(4);
        		}

				printf("%s: recived packet\n", prog_name);

				opcode = get_opcode(buffer);
				block_number = get_block_number(buffer);

				if(opcode == 3)
				{
					char* buffer_rcvd = buffer;
					char* data_file = get_file_data(buffer_rcvd);
					printf("    packet contains data block: %d with %d bytes\n", block_number, strlen(data_file));

					// open the file
                	// check if not empty
                	// parse data block
                	// copy the file - copy it at large buffer and then write that buffer to file

					// deallocate data packet after done writing

					if(strlen(data_file) < 512)
					{
						free(data_file);
						break;
					}
					free(data_file);

					//create ack packet
					char* ACK_packet_1 = create_ACK_packet(block_number);
					bzero(buffer, sizeof(buffer));
					memcpy(buffer,ACK_packet_1, sizeof(buffer));
					free(ACK_packet_1); // deallocate ack packet_1

					if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
        			{
            			printf("%s: ack block send to error on socket\n",prog_name);
						exit(1);
        			}
					printf("%s: sent ack block: %d \n", prog_name,block_number);
				
				}

			}
			// break from loop
			printf("%s: has already recived last data block \n",prog_name);
			char* ACK_packet_1 = create_ACK_packet(block_number);
			bzero(buffer, sizeof(buffer));
			memcpy(buffer,ACK_packet_1, sizeof(buffer));
			free(ACK_packet_1); // deallocate ack packet_1

			if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
        	{
            	printf("%s: ack block send to error on socket\n",prog_name);
				exit(1);
        	}
			printf("%s: sent last ack block: %d \n", prog_name,block_number);
		}

		
		
	}
	else // Wrong reqest
	{
		printf("%s: Not RRQ/WRQ request\n",prog_name);
		exit(2);
	}
	
	}
}
