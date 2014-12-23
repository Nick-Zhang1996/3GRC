// UGV_slave 
#include <Servo.h>
#include "NL_UGVLink.h"

const int pin_ch1=22;
const int pin_ch2=24;
//pin function:
const int pin_ch[10];

struct UGV_rc10 command;
Servo ch[10];
unsigned long timestamp;


void failsafe(){
  static uint16_t fs_val[10]={1500,1500,1500,1500,1500,1500,1500,1500,1500,1500};
  for(int i=0;i<10;i++){
     command.chl[i]=fs_val[i];
  }
  
  return;
}

//non blocking read and update
//time: <300microseconds
void read_pkg(){
  if(Serial1.peek()==-1){
   //no data
   return;
  } else if(Serial1.peek()!=':'){
    //wrong package
    Serial1.println("disgard");
    Serial1.read();
    return;
  } else if(Serial1.available()<=23){
    //not enough data
    return;
  }else {
    //remove the ':'
    Serial1.read();
    Serial1.readBytes((char*)&command,22);
    //Serial1.println(command.chl[1]);
	int i;
	for(i=0;i<10;i++){
		ch[i].writeMicroseconds(command.chl[i]);
	}
	timestamp=mills();
    return;
  }
  
}

void setup() {
    Serial1.begin(115200);
    Serial1.setTimeout(500);
	for(int i=0;i<10;i++){
		ch[i].attach(pin_ch[i]);
	}
    timestamp=millis();
    failsafe();
}

unsigned long pt=0;

void loop() {

   //unsigned long pp=micros();
   read_pkg();
  
   
   if(timestamp+1000<millis()){
     failsafe();
   }
   
    
}
