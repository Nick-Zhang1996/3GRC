//inet_comm.h
//created by NickZhang on Nov13 2014
//
//
#ifndef inet_comm_h
#define inet_comm_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/types.h>
//connect to asc_addr,port,return socket_fd on success and -1 on failure
int connect_gst(char* asc_addr,int port);

#endif
