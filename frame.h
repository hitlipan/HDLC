/*
   Author : Li Pan
   Date : 2012/12/8
 */

#ifndef _FRAME_H
#define _FRAME_H

#include "declares.h"

/* SABME, UA and DISC*/
Frame create_unnumbered_frame(short addr[], enum FRAME_TYPE ft, bool p_f);

/* REJ, RR*/
Frame create_sup_frame(short addr[], enum FRAME_TYPE ft, bool p_f, short next_receive);

/* Frame for normal data transferring.*/
Frame* create_infor_frames(short addr[], short send_number, bool p_f, 
                           short receive_number, short data[], int start, int end, Frame *frames);/* Using $start,$end, better than $size*/

/*Extract frame type from the given frame. */
enum FRAME_TYPE get_frame_type(Frame *p_frame);


/* Extract address field from the given frame.*/
void get_address(Frame *p_frame, short *storage);

/* The # of the frame.*/
short get_send_number(Frame *p_frame);

/* The # of the frame sender wants.*/
short get_expect_number(Frame *p_frame);

/* Extract the real data from the given frame.*/
void get_infor(Frame *p_frame, short *storage, int *size);


/*Here using fcs to judge whether the data is right.*/
bool is_fcs_right(Frame *p_frame);


#endif

