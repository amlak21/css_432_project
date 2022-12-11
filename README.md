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
   
Step 3: Open two integrated terminals from one css lab machine. One on server directory and the other on client directory.

      e.g user@csslab4 tftp/server%         and       user@csslab4 tftp/client%
