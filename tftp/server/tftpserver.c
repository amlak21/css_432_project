

#include "tftp.h"

char *prog_name;
//#define MAXMESG 2048
//#define REQUEST_BUFFER_SIZE 50
#define MAX_BUFFER_SIZE 516


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

		// get open the file to send
		
		const size_t MAX_LEN = 512; 
    	FILE * fp;
    	char f_array[ MAX_LEN +1];
    	int c;
    	size_t i = -1;
    	f_array[ MAX_LEN +1] = 0;

    	//char file_name[] = "hi.txt";
    	//char*f = file_name;

    	fp = fopen(argv[1],"r");

    	if ( NULL == fp )
		{
        	perror("Error opening file");
		}

    	else 
		{
        	while ( EOF != (c = fgetc( fp )) && ++i < MAX_LEN )
            	f_array[ i ] = c;

        	fclose (fp);
    	}
    	f_array[ i ] = 0;

   		//char* file34 = f_array;
		 //printf("\nThe content: \n%s\n", f_array);

		int n = 0; //loop counter
        int block_counter = 1; //block counter
		char* serv_large_file = f_array;
		//char* serv_large_file = f_array;


//////////////////
/*
		// while loop - for number of packets
		while(strlen(serv_large_file) > 512)
		{
			char* current_bytes = serv_large_file; //
			char* serv_one_data = get_one_packet_data(current_bytes); // get first 512 bytes



			
			serv_large_file + 512; // increment by 512 bytes
			// send data
			// receive acknowledge

		}
		// send the last data
		// block_counter++ for block
		char* serv_one_data = get_one_packet_data(serv_large_file); // get first 512 bytes
		// send last data packet
		// recieve acknoledege

*/
///////////////


		while(n < MAX_NUM_PACKETS) // to send only two packets
		{
			// get one data block size data <=512
		
        	char* serv_one_data = get_one_packet_data(serv_large_file);
			//printf("\n length of one:\n%d\n", strlen(serv_one_data));

			// increment large file by 512
			//printf("\n length of large:\n%d\n", strlen(serv_large_file));

        	if(strlen(serv_large_file) > 512)
        	{
            	serv_large_file + 512; // incerement by 512
        	}
        	// create data packet

        	char* data_packet = create_data_packet(block_counter, serv_one_data);
			
			// send data block 1
			bzero(buffer, sizeof(buffer));
		
			memcpy(buffer,data_packet, sizeof(buffer));

	
			// deallocate data packet

			if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
            {
                printf("%s: data block send to error on socket\n",prog_name);
			    exit(1);
            }
          
            printf("%s: sent data block: %d with %d bytes\n", prog_name,block_counter,strlen(serv_one_data) );
    
			
			// recv ack #1
			bzero(buffer, sizeof(buffer));
			if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
            {
            	printf("%s: ack recvfrom error\n",prog_name);
				exit(4);
            }
            
			
			//printf("%s: recived %d bytes from client\n", prog_name, packet_bytes);

			 printf("%s: recived packet\n", prog_name);
            printf("    packet contains ack block: %d\n", block_counter);
			block_counter++;
            n++;


			
		}
		
	}

	else if(opcode == 2) // WRQ
	{
		printf("%s: receievd WRQ \n",prog_name);
		//create ack packet
		char* ACK_packet = create_ACK_packet(0);

		// send ack #0
		bzero(buffer, sizeof(buffer));
		memcpy(buffer,ACK_packet, sizeof(buffer));

		// deallocate ack packet 0

		if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
        {
            printf("%s: ack block send to error on socket\n",prog_name);
			exit(1);
        }
		printf("%s: sent ack block: %d to client\n", prog_name,0);

		int n = 0; //loop counter
        int block_counter = 1; //block counter
		while(n < MAX_NUM_PACKETS) // to recive only two packets
		{
			// recv data packet
			bzero(buffer, sizeof(buffer));
			if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
        	{
        		printf("%s: data block recvfrom error\n",prog_name);
				exit(4);
        	}
        	//printf("%s: recived %d bytes from client\n", prog_name, packet_bytes);

			printf("%s: recived packet\n", prog_name);
        	printf("    packet contains data block: %d\n", block_counter);

			// parse data



			// deallocate data packet after done writing
		
			//create ack packet
			char* ACK_packet_1 = create_ACK_packet(block_counter);

			// send ack #0
			bzero(buffer, sizeof(buffer));
			memcpy(buffer,ACK_packet_1, sizeof(buffer));

			// deallocate ack packet_1


			if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, &pcli_addr, cli_addr_len)) == -1)
        	{
            	printf("%s: ack block send to error on socket\n",prog_name);
				exit(1);
        	}
			printf("%s: sent ack block: %d to client\n", prog_name,block_counter);
			block_counter++;
			n++;


			

		}

		
		
	}
	else // Wrong reqest
	{
		printf("%s: Not RRQ/WRQ request\n",prog_name);
		exit(2);
	}
	
	}
}




