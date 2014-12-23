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

//declare global vars
//frequency of update_serial,not precise,unit Hz
double freq=50.0;
//port of gst,host order
int gst_port=2222;
//ascii address,can be hostname or ip
char gst_addr_asc[30]="127.0.0.1";
char* serial_port;
char serial_port1[]="/dev/ttyACM0";
char serial_port2[]="/dev/ttyACM1";
char serial_port3[]="/dev/ttyAMA0";
//declare shared buffer
struct UGV_rc10 command;
pthread_mutex_t command_lock=PTHREAD_MUTEX_INITIALIZER;
int serial_fd;
int gst_fd;
sig_atomic_t end_now=0;


//declare functions
void* get_pkg(void*);
void* send_cmd(void*);
void handler(int);
void perr(char*);
int ascsend(int fd,int data);
int send_rc10(int fd,struct UGV_rc10* pkg);
size_t ard_enc(void* in,void* out,size_t in_count);
int main(int argc, const char * argv[]) {
	
//if supplied an address, use it,otherwise use default
	if(argc>=2){
		strcpy(gst_addr_asc,argv[1]);
		if(argc==3){
			gst_port=atoi(argv[2]);
		}
	}	
//stage 1 open serial and create inet socket
//init internet
   int rval;
   printf("connecting %s:%d\n",gst_addr_asc,gst_port);
  gst_fd=connect_gst(gst_addr_asc,gst_port); 
  if(gst_fd==-1){return -1;}
  else {printf("connect success!\naddress:%s port:%d\n",gst_addr_asc,gst_port);}

//init serialport
    //int timeout=500;
    int baud_rate=115200;
    
    rval=access(serial_port1,F_OK);
    serial_port=serial_port1;
    if(rval==-1){
	rval=access(serial_port2,F_OK);
        serial_port=serial_port2;
        if(rval==-1){
			rval=access(serial_port3,F_OK);
			serial_port=serial_port3;
			if(rval==-1){
                printf("can't find serial port\n");
                return -1;
			}
    	}
    }
    printf("find serial port %s\n",serial_port);
    printf("initializing serial port ... \n");
    serial_fd=serialport_init(serial_port, baud_rate);
    assert(serial_fd!=-1);
    sleep(1);
    printf("serial port initialized,baudrate:%dbps freq=%0.1fHz\n",baud_rate,freq);
//init command pkg
    command.len=sizeof(command);
    command.type=UGV_RC10;
    command.chl[1]=1500;
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
    return 0;
}


void* get_pkg(void* a){
    int rval;
    struct UGV_rc10 this_pkg;
    while (1){
        //uint8_t len;
            char test;
            int flag_error=0;
            //read a ':'which indicates the start of a UGV pkg
            rval=read(gst_fd,&test,sizeof(char));
			if(rval!=1){
				printf("get %d from read exp':'\n",rval);
				perror("get_pkg_read");
				flag_error=1;
			}

			else if (rval==0){perr("no more data");flag_error=1;continue;}
			if(test!=':'){perr("ignore mismatch pkg");flag_error=1;continue;}
			//read the length of the pkg
			rval=read(gst_fd,&this_pkg.len,1);
			if(rval!=1){perror("get_pkg_read length");flag_error=1;}
			if(!flag_error){
				size_t count=this_pkg.len-1;
				while(count>0){
					rval=read(gst_fd,&this_pkg.type,count);
					if(rval==-1){perror("get_pkg read whole pkg");flag_error=1;break;}
					count-=rval;
				}
				if(!flag_error){
					pthread_mutex_lock(&command_lock);
					command=this_pkg;
					pthread_mutex_unlock(&command_lock);
					//fprintf(stderr,"get%d\n",command.chl[1]);
				}
			}
			if(end_now){return NULL;}
    }
    
    return NULL;

}


void* send_cmd(void* a){
    int rval;
    struct timespec delay;
	struct UGV_rc10 local;
	uint8_t out_buf[50];
    delay.tv_sec=0;
    delay.tv_nsec=1000000000.0*(1.0/freq);
    while (1){
		uint8_t starter=':';
		write(serial_fd,&starter,1);
        pthread_mutex_lock(&command_lock);
		write(serial_fd,&command,sizeof(command));
    	pthread_mutex_unlock(&command_lock);
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
size_t ard_enc(void* in,void* out,size_t in_count){
	uint8_t* buf=(uint8_t*)in;
	int rval;
	int i=0;
	uint8_t* p=out;
	int out_count=0;
	while(i<in_count){
		if(*(buf+i)==0xff){
			*p=1;
			p++;
			*p=1;
			p++;
			out_count+=2;
			
		}else if(*(buf+i)==1){
			*p=1;
			p++;
			*p=2;
			p++;
			out_count+=2;
		}
		else {
			*p=*(buf+i);
			p++;
			out_count++;
		}
		i++;
	}	
	return out_count;
}
