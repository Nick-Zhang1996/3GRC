// UGV_slave 
#include "NL_UGVLink.h"

struct UGV_rc10 buf;

int ard_read(int fd,struct UGV_rc10* buf){
  char *out=(char*)buf;	
  int helper=0;
  int count=0;
	while (count<sizeof(struct UGV_rc10)){
		char temp;
		read(fd,&temp,1);
                      helper++;
		if(temp==1){
			read(fd,&temp,1);
                        helper++;
			if (temp==1){
				*(out+count)=0xff;
			}else if(temp==2){
				*(out+count)=1;

			}
		} else {
			*(out+count)=temp;
		}
		count++;
	}
	
	return helper;


}

//non blocking read and update
//time: <300microseconds
int read_rc10(struct UGV_rc10* buffer){
  
  buffer->len=Serial.parseInt();
  buffer->type=Serial.parseInt();
  int i;
  for (i=0;i<10;i++){
    buffer->chl[i]=Serial.parseInt();
  }
  return 0;
}
void read_pkg(){
  //don't really know why, but prevents unexpected failure
  //delay(5);
   if(Serial.peek()==-1){
     //no data
      return;
   } else if(Serial.peek()!=':'){
     //invalid pkg,discard this byte
      Serial.read(); 
      Serial.println("discard");
      return;
   } else if(Serial.available()<21){
     //not enough data,return
     Serial.println(Serial.available());
      return; 
   } else {
     //process data,update servo and timestamp
     //remove the ':' first
     Serial.println("reading");
     
      int rval;
      rval=Serial.read();
      if(rval!=':'){Serial.println("error!");}
      //rval=Serial.readBytes((char*)&buf,sizeof(buf));
      rval=ard_read(&buf);
      Serial.print("get=");
      Serial.println(rval);
      //ch1.writeMicroseconds(buf.chl[1]);
      //Serial.print("get");
      Serial.print(buf.chl[1]);
      timestamp=millis();
   }
  
}

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(500);
    ch1.attach(pin_ch1);
    ch2.attach(pin_ch2);
    timestamp=millis();
    failsafe();
}

unsigned long pt=0;

void loop() {

   //unsigned long pp=micros();
   read_pkg();
  
   //if(millis()>pt+1000){Serial.println("1");pt=millis();}
   //unsigned long tt=micros()-pp;
   //if(tt>pt){pt=tt;Serial.println(pt);}
   
   //if(timestamp+1000<millis()){
   //  failsafe();
   //}
   
    
}
