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

