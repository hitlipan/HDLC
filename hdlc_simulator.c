/*
   This file simulates the data transfer process of hdlc.

   Author : Li Pan
   Time : 2012/12/9 12:09
   Version : 0.3
 */

#include "frame.h"
#include "declares.h"
#include "net.h"

#include <stdio.h>

extern int window_length;
extern int raw_data_length;
extern int frame_number_range;
extern int maximum_data_transferred;
extern int time_out;

/* Store the frames from the other side.*/
Frame storage[10];

static int sending_rr(short addr[], int *wanted_frame_number, int socket_fd) {
  
  Frame rr_frame;
  rr_frame = create_sup_frame(addr, RR, 1, *wanted_frame_number); 
  hdlc_write((void *)&rr_frame, sizeof(Frame), 1, socket_fd); /* Send RR frame*/
  return 1;
}

static int sending_infor(short addr[], short data[], int *start_position, int *start_frame_number, 
                           int *wanted_frame_number, Frame current_frame, int socket_fd) {
  int infor_wanted_number;
  Frame send_frames[10];
  infor_wanted_number = get_expect_number(&current_frame); 
  *start_position += raw_data_length * 
                      ((infor_wanted_number < (*start_frame_number) ? infor_wanted_number + frame_number_range : infor_wanted_number) - (*start_frame_number));
  *start_frame_number = infor_wanted_number;

  create_infor_frames(addr, *start_frame_number, 1, *wanted_frame_number, data, 
                        *start_position, (*start_position) + window_length * raw_data_length, send_frames); 
  hdlc_write((void *)send_frames, sizeof(Frame), window_length, socket_fd);
  return 2;
}

static int rr_information(short addr[], short data[], int *start_position, int *start_frame_number, 
                           int *wanted_frame_number, Frame current_frame, int socket_fd) {
  
  int result;
  if (rand() % 2) {
    /*
      Return RR 
     */
    result = sending_rr(addr, wanted_frame_number, socket_fd);
  } else {
    /*Return Info*/
    result = sending_infor(addr, data, start_position, start_frame_number, wanted_frame_number, current_frame, socket_fd);
  }  
  return result;
}

