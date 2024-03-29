
//tftpserver.c

#include "tftp.h"

char *prog_name;
#define MAX_BUFFER_SIZE 516
const  size_t MAX_FILE_LEN = 1200000; // up to 1.2MB



int main(int argc, char* argv[])
{
	int  sockfd, old_sockfd;
	char rq_buffer[MAX_BUFFER_SIZE]; // store recived packet
	int port_number = SERV_UDP_PORT; //default port number

	int request_bytes; // store RRQ/WRQ packet bytes
	unsigned short opcode; // store opcode
	

	struct sockaddr_in serv_addr;

	struct sockaddr pcli_addr; //ptr to client address 
	unsigned int cli_addr_len = sizeof(struct sockaddr);

	// program name and optional -p port_number
	 if((argc == 1)|| (argc == 3)) // check number of arguments
    {
		if (argc == 3) // if -p port_number is given
		{
	 		if(strcmp(argv[1], "-P") == 0 || strcmp(argv[1], "-p") == 0)
		 	{
				port_number =  atoi(argv[2]); //port number if entered from the command line
		 	}
		}
	}
	else
	{
		fprintf(stderr,"Server erorr: Invalid number of argumnets\n");
		exit(1);
	}

	prog_name = argv[0]; //store program name for later use
	

	//create server socket
	if ( (old_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("%s: can't open datagram socket\n",prog_name);
		exit(1); 
	}
	// clear up server address
	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family      = AF_INET; // set address familt to interenet address

	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	serv_addr.sin_port        = htons(port_number); // set port

	// bind server address to the created socket
	if (bind(old_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{ 
		printf("%s: can't bind local address\n",prog_name);
		exit(2);
	}

	printf("%s: waiting to receive request\n",prog_name);
	
	for(; ; )
	{

		
	//recive request packet from client
	// and save client address for future sendto
	bzero(rq_buffer, sizeof(rq_buffer));
	if((request_bytes = recvfrom(old_sockfd, rq_buffer, sizeof(rq_buffer), 0, &pcli_addr, &cli_addr_len)) < 0)
	{
		printf("%s: WRQ/RRQ recvfrom error\n",prog_name);
		exit(3);
	}

	opcode = get_opcode(rq_buffer); //get opcode to determine the request

	pid_t pid = fork();
    
    if (pid == 0) 
	{ // we are in the child process

		char buffer[MAX_BUFFER_SIZE]; // store recived packet
		unsigned short block_number; // store block number - data and ack packet
		unsigned short next_block_nuber; // store next expected block number
		unsigned short error_code;	//store error code
		char error_msg[]= ""; // store error message

		int packet_bytes; // store sent/receive ack, data packet bytes
		int timeout_counter;

		char file_buffer[MAX_FILE_LEN + 1]; // store all received data file to be written on a file
    	bzero(file_buffer, sizeof(file_buffer));
    	char* file_buffer_ptr = file_buffer;

      
		if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			printf("%s: can't open datagram socket\n",prog_name);
			exit(1); 
		}


		if(opcode == 1) //RRQ
		{
		printf("%s: receievd RRQ \n",prog_name);

		char* packet_ptr = rq_buffer;
    	char* file_name = get_file_name(packet_ptr); //get file name from received request packet
		
		 
    	FILE * fp;
    	char f_array[ MAX_FILE_LEN +1]; //array to store the file to be read
    	int c;
    	size_t i = -1;
    	f_array[ MAX_FILE_LEN +1] = 0;
		
		// check if file exist in server
    	if ( access(file_name, F_OK) == -1) // check if file exists      
		{
			printf("%s: Erorr: File Does Not Exist\n",prog_name);
			error_code = 1 ; // set error code to 1
			strcat(error_msg, "File Does Not Exist"); // set error message
		}

		// check for file read access permission
		else if(access(file_name, R_OK) == -1)
		{
			printf("%s: ERROR: No permission to access a file\n",prog_name);
			error_code = 2; // set error code to 2
			strcat(error_msg, "No Read Permission"); // set error message	
		}

		// send error packet if doesnt exist or no permission to read
		if((access(file_name, F_OK) == -1 ) || (access(file_name, R_OK) == -1))
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
			fp = fopen(file_name,"r");

        	while ( EOF != (c = fgetc( fp )) && ++i < MAX_FILE_LEN )
            	f_array[ i ] = c;

        	fclose (fp);
    	}
    	f_array[ i ] = 0;

		//deallocate file name
		free(file_name);

        int block_counter = 1; //block counter
		char* serv_large_file = f_array;

		if(register_handler()!=0)
        {
           printf("failed to register timeout\n"); 
		}
		
		// while loop - for number of packets
		while(strlen(serv_large_file) > 512)
		{
			char* current_bytes = serv_large_file; //
			char* serv_one_data = get_one_packet_data(current_bytes); // get first 512 bytes
        	char* data_packet = create_data_packet(block_counter, serv_one_data); // create data packet
			
			timeout_counter = 0;

			while(1) //retransmit untill reciveing ack
			{
				bzero(buffer, sizeof(buffer));
				memcpy(buffer,data_packet, sizeof(buffer));

				//send data blocks
				if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
            	{
                	printf("%s: data block send to error on socket\n",prog_name);
					free(data_packet); // deallocate data packet
					free(serv_one_data);  //deallocate one the data block
			    	exit(1);
            	}
          
            	printf("%s: sent data block: %d with %li bytes. Timer set.\n", prog_name,block_counter,strlen(serv_one_data) );
			 
				//start timer
				alarm(1);
            	

				bzero(buffer, sizeof(buffer)); //clear buffer
				if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
            	{
            	
					if( errno == EINTR )
					{
			    		
						
						timeout_counter++; //increment timeout counter
						if(timeout_counter == 10) //terminate after 10 consecutive timeouts
						{
							printf("%s: 10 timeouts reached and terminated\n",prog_name);
							free(data_packet); // deallocate data packet
							free(serv_one_data);  //deallocate one the data block
							exit(4);
							
						}
							
						//retransmit same block data on timeout
						alarm(0);
						continue;
					}

					else //terminate on other recvfrom error
					{
						printf("%s: ack recvfrom error\n",prog_name);
						free(data_packet); // deallocate data packet
						free(serv_one_data);  //deallocate one the data block
						exit(4);
					}
			 	} 

				else 
				{
			       break;
			 	}
            }

			free(serv_one_data);  //deallocate one the data block
			free(data_packet); // deallocate data packet

			printf("%s: recived packet. Timer cleared!\n", prog_name);
			alarm(0); // clear timer
			timeout_counter = 0; //reset timeout counter
            printf("    packet contains ack block: %d\n", block_counter);

			block_counter++; //increment block number
			char* increment = serv_large_file + 512; // increment by 512 bytes
			serv_large_file = increment;

		  }
		

		if(opcode != 5) //if error packt was sent do nothing
		{
			char* serv_one_data = get_one_packet_data(serv_large_file); // get first 512 bytes
			char* data_packet = create_data_packet(block_counter, serv_one_data); //create data packet
			

			timeout_counter = 0;
			while(1) //retransmit until receiving ack
			{
				bzero(buffer, sizeof(buffer));
				memcpy(buffer,data_packet, sizeof(buffer)); //copy data packet to buffer

				if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
        		{
            		printf("%s: data block send to error on socket\n",prog_name);

					free(serv_one_data);
					free(data_packet); // deallocate data packet

					exit(1);
        		}
	
        		printf("%s: sent last data block: %d with %li bytes. Timer set!\n", prog_name,block_counter,strlen(serv_one_data) );
				

				//start timer
				alarm(1);

				// recieve acknoledege
				bzero(buffer, sizeof(buffer));
				if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
        		{
					if( errno == EINTR )
					{
			    		
						
						timeout_counter++; //increment timeout counter
						if(timeout_counter == 10) //terminate after 10 consecutive timeouts
						{
							printf("%s: 10 timeouts reached and terminated\n",prog_name);
							free(serv_one_data);
							free(data_packet); // deallocate data packet

							exit(4);
						}
							
						//retransmit on timeout
						alarm(0);
						continue;
					}

					else //terminate on other recvfrom error
					{
						printf("%s: ack recvfrom error\n",prog_name);

						free(serv_one_data);
						free(data_packet); // deallocate data packet

						exit(4);
					}		
        		}

				else //when no timeout
				{
					break;
				}
				
			}

			free(serv_one_data);
			free(data_packet); // deallocate data packet

			printf("%s: recived packet. Timer cleared\n", prog_name);
			alarm(0);
			timeout_counter = 0; //reset timeout counter
        	printf("    packet contains last ack block: %d\n", block_counter);
			printf("\nFile Sent Successfully!\n");
			printf("RRQ done!\n");
			
		}
	}



	else if(opcode == 2) // if it is WRQ
	{
		printf("%s: receievd WRQ\n",prog_name);

		char* packet_ptr = rq_buffer;
    	char* file_name = get_file_name(packet_ptr); //get file name from received request packet

		
		if ( access(file_name, F_OK) == 0) //if file already exists
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
			opcode = 5; //set opcode back to error opcode
		} 

		
		if(opcode != 5) //if error packet was not sent
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

			next_block_nuber = 1; //expected block number
			while(1) 
			{
				// recv data packet
				bzero(buffer, sizeof(buffer));
				if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
        		{
        			printf("%s: data block recvfrom error\n",prog_name);
					exit(4);
        		}

		
				opcode = get_opcode(buffer);
				block_number = get_block_number(buffer);

				if(opcode == 3)
				{
					printf("%s: recived packet\n", prog_name);
					char* buffer_rcvd = buffer;
					char* data_file = get_file_data(buffer_rcvd);

					 //discard duplicates
                	if(next_block_nuber > block_number)
                	{
                    	printf("    packet contains data block: %d with %li bytes: duplicate and discarded\n", block_number,strlen(data_file) );
                	}


                	else if(next_block_nuber == block_number) //if block number is the expected block number
                	{
                    	printf("    packet contains data block: %d with %li bytes\n", block_number,strlen(data_file) );
                    	

						//collect the recived file to file_buffer
						char* data_file_ptr = data_file;
                    	memcpy(file_buffer_ptr, data_file_ptr, strlen(data_file_ptr));
                    	char* file_increment = file_buffer_ptr + strlen(data_file_ptr); // increment to point at after data file index
                    	file_buffer_ptr = file_increment;
//////////////////////////////////////////////////////////////////////////////////////////
					/*
						//needed only to test timeout
						// suspend server after it recievd data block 5

						if(block_number == 3)
                		{
							
                    		printf("%s: sleep for 5 seconds \n", prog_name);
							sleep(5); // sleep for five seconds
						
                		}
					*/
////////////////////////////////////////////////////////////////////////////////////////


                    	next_block_nuber++; //increment next expected block
                	}

                	else //discard unorderd block
                	{
                    	printf("    packet contains data block: %d with %li bytes: unordered and discarded\n", block_number,strlen(data_file) );

                	}

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

			// when break from loop
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
			printf("\nFile Received Successfully!\n");
		}


		// write file to directory 
        FILE *fp; 
        fp = fopen(file_name, "w" ); //open file in write mode
									//file_name recived from the request packet
        fwrite(file_buffer , 1 , strlen(file_buffer) , fp ); // write file

        fclose(fp); // close file

		//deallocate file_name
		free(file_name);
		
		printf("WRQ done!!\n");
	}

	else if(opcode == 3 || opcode == 4)
	{
		
	}

	else // unkown request if it is not RRQ or WRQ
	{
		//printf("%s: Not RRQ or WRQ request\n",prog_name);
		printf("%s: waiting to receive request\n",prog_name);
		
	}
        exit (0); // terminates the child process
}

else
{
	waitpid(pid, NULL, 0);
}
	
}
	
	return 0;
}
