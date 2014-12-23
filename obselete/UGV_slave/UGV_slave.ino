// UGV_slave 
#include <Servo.h>
#include "NL_UGVLink.h"

const int pin_ch1=22;
const int pin_ch2=24;

struct UGV_rc10 buf;
Servo ch1,ch2;
unsigned long timestamp;


void failsafe(){
  static uint16_t fs_val[10];
  for(int i=0;i<10;i++){
     fs_val[i]=1500; 
     buf.chl[i]=fs_val[i];
  }
  
  return;
}
int parse(uint8_t* in_buf,uint8_t* out_buf,int in_len){
  uint8_t* p_read=in_buf;
  uint8_t* p_out=out_buf;
  //number of bytes read
  int count=0;
  while(count<in_len){
    if(*p_read==1){
        p_read++;
        count++;
        if (*p_read==1){
            *p_out=0xff;
            p_out++;
            
        }else if(*p_read==2){
            *p_out=1;
            p_out++;
        }
     } else {
          *p_out=*p_read;
          p_out++;
      }
  p_read++;
  count++;
  }
  return p_out-out_buf;
}


int ard_read(struct UGV_rc10* buf){
  uint8_t *out=(uint8_t*)buf;	
  //total read
  int helper=0;
  //total write
  int count=0;
	while (count<sizeof(struct UGV_rc10)){
		char temp=Serial.read();
                      helper++;
		if(temp==1){
			temp=Serial.read();
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
	return helper-count;


}
int this_len=-1;
int expect=2;
void read_pkg(){
  uint8_t buffer[50];
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
   } else if(Serial.available()<expect){
     return;
   } else{
     //remove the ':'which indicates the start of a pkg
      Serial.read();
      if(this_len==-1){
        //this pkg tells us the length
        uint8_t temp=Serial.read();
        
        
        expect=temp;
        this_len=temp;
      
      } else{
        //this is a data pkg
        int rval=Serial.readBytes((char*)buffer,this_len);
        if(rval!=this_len){Serial.print("readbytes");}
        rval=parse(buffer,(uint8_t*)&buf,this_len);
        if(rval!=22){Serial.print("parse");}
        Serial.println(buf.chl[1]);
        ch1.writeMicroseconds(buf.chl[1]);
       this_len=-1; 
       expect=2;
       timestamp=millis();
      }
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
