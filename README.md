 
 
 
css_432_project

Implementing and testing TFTP



Program description:
      a client/server network application program in C/C++ on a UNIX platform
     is implemented and tested using the UDP/IP family of Internet protocols.



Files:
   1. tftpserver.c
   2. tftpclient.c
   3. tftp.h
   4. tftp.c
   5. test1.txt
   6. test2.txt


Instruction to run:
   1. Open two terminals one for server and the other for client 
   2. On the server terminal:   enter compiler
            For example: ./server 
   3. On the client terminal: enter compiler, the request, Ip address and the file name.
            For example: ./client RRQ 127.0.0.1 test2.txt 
                       : ./client WQR 127.0.0.1 test1.txt


Implementation details:
  1. Read and write request: implemented
  2. Acknowledgement and data packet transferred between server and client: implemented




      =================//=======================

 
