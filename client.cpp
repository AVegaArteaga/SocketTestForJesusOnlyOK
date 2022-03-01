//************************************************************************
//
//  CSCI 463 Assignment 7
//
//  Author: Abel Vega Arteaga
//
//  Assignment: IPC (client)
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
#include <sstream>
#include <algorithm>
#include <sstream>

/**
 * @brief same as write, but iincludes a loop to complete any partials
 * 
 * @note watch part 2 13min mark for explaination of safe_write
 * 
 * @param fd  the file descripter the write is writing to 
 * @param buf the character text that is stored in the buffer
 * @param len the legnth of bytes to write to server
 * @return ssize_t 
 */
static ssize_t safe_write(int fd, const char *buf, size_t len){
   
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
 * @brief  read data from the given socket fd & print it until we reach EOF
 * 
 * @note  watch part 2 34min mark for explaination
 * 
 * @param fd file descriptor
 * @return int -1 if fail, 0 if success read. 
 */
static int print_response(int fd){

   char buf[2048];
   ssize_t buf_len = 1;
   
   while(buf_len > 0){

      //keeps going until rval is 0
      if((buf_len = read(fd, buf, sizeof(buf))) == -1){
         perror("reading stream message");
         return -1;
      }
      else if (buf_len > 0){
        buf_len = write(fileno(stdout), buf, buf_len);
      }
   }
   return 0;
}


/**
 * @brief prints an appropriate "Usage" error message and "pattern" to stdeer and terminates the program
 *		   in the traditional manner.
 *
 */
static void usage()
{
	std::cerr << "Usage: client [-s server-ip] server-port" << std::endl;
	std::cerr << "    -s Specify the server’s IPv4 number in dotted-quad format (default: 127.0.0.1)" << std::endl;
	std::cerr << "    The server port number to which the client must connect" << std::endl;
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

   int sock;                  // FD (file descriptor) for the socket connecting the servrer
   struct sockaddr_in server; // Socket address for the server connection

   char port_IP[255] = "127.0.0.1";	///< default threads to 2
   char buf[2048];
   ssize_t len = 1; // used to read the how many bytes, 1 is the priming 

	int opt;                    ///< places the charater

	while ((opt = getopt(argc, argv, "s:")) != -1){
		switch(opt){
		case 's':{
         strncpy(port_IP,optarg,255);
			break;
         }			
		default:{
			   usage();   // prints error if the command does not appear
         }
		}
	}


   //atoi(argv[optind]);
   if(argv[optind] == NULL)
      usage();

   /* Create socket */
   sock = socket(AF_INET, SOCK_STREAM, 0); //AF_INET: I want to use the internet protocal version 4. SOCK_STREAM use TCP protocal. 0 streaming connection and TCP protoccal
   /*can not create socket */
   if (sock < 0) {
	   perror("opening stream socket");
	   exit(1);
   }

   server.sin_family = AF_INET; //we want the other end of the connect to be AF_INET

   if(inet_pton(AF_INET, port_IP, &server.sin_addr) <= 0){
      fprintf(stderr, "%s: invalid address/format\n", port_IP);
      exit(2);
   }

   //setting the port number
   server.sin_port = htons(atoi(argv[optind]));

   if(connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
	   perror("connecting stream socket");
	   exit(1);
   }

   while(len != 0){ /// keeps reading till returns 0
      //256
      len = read(fileno(stdin), buf, sizeof(buf)); //reads the file sizeof(buf) defualt 2048
	   if(safe_write(sock, buf, len) < 0)           //write(fileno(stdout), buf, len);
         perror("writing on stream socket");       // -1 error checking
   }

   // half-close the socket
   // tell the server that no more data will be sent
   // this will cause any subsequent read() calls to return EOF on the server side!
   shutdown(sock, SHUT_WR);
   //getting a response from server
   print_response(sock);
   //close it for good
   close(sock);

   return 0;
}
