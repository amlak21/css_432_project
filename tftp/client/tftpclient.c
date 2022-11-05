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
        
        //deallocate RRQ packet


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
                    break;
                }
                
                char* ACK_packet = create_ACK_packet(block_number); // creat ack packet
                bzero(buffer,sizeof(buffer));
                memcpy(buffer,ACK_packet, sizeof(buffer));

                 // deallocate ACK_packet

                if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
                {
                    printf("%s: ack sendto error on socket\n",prog_name);
			        exit(1);
                }
                printf("%s: sent ACK block: %d to %s\n", prog_name, block_number, serv_host_addr);
                //n++;
            }


            //else if(opcode == 5) // error packet
            //{
            // get error msg
            // get error code
            //printf("packet contains an error packet: error code: %s, error msg: %s\n", error code,error msg) );
            //what to do when recieving error packet
            //}

        }
        // after break from the loop
        printf("%s: has already recived last data block \n",prog_name);

        // send last acknoledge
        char* ACK_packet = create_ACK_packet(block_number); // creat ack packet
        bzero(buffer,sizeof(buffer));
        memcpy(buffer,ACK_packet, sizeof(buffer));

        // deallocate ACK_packet

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
        // create WRQ packet
        char* WRQ_packet = create_WRQ_packet(file_name); //create RRQ packet
        
        //send out WRQ - once
        bzero(buffer,sizeof(buffer));
        memcpy(buffer,WRQ_packet, sizeof(buffer));

        //deallocate WRQ packet

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
       // printf("%s: recived %d bytes from %s\n", prog_name, packet_bytes, serv_host_addr);

        printf("%s: recived packet\n", prog_name);
        opcode = get_opcode(buffer);
        block_number = get_block_number(buffer);

        if(opcode == 4) // ack packet
        {
            printf("    packet contains ack block: %d\n", block_number);

            // starts writing once it get ack
            //const size_t MAX_LEN = 1600; 
    	    FILE * fp;
    	    char f_array[ MAX_FILE_LEN +1];
    	    int c;
    	    size_t i = -1; //
    	    f_array[ MAX_FILE_LEN +1] = 0;
    	    fp = fopen(argv[3],"r");   // argv[3] - input file

    	    if ( NULL == fp )
		    {
        	    perror("Error opening file");
		    }

    	    else 
		    {
        	    while ( EOF != (c = fgetc( fp )) && ++i < MAX_FILE_LEN )
            	    f_array[ i ] = c;

        	    fclose (fp);
    	    }
    	    f_array[ i ] = 0;

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

                //deallocate data packet

                if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
                {
                    printf("%s: data block send to error on socket\n",prog_name);
			        exit(1);
                }
            
                printf("%s: sent data block: %d with %d bytes to %s\n", prog_name,block_counter, strlen(one_data),serv_host_addr);
                
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
                char* increment = large_file + 512;
                large_file = increment;
                //n++;
            }
            // send last data block
            char* one_data = get_one_packet_data(large_file);
            char* data_packet = create_data_packet(block_counter, one_data);

            bzero(buffer,sizeof(buffer));
            memcpy(buffer,data_packet, sizeof(buffer));

            //deallocate data packet

            if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
            {
                printf("%s: data block send to error on socket\n",prog_name);
			    exit(1);
            }
            printf("%s: sent last data block: %d with %d bytes to %s\n", prog_name,block_counter, strlen(one_data),serv_host_addr);
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

        //else if //error packet
        else // send request again after sometime ?
        {
            printf("no ack from server");
        }

        //deallocate WRQ packet
          
    }

    close(sockfd);
	exit(0);
}