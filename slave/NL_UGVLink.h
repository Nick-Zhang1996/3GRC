//
//  NL_UGVLink.h
//  UGV_control
//
//  Created by Nick zhang on 14-11-12.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//

#ifndef UGV_control_NL_UGVLink_h
#define UGV_control_NL_UGVLink_h


#define UGV_RC10 1
#define UGV_RXT 2
#define UGV_CTL 3

struct UGV_uni {
    uint8_t pkg_len;//length of whole package
    uint8_t pkg_type; 
};
struct UGV_rc10 {
    uint8_t pkg_len;
    uint8_t pkg_type;
    uint16_t chl[10];
};

int write_UGV(int fd,void* rdata);
#endif
