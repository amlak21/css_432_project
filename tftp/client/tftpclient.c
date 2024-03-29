
//fttpclient.c



#include "tftp.h"

#define serv_host_addr "127.0.0.1"    // server address
char *prog_name; // pointer to program name
#define MAX_BUFFER_SIZE 516
const size_t MAX_FILE_LEN = 1200000 ; // up to 1.2 MB



int main(int argc, char* argv[])
{
    int sockfd;
    int packet_bytes; // store sent/receive packet bytes
    char buffer[MAX_BUFFER_SIZE];  // store recived data packet

    char file_buffer[MAX_FILE_LEN + 1]; // store all received data file to be written on a file
    bzero(file_buffer, sizeof(file_buffer));
    char* file_buffer_ptr = file_buffer;


    int port_number = SERV_UDP_PORT; //default port number

    unsigned short  opcode; // store opcode
    unsigned short block_number; // store block number - data and ack packet
    unsigned short next_block_number; // store next expected block number

    int timeout_counter; // to store number of timeouts
    unsigned short error_code;	//store error code
  
 
    // setting up server and client addresses                                                    
    struct sockaddr_in  cli_addr, serv_addr;

    unsigned int serv_addr_len = sizeof(struct sockaddr_in);

    // argumnets order: prog_name, RQ, file,
     if((argc == 3) || (argc == 5)) // check number of arguments
    {
	  	if (argc == 5)
		{
	 		if(strcmp(argv[3], "-P") == 0 || strcmp(argv[3], "-p") == 0)
		 	{
			  port_number = atoi(argv[4]);
		 	}
		}
	}

	else
	{
		fprintf(stderr,"Server erorr: Invalid number of argumnets\n");
         exit(1);
	}

    prog_name = argv[0]; //store program name for later use
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

     if(register_handler()!=0)
    {
        printf("failed to register timeout\n"); 
	}
    // ready to send request to server
    // if RRQ
    if(strcmp(argv[1], "-R") == 0 || strcmp(argv[1], "-r") == 0)
    { 
        char* file_name = input_file;
        char* RRQ_packet = create_RRQ_packet(file_name); //create RRQ packet

        bzero(buffer,sizeof(buffer));
        memcpy(buffer,RRQ_packet, sizeof(buffer)); //copy RRQ_packet into buffer
        
        free(RRQ_packet); //deallocate RRQ packet

        //send out RRQ 
        if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
        {
            printf("%s: RRQ send to error on socket\n",prog_name);
			exit(1);
        }

        printf("%s: sent RRQ to %s\n", prog_name, serv_host_addr);

        next_block_number = 1; //expected block number
        
        while(1) //infinite while loop to receive data and send ack unilt last data.
        {           //loop will break when last data block is received
            bzero(buffer,sizeof(buffer));

            if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &serv_addr_len)) < 0)
            {
                printf("%s: data block recvfrom error\n",prog_name);
			    exit(4);			
            }

            opcode = get_opcode(buffer);
            block_number = get_block_number(buffer);
            
            if(opcode == 3) //received data packet
            {
                printf("%s: recived packet \n",prog_name);
                char* buffer_rcvd = buffer;
                char* data_file = get_file_data(buffer_rcvd); //get the actual data from packet
                
                //discard duplicates
                if(next_block_number > block_number)
                {
                    printf("    packet contains data block: %d with %li bytes: duplicate and discarded\n", block_number,strlen(data_file) );
                }


                else if(next_block_number == block_number)
                {
                    printf("    packet contains data block: %d with %li bytes\n", block_number,strlen(data_file) );


                    //store the recived file
                    char* data_file_ptr = data_file;
                    memcpy(file_buffer_ptr, data_file_ptr, strlen(data_file_ptr));
                    char* file_increment = file_buffer_ptr + strlen(data_file_ptr); //point after data file index
                    file_buffer_ptr = file_increment;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /*
                // needed only to test timeout
                // to suspend it after reciving data block 5
                if(block_number == 3)
                {
                    printf("%s: sleep for 5 seconds \n",prog_name);
                    sleep(5); // sleep for five seconds
                }
            */
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                    next_block_number++; //increment next expected block
                }

                else //discard unorderd block
                {
                    printf("    packet contains data block: %d with %li bytes: unordered and discarded\n", block_number,strlen(data_file) );

                }
               
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
                printf("%s: recived packet \n",prog_name);
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


        // write file to directory 
        FILE *fp; 
        fp = fopen(argv[2], "w" ); //open file in write mode
        fwrite(file_buffer , 1 , strlen(file_buffer) , fp ); //write to file

        fclose(fp); //close file
        printf("\nFile Received Successfully!\n");
        
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

            printf("\n \n"); // used for issue with malloc assertion

        	fclose (fp);
    	}
    	f_array[ i ] = 0;
   
        char* file_name = input_file;

        char* WRQ_packet = create_WRQ_packet(file_name); //create RRQ packet
       
        timeout_counter = 0;
        while(1) // for request
        {
            bzero(buffer,sizeof(buffer));
            memcpy(buffer,WRQ_packet, sizeof(buffer));

            if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
            {
                printf("%s: WRQ send to error on socket\n",prog_name);
                free(WRQ_packet);//deallocate WRQ packet

			    exit(1);
            }
            printf("%s: sent WRQ to %s . Timer set!\n", prog_name, serv_host_addr);

            //recievefrom ack #0 
            alarm(1);
            bzero(buffer,sizeof(buffer));
            if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, &serv_addr_len)) < 0)
            {
                if( errno == EINTR )
				{
			    	//printf("timeout triggered!\n");
					timeout_counter++; //increment timeout counter
					if(timeout_counter == 10) //terminate after 10 consecutive timeouts
					{
						printf("%s: 10 timeouts reached and terminated\n",prog_name);
                        free(WRQ_packet);//deallocate WRQ packet
						exit(4);	
					}
					//retransmit on timeout
					continue;
				}

				else //terminate on other recvfrom error
				{
					printf("%s: ack recvfrom error\n",prog_name);
                    free(WRQ_packet);//deallocate WRQ packet
					exit(4);
				}
            }

            else 
			{
			    break;
			}

        }

        free(WRQ_packet);//deallocate WRQ packet
       
        printf("%s: recived packet. Timer cleared!\n", prog_name);
        alarm(0);
		timeout_counter = 0; //reset timeout counter
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
               

                timeout_counter = 0;
                while(1) //retransmiti until receive ack
                {
                    bzero(buffer,sizeof(buffer));
                    memcpy(buffer,data_packet, sizeof(buffer));

                    if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
                    {
                        printf("%s: data block send to error on socket\n",prog_name);
                        free(data_packet);
                        free(one_data);

			            exit(1);
                    }
            
                    printf("%s: sent data block: %d with %li bytes to %s . Timer set!\n", prog_name,block_counter, strlen(one_data),serv_host_addr);
                   
                    // rceive ack
                    alarm(1); //set timer

                    bzero(buffer,sizeof(buffer));
                    if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, &serv_addr_len)) < 0)
                    {
                        if( errno == EINTR )
					    {
			    		    //printf("timeout triggered!\n");
						    timeout_counter++; //increment timeout counter
						    if(timeout_counter == 10) //terminate after 10 consecutive timeouts
						    {
							    printf("%s: 10 timeouts reached and terminated\n",prog_name);
                                free(data_packet);
                                free(one_data);
							    exit(4);
							
						    }
							
						    //retransmit on timeout
                            alarm(0);
						    continue;
					    }

					    else //terminate on other recvfrom error
					    {
						    printf("%s: ack recvfrom error\n",prog_name);
                            free(data_packet);
                            free(one_data);

						    exit(4);
					    }
                    }

                    else 
				    {
			            break;
			 	    }
                }
                
                free(one_data);
                free(data_packet);

                printf("%s: recived packet. Timer cleared!\n", prog_name);
                alarm(0);
			    timeout_counter = 0; //reset timeout counter
                printf("    packet contains ack block: %d\n", block_counter);

                block_counter++; //increment block number
                char* increment = large_file + 512; //increment by 512 size
                large_file = increment;
            }

            // send last data block
            char* one_data = get_one_packet_data(large_file);
            char* data_packet = create_data_packet(block_counter, one_data);

        
            timeout_counter = 0;

            while(1) // for last data block
            {
                bzero(buffer,sizeof(buffer));
                memcpy(buffer,data_packet, sizeof(buffer));
                if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
                {
                    printf("%s: data block send to error on socket\n",prog_name);
                    free(data_packet);//deallocate data packet
                    free(one_data);
			        exit(1);
                }

                printf("%s: sent last data block: %d with %li bytes to %s.Timer set!\n", prog_name,block_counter, strlen(one_data),serv_host_addr);
               
                alarm(1);
                bzero(buffer,sizeof(buffer));

                //receive last ack
                if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, &serv_addr_len)) < 0)
                {
                    if( errno == EINTR )
					{
			    		//printf("timeout triggered!\n");
						
						timeout_counter++; //increment timeout counter
						if(timeout_counter == 10) //terminate after 10 consecutive timeouts
						{
							printf("%s: 10 timeouts reached and terminated\n",prog_name);
                            free(data_packet);//deallocate data packet
                            free(one_data);
							exit(4);
							
						}
							
						//retransmit on timeout
                        alarm(0);
						continue;
					}

					else //terminate on other recvfrom error
					{
						printf("%s: ack recvfrom error\n",prog_name);
                        free(data_packet);//deallocate data packet
                        free(one_data);
						exit(4);
					}
                }
                else 
				{
			       break;
			 	}         
            }

            free(one_data);
            free(data_packet);//deallocate data packet

            printf("%s: recived packet. Timer cleared!\n", prog_name);
            alarm(0); //reset timer
			timeout_counter = 0; //reset timeout counter
            printf("    packet contains last ack block: %d\n", block_counter);  
            printf("\nFile Sent Successfully!\n"); 
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