int simulate(short addr[], short data[], int *start_position, int *start_frame_number, int socket_fd, int *wanted_frame_number) {

  int read_num;
  Frame frame;
  Frame current_frame;
  Frame rr_frame;
  Frame ua_frame;
  Frame disc_frame;
  Frame send_frames[10];
  enum FRAME_TYPE f_type; 
  int i;

  int rej_wanted_number;
  int rr_wanted_number;
  
  int random_select;
  /*Here read $window_length frames. 
    Sometimes a frame may be a sup frame, only one frame.
    But at most $window_length frames.
   */
  read_num = hdlc_read((void *)storage, sizeof(Frame), window_length, socket_fd);
  
  /* Sleep a little time*/
  sleep(time_out / 2);

  if (read_num == 0) {
 
    fprintf(stderr, "***Time out!***\n");  
    fprintf(stdout, "Sending RR %d\n", *wanted_frame_number);
    sending_rr(addr, wanted_frame_number, socket_fd);
    return 0;
  }

  if (!(rand() % 5)) {
    sleep(time_out + 1);
  }
  for (i = 0; i < read_num; ++i) {
    
    current_frame = storage[i];
    
    /* Introduce data error event.*/
    if (!(rand() % 9)) {
      filter(&current_frame, 1);
    }

    /* Data is wrong. Throw all the data which is after the error data.*/
    if (!is_fcs_right(&current_frame)) {
      
      /*Send REJ frame*/

      frame = create_sup_frame(addr, REJ, 1, *wanted_frame_number);

      /* Send the data to the other side.*/
      hdlc_write((void *)&frame, sizeof(Frame), 1, socket_fd);

      fprintf(stdout, "***Receiving Data Error***\n");
      fprintf(stdout, "Sending REJ %d\n", *wanted_frame_number);
      return 0;
    } else {

      f_type = get_frame_type(&current_frame);
      switch(f_type) {
        case SABME: 
          fprintf(stdout, "Receiving SABME\n"); 

        case DISC : 
          if (f_type == DISC) {
            fprintf(stdout, "Receiving DISC\n");
          }
         
          /* Here Sending UA frame.*/
  
          ua_frame = create_unnumbered_frame(addr, UA, 1);
          hdlc_write((void *)(&ua_frame), sizeof(Frame), 1, socket_fd);

          fprintf(stdout, "Sending UA\n");

          if (f_type == DISC) {
            return 1;
          }
        break;
    
        case UA : 
          fprintf(stdout, "Receiving UA\n");

          /*
             if (*start_position == 0), knowing it is the beginning.
           */

            if (0 == (*start_position)) {
	      current_frame.control[5] = 0;
              current_frame.control[6] = 0;
              current_frame.control[7] = 0;              
              random_select = rr_information(addr, data, start_position, start_frame_number, wanted_frame_number, current_frame, socket_fd);
              if (1 == random_select) {
                fprintf(stdout, "Sending RR %d\n", *wanted_frame_number);
              } else {
                fprintf(stdout, "Sending Info %d %d %d %d\n", *start_frame_number, (*start_frame_number + 1) % frame_number_range, (*start_frame_number + 2) % frame_number_range,  *wanted_frame_number);
              }
            } else {
              /* The end of this conversation.*/
              return 1; /* '1' stands for the end. */
            }
        break;
    
        case REJ : 
          rej_wanted_number = get_expect_number(&current_frame);
          fprintf(stdout, "Receiving REJ %d\n", rej_wanted_number);
          /*Here change the parameter transferred.*/
          *start_position += raw_data_length * 
                      ((rej_wanted_number < (*start_frame_number) ? rej_wanted_number + frame_number_range : rej_wanted_number) - (*start_frame_number));
        
          *start_frame_number = rej_wanted_number;
           /* Create $window_length frames.*/
           create_infor_frames(addr, *start_frame_number, 1, *wanted_frame_number, data, 
                                *start_position, (*start_position) + window_length * raw_data_length, send_frames);       
          
           hdlc_write((void *)send_frames, sizeof(Frame), window_length, socket_fd);
           fprintf(stdout, "Sending infor %d %d %d %d\n", *start_frame_number, (*start_frame_number + 1) % frame_number_range, (*start_frame_number + 2) % frame_number_range,  *wanted_frame_number);
        break;
    
        case RR : 
          fprintf(stdout, "Receiving RR %d\n", get_expect_number(&current_frame));
          random_select = rr_information(addr, data, start_position, start_frame_number, wanted_frame_number, current_frame, socket_fd);
          if (1 == random_select) {

            fprintf(stdout, "Sending RR %d\n", *wanted_frame_number); 
          } else {

            fprintf(stdout, "Sending infor %d %d %d %d\n", *start_frame_number, (*start_frame_number + 1) % frame_number_range, (*start_frame_number + 2) %                         frame_number_range, *wanted_frame_number);
          }
        break;
    
        case INFOR : 

         
          /* Judge if the information is repetitive.*/
          if (get_send_number(&current_frame) != (*wanted_frame_number)) {
            
            fprintf(stdout, "Throwing repetitive infor %d\n", get_send_number(&current_frame));
          }
          else {
            
            fprintf(stdout, "Receiving infor %d\n", get_send_number(&current_frame));
            *wanted_frame_number = (*wanted_frame_number + 1) % frame_number_range;
          }

          /* Receive all the information frames, send data to the other side.*/
          if (i == read_num - 1) {
            if ((*start_position) < maximum_data_transferred) {
              random_select = rr_information(addr, data, start_position, start_frame_number, wanted_frame_number, current_frame, socket_fd);
   	      if (1 == random_select) {
                fprintf(stdout, "Sending RR %d\n", *wanted_frame_number);
              } else {
                fprintf(stdout, "Sending infor %d %d %d %d\n", *start_frame_number, (*start_frame_number + 1) % frame_number_range, (*start_frame_number + 2) % frame_number_range, *wanted_frame_number);
              }
            } else {
               /*SENDING DISC*/
               disc_frame = create_unnumbered_frame(addr, DISC, 1);
               hdlc_write((void *)&disc_frame, sizeof(Frame), 1, socket_fd);
               fprintf(stdout, "Sending DISC\n");
            }
          }
        break;
  
        default : 
          fprintf(stdout, "simulate : Default!\n");
        break;
      }
    }
  }
  return 0;
}



/* Here introduce some errors to the given frame.*/
void filter(Frame *p_frame, int errorNo) {
  int i;
  for (i = 0; i < errorNo; ++i) {

    p_frame ->s.information[i] ^= 1;
  }
}

