/*
  This file declares the data structure the project will use.
  Author : Li Pan
  Date : 2012/12/7 
*/

#ifdef _c_plus_plus
extern "C" {
#endif

#ifndef _DECLARES_H
#define _DECLARES_H

typedef int bool;

/*This is the frame structure in HDLC.*/
typedef struct Frame {

  short  head_flag[8];
  short address[8];
  short control[8];
  struct{
    
    /*Actuall size should also be arry. It is also sequence of '01', here I simplify it.*/
    short size; /*Record the real length of the data.*/
    short information[1024];/*Capacity is 1024.*/
    short infor_type[1024]; /*0 : bit stuffing; 1 : real data.*/
  }s;
   
  short fcs[32];
  short rail_flag[8];
}Frame;

enum FRAME_TYPE {SABME, DISC, UA, REJ, RR, INFOR};/* 6 kinds of frames, first 3 for connection, the other 3 for controlling and data transferring.*/


#endif //_DECLARES_H

#ifdef _c_plus_plus
}
#endif

