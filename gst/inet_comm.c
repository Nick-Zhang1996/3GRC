//inet_comm.c
//created by Nick Zhang on Nov13 2014
//
#include "inet_comm.h"
int connect_gst(char* asc_addr,int port){
    int socket_fd;
	int rval;
    struct addrinfo hint;
    struct addrinfo* target_ai;
    char asc_port[10];

    memset(&hint,0,sizeof(hint));
    hint.ai_family=AF_INET;
    hint.ai_socktype=SOCK_STREAM;
    hint.ai_flags=0;
    hint.ai_protocol=0;

    sprintf(asc_port,"%d",port);
    rval=getaddrinfo(asc_addr,asc_port,&hint,&target_ai);
    if(rval!=0){perror("getaddrinfo");return -1;}
    socket_fd=socket(PF_INET,SOCK_STREAM,0);
    if(rval!=0){perror("socket");return -1;}
    rval=connect(socket_fd,target_ai->ai_addr,target_ai->ai_addrlen);
    if(rval!=0){perror("connect");return -1;}
    /*
    struct in_addr testaddr;
    struct sockaddr_in name;
    memset(&name,0,sizeof(name));
    rval=inet_aton("127.0.0.1",&name.sin_addr);
    name.sin_family=AF_INET;
    name.sin_port=htons(2222);
    rval=connect(socket_fd,(struct sockaddr*)&name,sizeof(name));
	//hostinfo=gethostbyname("nickzhang1996.xicp.net");
    //
   */ 
    freeaddrinfo(target_ai);
    return socket_fd;
}
