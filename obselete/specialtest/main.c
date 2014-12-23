//
//  main.c
//  UGV_control
//  for use on RPi to control arduino via serial or USBSerial
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
#include "arduino-serial-lib.h"
#include "NL_UGVLink.h"
#include "inet_comm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//declare global vars
char* serial_port;
char serial_port1[]="/dev/ttyACM0";
char serial_port2[]="/dev/ttyACM1";
//declare shared buffer
struct UGV_rc10 command;
pthread_mutex_t command_lock=PTHREAD_MUTEX_INITIALIZER;
int serial_fd;
int gst_fd;
sig_atomic_t end_now=0;
double freq=20.0;
int ran;
//declare functions
void* get_pkg(void*);
void* send_cmd(void*);
void handler(int);
void perr(char*);
int ascsend(int fd,int data);
int send_rc10(int fd,struct UGV_rc10* pkg);
int ard_send(int fd,void* r_buf,size_t count);
int get_r(){
	unsigned int val;
	read(ran,&val,sizeof(unsigned int));
	val%=1000;
	val+=1000;	
	return (int)val;
}
int main(int argc, const char * argv[]) {
	
	ran=open("/dev/urandom",O_RDONLY);

//stage 1 open serial and create inet socket
//init serialport
    //int timeout=500;
    int baud_rate=115200;
   int rval; 
    rval=access(serial_port1,F_OK);
    serial_port=serial_port1;
    if(rval==-1){
	rval=access(serial_port2,F_OK);
        serial_port=serial_port2;
        if(rval==-1){
                printf("can't find serial port\n");
                return -1;
    	}
    }
    printf("find serial port %s\n",serial_port);
    printf("initializing serial port ... \n");
    serial_fd=serialport_init(serial_port, baud_rate);
    assert(serial_fd!=-1);
    sleep(1);
//init command pkg
    command.len=sizeof(command);
    command.type=UGV_RC10;
	int i;
	for (i=0;i<10;i++){
    command.chl[i]=1500;
	}


int rr=0;
uint8_t a=':';
rr+=write(serial_fd,&a,1);
rr+=ard_send(serial_fd,(void*)&command,sizeof(command));
uint8_t b[200];
printf("send=%d\n",rr);
memset(b,0,200);
usleep(30000);
read(serial_fd,b,200);
printf("%s",b);
close(serial_fd);
return 0;
//create two threads
    pthread_t id_get_pkg,id_send_cmd;
    pthread_create(&id_get_pkg,NULL,&get_pkg,NULL);
    pthread_create(&id_send_cmd,NULL,&send_cmd,NULL);
//handle the SIGINT signal,(ctrl+c)
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler=&handler;
    sigaction(SIGINT,&sa,NULL);
//loop forever
    while(!end_now){sleep(1);};
//wrapup    
    perr("ending...");
    perr("waiting get_pkg");
    pthread_join(id_get_pkg,NULL);
    perr("waiting send_cmd");
    pthread_join(id_send_cmd,NULL);
    perr("closing serialport");
    serialport_close(serial_fd);
    perr("closing gst_fd");
    close(gst_fd);
	close(ran);
    return 0;
}


void* get_pkg(void* a){
	int i,step=1;
	for (i=1500;;i+=step)
	{
					pthread_mutex_lock(&command_lock);
					command.chl[1]=get_r();
					pthread_mutex_unlock(&command_lock);
					if(i+step>2000||i+step<1000){step=-step;}
			if(end_now){return NULL;}
			usleep(70000);
    }
    
    return NULL;

}


void* send_cmd(void* a){
    int rval;
    struct timespec delay;
    delay.tv_sec=0;
    delay.tv_nsec=1000000000.0*(1.0/freq);
    while (1){
    //to indicate ardu the start of a package;
        rval=write(serial_fd,":",1);
		if(rval!=1){perror("write");}
        pthread_mutex_lock(&command_lock);
       // rval=write(serial_fd,&command,sizeof(command));
	//assert(rval==sizeof(command));
//	    fprintf(stderr,"byte=%d val=%d\n",rval,command.chl[1]);
		rval=ard_send(serial_fd,&command,sizeof(command));	
    	pthread_mutex_unlock(&command_lock);
	fprintf(stderr,"real=%d\n",rval);	
		//if(rval==-1){perr("send_rc10");continue;}	
        if(end_now) {return NULL;}
	    nanosleep(&delay,NULL);
    //for test purpose
        char temp[30];
        memset(temp,0,30);
        rval=read(serial_fd,temp,20);
		//if(rval!=0){perr("noget");exit(1);}
		if(rval>0){perr(temp);}
    } 
    return NULL;
}
void handler(int signum){
    end_now=1;
    return;
}

void perr(char* msg){
    fprintf(stderr,msg);
    fprintf(stderr,"\n");
    return ;
}
int ascsend(int fd,int data){
	char temp[10];
	sprintf(temp,"%d ",data);
	int rval=write(fd,temp,strlen(temp));
	if(rval!=strlen(temp)){return -1;}
	//perr(temp);
	return 0;
}
int send_rc10(int fd,struct UGV_rc10* pkg){

	int rval;
	rval=ascsend(fd,pkg->len);
	if(rval==-1){return -1;}
	rval=ascsend(fd,pkg->type);
	if(rval==-1){return -1;}
	int i;
	for (i=0;i<10;i++){
	rval=ascsend(fd,pkg->chl[i]);
	if(rval==-1){return -1;}
				}
	return 0;
}
//send buf,avoid 0xff, use \1 (escape)\1 instead, \1 is replaced by \1\2
int ard_send(int fd,void* r_buf,size_t count){
	uint8_t changed[50];
	uint8_t* buf=(uint8_t*)r_buf;
	int rval;
	uint8_t e_ff[2]={1,1};
	uint8_t e_1[2]={1,2};
	
	int i=0;
	uint8_t* p=changed;
	int cgd_count=0;
	while(i<count){
		if(*(buf+i)==0xff){
			*p=1;
			p++;
			*p=1;
			p++;
			cgd_count+=2;
			
		}else if(*(buf+i)==1){
			*p=1;
			p++;
			*p=2;
			p++;
			cgd_count+=2;
		}
		else {
			*p=*(buf+i);
			p++;
			cgd_count++;
		}
		i++;
	}	
	rval=write(fd,changed,cgd_count);
	assert(rval==cgd_count);
	return cgd_count;
}
