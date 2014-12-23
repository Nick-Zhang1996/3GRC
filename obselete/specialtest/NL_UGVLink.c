//
//  NL_UGVLink.c
//  UGV_control
//
//  Created by Nick zhang on 14-11-12.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//

#include <stdio.h>
#include "NL_UGVLink.h"

int write_UGV(int fd,void* rdata){
    struct UGV_uni* data=(struct UGV_uni*)rdata;
    size_t len=data->len;
    size_t rval=write(fd,data,len);
    if (rval!=len){return -1;}
    return 0;
}

int raw2asc(struct UGV_rc10 * in,struct UGV_rc10_asc* out){
	memset(out,0,sizeof(*out));
	sprintf(out->len,"%d",in->len);
	sprintf(out->type,"%d",in->type);
	int i;
	for (i=0;i<10;i++){
	sprintf(out->chl[i],"%d",in->chl[i]);
	}
	return 0;
}

