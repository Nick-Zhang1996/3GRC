//
//  main.c
//  UGV_gst
//  for use on Linux PC to send UGV packages over internet
//  This program acts as a server,client would have to rely on DDNS or staticIP address to connect to it
//  Created by Nick zhang on 14-11-12.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <sys/select.h>
#include "NL_UGVLink.h"
#include "inet_comm.h"
#include "js.h"

//declare global vars

//frequency of update,not precise,unit Hz
double freq=50.0;

//global flags

sig_atomic_t end_now=0;

//declare shared buffer
struct UGV_rc10 command;
pthread_mutex_t command_lock=PTHREAD_MUTEX_INITIALIZER;
int server_port=2222;
int client_port;
//declare functions
void* send_cmd(void*);
void handler(int);
int init_server(int port);
void* update_joystick(void*);
void perr(char* msg);


int main(int argc, const char * argv[]) {

    int rval;
//init command pkg
    command.len=sizeof(command);
    command.type=UGV_RC10;
	int i;
	for(i=0;i<10;i++){
    command.chl[i]=1500;
	}
//start inet server
    
    int server_fd=init_server(server_port);
    if(server_fd!=-1){printf("server started on port:%d\n",server_port);}
                else {return -1;}
//init joystick

    js_fd=joystick_init(JOY_DEV);
    if(js_fd==-1){fprintf(stderr,"can't open js");return -1;}
		else {printf("joystick detected\n");}

//initialize updater(input from joystick)
    pthread_t id_read_js;
    pthread_create(&id_read_js,NULL,&read_js,(void*)&js_fd);

//initialize local vars
    pthread_t id_send_cmd;
    struct sockaddr_in client_addr;
    memset(&client_addr,0,sizeof(client_addr));
    socklen_t client_addr_len=sizeof(client_addr);
    int client_fd;

    fd_set master,current;
    int fdmax;
    FD_ZERO(&master);
    FD_ZERO(&current);
    FD_SET(server_fd,&master);
    fdmax=server_fd;
    //define the time select blocks in every loop
    //this may have a large inpact on CPU usage
    struct timeval tot,tmp;
    tot.tv_sec=1;
    tot.tv_usec=0;
//handle the SIGINT signal
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler=&handler;
    sigaction(SIGINT,&sa,NULL);


    while(1){
       current=master;
       tmp=tot;
       rval=select(fdmax+1,&current,NULL,NULL,&tmp); 
       if(rval>0){
           if(FD_ISSET(server_fd,&current)){
           //create a thread to handle the connection
	   printf("find a connection request!\n");
               client_fd=accept(server_fd,(struct sockaddr*)&client_addr,&client_addr_len);


               if(client_fd==-1){
                       perror("accept");
                       close(server_fd);
                       return -1;
               }else {
                 pthread_create(&id_send_cmd,NULL, &send_cmd,(void*)&client_fd);
                 printf("accept connect from %s port:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
               }
           }     
            //maybe its just stdin
       } 
       else if(rval==-1){perror("select");return -1;}

	   //fprintf(stderr,"%d\n",command.chl[1]);
       if(end_now){
            break;
       } 
    }
    
//wrapup    
    printf("ending...\n");
    pthread_join(id_send_cmd,NULL);
	pthread_join(id_read_js,NULL);
    close(client_fd);
	close(js_fd);
    return 0;
}




void* send_cmd(void* arg_p){
    int fd=*(int *)arg_p;
    int rval;
    struct timespec delay;
    delay.tv_sec=0;
    delay.tv_nsec=1000000000.0*(1.0/freq);
    while (1){
    //to indicate recipant the start of a package;
        write(fd,":",1);
        pthread_mutex_lock(&command_lock);
        rval=write(fd,&command,sizeof(command));
	    //fprintf(stderr,"write=%d send=%d\n",rval,command.chl[1]);
    	pthread_mutex_unlock(&command_lock);
        if(end_now){return NULL;}
	    nanosleep(&delay,NULL);
    //for test purpose
    /*    char temp[30];
        memset(temp,0,30);
        rval=read(serial_fd,temp,20);
        if(rval>0){fprintf(stderr,"%s",temp);}
	*/
    } 
    return NULL;
}
//this is a rudimentary test function,will be replaced in final release
void* update_joystick(void* nonsense){
    int temp=1500;
    int step=10;
    while(1){
        //scanf("%d",&temp);
        temp+=step;
        if(temp>=2000||temp<=1000){step=-step;}
        usleep(300000);
        pthread_mutex_lock(&command_lock);
        command.chl[1]=temp;
        pthread_mutex_unlock(&command_lock);
        if(end_now){
                return NULL;
        }
    }
    return NULL;
}

int init_server(int port){
    int server_fd;
    struct sockaddr_in local;
    int rval;

    server_fd=socket(PF_INET,SOCK_STREAM,0);
    if(server_fd==-1){perror("server socket creation");return -1;}

    memset(&local,0,sizeof(local));

    local.sin_family=AF_INET;
    local.sin_port=htons(port);
    local.sin_addr.s_addr=INADDR_ANY;
    rval=bind(server_fd,(struct sockaddr*)&local,sizeof(local));
    if(rval!=0){perror("bind");close(server_fd);return -1;}
    
    rval=listen(server_fd,5);
    if(rval!=0){perror("listen");close(server_fd);return -1;}
    return server_fd;

}
void handler(int signum){
    end_now=1;
}
void perr(char* msg){
	fprintf(stderr,msg);
	fprintf(stderr,"\n");
	return ;
}
