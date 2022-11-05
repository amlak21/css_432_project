#include "tftp.h"

char *prog_name; // pointer to program name
//#define MAXLINE 512 // size of data to be sent
#define MAX_BUFFER_SIZE 516
                   

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
         
        
        int n = 0;
        while(n < MAX_NUM_PACKETS) // need while(true)
        {
            // recvfrom data block #1 - recieve until server done sending - need some loop
            bzero(buffer,sizeof(buffer));
            if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
            {
                printf("%s: data block recvfrom error\n",prog_name);
			    exit(4);
            }
            //printf("%s: recived %d bytes from %s\n", prog_name, packet_bytes, serv_host_addr);
            printf("%s: recived packet \n",prog_name);




        //test

            opcode = get_opcode(buffer);
            block_number = get_block_number(buffer);
            // maybe use switch
            if(opcode == 3) //data packet
            {
                //printf("\n length of buffer\n %d\n", strlen(buffer));
                char* buffer_rcvd = buffer;

                char* data_file = get_file_data(buffer_rcvd);
                
               // printf("\n length of data file\n %d\n", strlen(data_file));

                printf("    packet contains data block: %d with %d bytes\n", block_number,strlen(data_file) );

                // open the file
                // check if not empty
                // parse data block
                // copy the file - copy it at large buffer and then write that buffer to file

                // send ack and track ack block - send until server done sending and once after it is done

                /////////////////
               // if(strlen(data_file) < 512) // reciving last data file
                //{
                   // break;
               // }

                /////////////

                char* ACK_packet = create_ACK_packet(block_number); 

                bzero(buffer,sizeof(buffer));
                memcpy(buffer,ACK_packet, sizeof(buffer));

                 // deallocate ACK_packet



                if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
                {
                    printf("%s: ack sendto error on socket\n",prog_name);
			        exit(1);
                }
                printf("%s: sent ACK block: %d to %s\n", prog_name, block_number, serv_host_addr);

               
                n++;
        
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
            // last data block have been receivec
        // send last acknoledge
            // create ack packet
            // copy it 
            // deallocate ack packet
            //sent last ack





        

        // deallocate RRQ_packet when done
        
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

            int n = 0; //loop counter
            int block_counter = 1; //block counter
            char* large_file = input_file;

            while(n < MAX_NUM_PACKETS) // to send only two packets
            {
                // get one data block size data <=512
                char* one_data = get_one_packet_data(large_file);
                // increment large file by 512
                if(sizeof(large_file) > 512)
                {
                    large_file + 512; // incerement
                }
                // create data packet
                char* data_packet = create_data_packet(block_counter, one_data);
                // send data block #1

                bzero(buffer,sizeof(buffer));
                memcpy(buffer,data_packet, sizeof(buffer));

                //deallocate data packet

                if((packet_bytes = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
                {
                    printf("%s: data block send to error on socket\n",prog_name);
			        exit(1);
                }
            
                printf("%s: sent data block: %d to %s\n", prog_name,block_counter, serv_host_addr);
                
                // rceive ack
                bzero(buffer,sizeof(buffer));
                if((packet_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0)
                {
                    printf("%s: ack recvfrom error\n",prog_name);
			        exit(4);
                }
                //printf("%s: recived %d bytes from %s\n", prog_name, packet_bytes, serv_host_addr);

                printf("%s: recived packet\n", prog_name);
                printf("    packet contains ack block: %d\n", block_counter);
                block_counter++;
                n++;
            // create data block using 512 byets of input file - increment block
            // increment pointer to next 512 bytes of input file
            // compare size - if less than - "send last data block"
            // send out data block #1 - send untill the data is done - need some loop
            // recieve ack block #1
                
         

            }
            
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