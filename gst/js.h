#ifndef JS_H
#define JS_H

#include <pthread.h>
#include <sys/timeb.h>
#include <curses.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <assert.h>
#include "NL_UGVLink.h"

#define NUM_THREADS        3
#define JOY_DEV "/dev/input/js0"
#define HZ  12

#define MAX_RC_VALUE 2000
#define MIN_RC_VALUE 1000
#define MIN_JS_VALUE -32767
#define MAX_JS_VALUE 32767
extern struct UGV_rc10 command;
extern int num_of_axis,num_of_buttons,js_fd;

extern pthread_mutex_t aab_lock,command_lock;
extern int axis[10],button[10];
//init the joystick and return js_fd on success,-1 on failure
int joystick_init(char* js_path);

//read from fd_p,and store correct value in axis[] and button[]
void* read_js(void* fd_p);
//display raw value to screen
void* display(void* unused);
//map joystick value to command pkg
void map_j2c();
int map(int num,int clow,int chigh,int tlow,int thigh );
int res(int num,int low,int high);

#endif
