//************************************************************************
//
//  CSCI 463 Assignment 7
//
//  Author: Abel Vega Arteaga
//
//  Assignment: IPC (server)
//
//  I certify that this is my own work and where appropriate an extension 
//  of the starter code provided for the assignment.
//
//***********************************************************************

/*
 Copyright (c) 1986, 1993
 The Regents of the University of California.  All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. All advertising materials mentioning features or use of this software
    must display the following acknowledgement:
 This product includes software developed by the University of
 California, Berkeley and its contributors.
 4. Neither the name of the University nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

    Modifications to make this build & run on Linux by John Winans, 2021
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <signal.h>
#include <sstream> 
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * @brief this function helps to safely write to the file discriptor all of the info reliablly.
 * 
 * @param fd the file descriptor. in this case, the socket ip address
 * @param buf the contents to write the data from 
 * @param len the length of how much to send over the data
 * @return ssize_t 0 to idicate that it was successful write.
 */
static ssize_t safe_write(int fd, const char *buf, size_t len){

   //keeps looping till all the buffer is empty.
   while(len != 0) {
      ssize_t wlen = write(fd, buf, len);
      if (wlen == -1)
         return -1;  //write returned an unrecoverable error, errno will be set
      
      len -= wlen; // reduce the remaining number of bytes to send
      buf += wlen; // if we get here then we sent the full requested length!
   }

   return len;
}


/**
 * @brief usage instructions to use the program
 * 
 */
static void usage()
{
	std::cerr << "Usage: client -l listener-port" << std::endl;
	std::cerr << "    –l listener-port (default: 0)" << std::endl;

	exit(1);
}



/**
 * @brief main driver
 * 
 * @param argc how many arguments are in total
 * @param argv where the arguments are located
 * @return int 0 for secsess
 */
int main(int argc, char *argv[])
{
   int sock;    //< sock number
   int IP = 0;  //< defualt 
   int rval;    //< rval number
   int opt;     //< use for get opt
   int msgsock; //< msgsock used to write back
   socklen_t length; 
   struct sockaddr_in server;
   char buf[2048]; //< buffer of data

   // uint32_t byte_Count = 0; //< totals the number of bytes
   // uint16_t total_sum = 0;  //< total sum of the hex of the bytes
   // uint8_t  byte = 0;       //< holds a single byte

	while ((opt = getopt(argc, argv, "l:")) != -1){
		switch(opt){
		case 'l':{

         IP = atoi(optarg);

			break;
         }			
		default:{
			   usage();   // prints error if the command does not appear
         }
		}
	}

   //if( IP <= 1024)
      

   //ignore writing if the client cuts off;
   signal(SIGPIPE, SIG_IGN);


   /* Create socket */
   sock = socket(AF_INET, SOCK_STREAM, 0); // sockend end of the server. used to listen when thing get along
   if (sock < 0) {
	   perror("opening stream socket");
	   exit(1);
   }

   /* Name socket using wildcards. socket end of the server*/
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;

   //server.sin_port = 0;                               // we dont care 
   //std::cout << "Socket has port #"<< IP << std::endl;
   server.sin_port = htons(IP);                            // we want to set a specific port number
   if(bind(sock, (sockaddr*)&server, sizeof(server))){
	   perror("binding stream socket");
	   exit(1);
   }

   /* Find out assigned port number and print it out */
   length = sizeof(server);
   if(getsockname(sock, (sockaddr*)&server, &length)) {  // gets the sock name &length can change the length depending the (sockaddr*)&server
	   perror("getting socket name");
	   exit(1);
   }

   /*prints the port number */
   std::cout << "Socket has port #" << ntohs(server.sin_port) << std::endl;

    
   int writeFD = open("test.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    
   /* Start accepting connections */
   listen(sock, 5);
   do {
	   struct sockaddr_in from;      // used to display the address of the connection peer
      socklen_t from_len = sizeof(from);
      msgsock = accept(sock, (struct sockaddr*)&from, &from_len);
	  
      if (msgsock == -1)   //if fail
	       perror("accept");
      else { 
         
         inet_ntop(from.sin_family, &from.sin_addr, buf, sizeof(buf));
         std::cout << "Accepted connection from '" << buf  << "', port " << ntohs(from.sin_port) << std::endl;
         do {

            if((rval = read(msgsock, buf, sizeof(buf))) < 0) //any size greater than 1
		         perror("reading stream message");
	         if (rval == 0)
		         printf("Ending connection\n");
	         else{
               
               // reading the buffer
               // for(int i = 0; i < rval; i++){
               //    //put in 8 bit                                              //byte      =80     ---v
               //    byte = buf[i];     //getting the first 2 hex digits      ascii = get past 7f  623480
               //    total_sum += byte; // acculmating the total hex sum 
               // }

               safe_write(writeFD, buf, rval);

            }
         } while (rval != 0);
	   }  

      std::ostringstream os;
      std::string out_Message;
      
      // os << "Sum: " << total_sum << " Len: " << byte_Count << "\n";
      // out_Message = os.str();
      // const char *cout_Message = out_Message.c_str();

      



      os.clear();

      //reset totals
      // byte_Count = 0;
      // total_sum  = 0;

      //close socket
      close(msgsock);

   } while (true);

   
   /*
    * Since this program has an infinite loop, the socket "sock" is
    * never explicitly closed.  However, all sockets will be closed
    * automatically when a process is killed or terminates normally.
    */
}
