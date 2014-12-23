#include "js.h"
//modified by Nick Zhang on Nov 19 2014 to suit UGV needs
extern struct UGV_rc10 command;
int num_of_axis,num_of_buttons,js_fd;

pthread_mutex_t aab_lock=PTHREAD_MUTEX_INITIALIZER;
int axis[10],button[10];
//init the joystick and return js_fd on success,-1 on failure
int joystick_init(char* js_path){
        int cnt, joy_fd, *axis=NULL,x;
        char *button=NULL, name_of_joystick[80];
        struct js_event js;
        int rval;

        joy_fd = open( js_path , O_RDONLY); 
        if(joy_fd==-1){return -1;}

        ioctl( joy_fd, JSIOCGAXES, &num_of_axis );                                //        GET THE NUMBER OF AXIS ON JS
        ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );                //        GET THE NUMBER OF BUTTONS ON THE JS
        //name is not of interest
        //ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );        //        GET THE NAME OF THE JS
      

//        CHANGE THE STATUS FLAG OF THE FILE DESCRIPTOR TO NON-BLOCKING MODE
        fcntl( joy_fd, F_SETFL, O_NONBLOCK ); 
        return joy_fd;
}
void* read_js(void* fd_p){
        int joy_fd=*(int *)fd_p;
        struct js_event js;
        int rval;
        fd_set master,current;
        int fdmax=joy_fd;
        FD_ZERO(&master);
        FD_SET(joy_fd,&master);
        //define the time select blocks in every loop
        //this may have a large inpact on CPU usage
        struct timeval tot,tmp;
        tot.tv_sec=1;
        tot.tv_usec=0;

        while( 1 ){
            //uncomment to add frequency control
            //usleep(16666);

            current=master;
            tmp=tot;
            rval=select(fdmax+1,&current,NULL,NULL,&tmp);
            if(rval==-1){perror("js select");return NULL;}
            else if(rval>0){
                if(FD_ISSET(joy_fd,&current)){
                    //        READ THE JOYSTICK STATE, IT WILL BE RETURNED IN THE JS_EVENT STRUCT
                    rval=read(joy_fd, &js, sizeof(struct js_event));
                    //just curious...
                    assert(rval==sizeof(struct js_event));
                    //        CHECK THE EVENT
		    pthread_mutex_lock(&aab_lock);
                    switch (js.type & ~JS_EVENT_INIT){
                        case JS_EVENT_AXIS:
                            axis  [ js.number ] = js.value;
                            break;
                        case JS_EVENT_BUTTON:
                            button [ js.number ] = js.value;
                            break;
                    }
		    pthread_mutex_unlock(&aab_lock);
			map_j2c();
                }
            }                       
        }
}

void* display(void* unused){
        
    while(1){
        usleep(10000);
        pthread_mutex_lock(&aab_lock);
        int i;
        for (i=0;i<num_of_axis;i++){
            printf("%6d",axis[i]);
        }
        for (i=0;i<num_of_buttons;i++){
            printf("%6d",button[i]);
        }
        printf("\n");

        pthread_mutex_unlock(&aab_lock);
    }
    return NULL;
}
/*
int main(int argc, char *argv[]){
    js_fd=joystick_init(JOY_DEV);
    if(js_fd==-1){fprintf(stderr,"can't open js");}
    pthread_t id_read_js,id_display;
    pthread_create(&id_read_js,NULL,&read_js, (void *)&js_fd);
    pthread_create(&id_display,NULL,&display,NULL);
    getchar();    
    pthread_join(id_read_js, NULL);

    return 0;
}
*/

int map(int num,int clow,int chigh,int tlow,int thigh ){
	num=res(num,clow,chigh);
	return (num-clow)*(thigh-tlow)/(chigh-clow)+tlow;
}

int res(int num,int low,int high){
	if (num>high) return high;
	if (num<low) return low;
	return num;
}
//yet to be completed
void map_j2c(){
	pthread_mutex_lock(&aab_lock);
	pthread_mutex_lock(&command_lock);
	command.chl[1]=map(axis[0],MIN_JS_VALUE,MAX_JS_VALUE,MIN_RC_VALUE,MAX_RC_VALUE);

	pthread_mutex_unlock(&aab_lock);
	pthread_mutex_unlock(&command_lock);
	return;
}






