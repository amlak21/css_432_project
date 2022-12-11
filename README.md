# Directory structure

/tftp 
      
      makefile
      
      /client
    
            tftpclient.c
         
            makefile 
            
            tftp.h
         
      /server
    
            tftpserver.c
         
            makefile
            
            tftp.h
         
      /common
    
            tftp.h
            
 # How to compile
 Step 1: Put the directories and files in the structure as shown above.
 
 Step 2: open integrated terminal from css lab machine on tfttp directtory and run make. It will excute the makefile in the tftp, and that makefile inturn will excute both the makefile in server and in client directories. Or run make on server and client directories individually. Either way an excutable file client from tftpclient.c and server from tftpserver.c will be created. 
      
      e.g. user@csslab4 tftp% make 
      
      or user@csslab4 tftp/server%make       and     user@csslab4 tftp/client% make
      
# How to run
   
Step 1: Open two integrated terminals from one css lab machine. One on server directory and the other on client directory.

      e.g user@csslab4 tftp/server%         and       user@csslab4 tftp/client%
      
Step 2: First run ./server  on server directory. Then run ./client -r file_name  for read request or ./client -w file_name for write request on client directory.

      e.g. For read request:    user@csslab4 tftp/server%./server    and then       user@csslab4 tftp/client%./client -r file_name
      
                              The server will send the file, and the file  with same file name and its content will be found at client directory.
      
            For write request:   user@csslab4 tftp/server%./server    and then      user@csslab4 tftp/client%./client -w file_name
            
                              The client will send the file, and the file  with same file name and its content will be found at server directory.
                              
                              
      To give port number at command line:  same port number for client and server.
      
            For read request:    user@csslab4 tftp/server%./server -p port_number    and then       user@csslab4 tftp/client%./client -r file_name -p port_number
            
                        e.g:  user@csslab4 tftp/server%./server -p 5000555     and then   user@csslab4 tftp/client%./client -r file_name -p 5000555
            
      
            For write request:   user@csslab4 tftp/server%./server    and then      user@csslab4 tftp/client%./client -w file_name -p port_number
            
                        
                        e.g:   user@csslab4 tftp/server%./server -p 5000555    and then      user@csslab4 tftp/client%./client -w file_name -p 5000555
                         
      
      
      
      
      
      
      
      
      
      
