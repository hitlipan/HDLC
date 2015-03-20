/* 
  This file implements the universal algoritms supporting
    the other part of the project.
  Author : Li Pan
  Date : 2012/12/8
 */

#include "declares.h"
#include "frame.h"

#include <stdio.h>

extern int window_length;
extern int raw_data_length;
extern int frame_number_range;


/* 33 crc*/
short crc[] = {1, 0, 0, 0, 0, 0,
               1, 0, 0, 1, 1, 0,
               0, 0, 0, 0, 1,
               0, 0, 0, 1, 1, 1,
               0, 1, 1, 0, 1, 1,
               0, 1, 1, 1};



short send_receive_num[][3] = {{0, 0, 0},
                               {0, 0, 1},
                               {0, 1, 0},
                               {0, 1, 1},
                               {1, 0, 0},
                               {1, 0, 1},
                               {1, 1, 0},
                               {1, 1, 1}
                              };

static Frame *fill_flag_and_fcs(Frame *p_frame) {
  
  short s[] = {0, 1, 1, 1, 1, 1, 1, 0};
  int i;
  for (i = 0; i < 8; ++i) {
    p_frame ->head_flag[i] = s[i];
    p_frame ->rail_flag[i] = s[i];
  }
  for (i = 0; i < (sizeof(crc) / sizeof(short) - 1); ++i) {
    p_frame ->fcs[i] = 0;
  }
  
  return p_frame;
}

static Frame *create_generic_unnumbered_frame(Frame *p_frame) {

  fill_flag_and_fcs(p_frame);
  p_frame ->control[0] = 1;
  p_frame ->control[1] = 1;
  p_frame ->s.size = 0;
  return p_frame;
}

static Frame *create_generic_info_frame(Frame *p_frame) {

  fill_flag_and_fcs(p_frame);
  p_frame ->control[0] = 0;
  p_frame ->s.size = 0;
  return p_frame;
}

static Frame *create_generic_sup_frame(Frame *p_frame) {

  fill_flag_and_fcs(p_frame);
  p_frame ->control[0] = 1;
  p_frame ->control[1] = 0;
  p_frame ->s.size = 0;
  return p_frame;
}


/*-------------------------------------------
   Extract the following code from
    the util.c file in version 2.
-------------------------------------------*/
static void convert_frame_to_seq(Frame *p_frame, short seq[], int *size) {

  int i;
  (*size) = 0;
  for (i = 0; i < 8; ++i) {
    seq[*size] = p_frame ->address[i];
    ++(*size);
  }
  
  for (i = 0; i < 8; ++i) {
    seq[*size] = p_frame ->control[i];
    ++(*size);
  }
  
  for (i = 0; i < p_frame ->s.size; ++i) {
    if (1 == (p_frame ->s.infor_type[i])) {
      seq[*size] = p_frame ->s.information[i];
      ++(*size);
    }
  }
  for (i = 0; i < sizeof(crc) / sizeof(short) - 1; ++i) {
    seq[*size] = p_frame ->fcs[i];
    ++(*size);
  }
}
/* 0 has been filled before calling this method.*/

static int seq_fcs(short seq[], int size) {
  
  int i;
  int non_zero = 0;
  int temp_non_zero;
  bool new_non_zero = 0;

  for (i = 0; i < size; ++i) {
    if (1 == seq[i]) {
      non_zero = i;
      new_non_zero = 1;
      break;
    }
  }
  
  for (;non_zero <= size - sizeof(crc)/sizeof(short);) {
    new_non_zero = 0; 
    temp_non_zero = -1;
    for (i = non_zero; i < non_zero + sizeof(crc)/sizeof(short); ++i) {
      seq[i] ^= crc[i - non_zero];
      if ((1 == seq[i]) && !new_non_zero) {
        temp_non_zero = i;
	new_non_zero = 1;
      }
    }
    
    if (-1 == temp_non_zero) {
      for (i = non_zero + sizeof(crc) / sizeof(short); i < size; ++i) {
        if (1 == seq[i]) {
          non_zero = i;
          break;
        }
      }
      if (i == size) return -1;
    } else {
      non_zero = temp_non_zero;
    }
  }

  return non_zero;
}

/*Here add fcs to one frame*/
static void add_fcs_to_frame(Frame *p_frame) {
  
  short seq[1024];
  int size = 0;
  int i;
  int non_zero = 0;

  convert_frame_to_seq(p_frame, seq, &size);
  
  seq_fcs(seq, size);

  for (i = 0; i < sizeof(crc)/sizeof(short) - 1; ++i) {
    p_frame ->fcs[i] = seq[size - (sizeof(crc)/sizeof(short) - 1) + i];
  }
}


static void convert_decimal_2_binary(int decimal, short binary[]) {
  
  binary[2] = decimal % 2;
  binary[0] = decimal / 4; 
  binary[1] = (decimal - binary[0] * 4) / 2;
}







/*Here create SABME, UA and DISC.*/
Frame create_unnumbered_frame(short addr[], enum FRAME_TYPE ft, bool p_f) {
  
  Frame f;
  short s1[] = {1, 1, 0, 1, 1, 0};/* For SABME*/
  short s2[] = {0, 0, 0, 1, 1, 0};/* For UA*/
  short s3[] = {0, 0, 0, 0, 1, 0};/* For DISC*/
  int i;

  create_generic_unnumbered_frame(&f);

  for (i = 0; i < 8; ++i) {
    f.address[i] = addr[i];
  }

  switch(ft) {
    case SABME : 
      for (i = 2; i < 8; ++i) {
        f.control[i] = s1[i - 2];
      }                   
    break;

    case UA : 
      for (i = 2; i < 8; ++i) {
        f.control[i] = s2[i - 2];
      }
    break;

    case DISC : 
      for (i = 2; i < 8; ++i) {
        f.control[i] = s3[i - 2];
      }
    break;
    default : fprintf(stderr, "Call create unnumbered frame error!\n"); 
   
    break;
  }

  f.control[4] = p_f;
  
  add_fcs_to_frame(&f);
  return f;
}

Frame create_sup_frame(short addr[], enum FRAME_TYPE ft, bool p_f, short want_receive) { 
  Frame f;
  int i;
  short next_receive[3];
  create_generic_sup_frame(&f);
  
  for (i = 0; i < 8; ++i) {
    f.address[i] = addr[i];
  }
   
  f.control[2] = 0;
  switch(ft) {
    case RR : 
      f.control[3] = 0;
    break;
    case REJ : 
      f.control[3] = 1;
    break;
    default : 
      fprintf(stderr, "create sup frame error!\n");
    break;
  }
  f.control[4] = p_f;


  convert_decimal_2_binary(want_receive, next_receive);
  for (i = 5; i < 8; ++i) {
    f.control[i] = next_receive[i - 5];
  }
 
  add_fcs_to_frame(&f);
  return f;
}



/* prequestic : ($end - $start) % $raw_data_length = 0*/
Frame* create_infor_frames(short addr[], short send_n, bool p_f, short receive_n, 
                          short data[], int start, int end, Frame *frames) { /*Normal data transferring.*/
  
  Frame f;
  int i;
  int j;
  int k;
  int ite = 0;
  short receive_number[3];
  short send_number[3];

  if ((end -start + 1) > (850 * window_length)) {
    fprintf(stderr, "One frame takes too long data!\n");
    return &f;
  }
  
  convert_decimal_2_binary(receive_n, receive_number);
  
  for (ite = 0; ite < ((end - start) / raw_data_length); ++ite) {
    
    create_generic_info_frame(&f);
    convert_decimal_2_binary(send_n, send_number);
    for (i = 1; i < 4; ++i) {

      f.control[i] = send_number[i - 1];
      f.control[i + 4] = receive_number[i - 1];
    }

    send_n = (send_n + 1) % frame_number_range;
    f.control[4] = p_f;

    j = 0;
    k = 0;

     
    /*bit stuffing!*/
    for (i = start + ite * raw_data_length; i < start + (ite + 1) * raw_data_length; ++i) {

     f.s.information[k] = data[i]; f.s.infor_type[k] = 1; ++k;  ++f.s.size;
     if (1 == data[i]) {
       if (4 == (i - j)) {
         f.s.information[k] = 0; f.s.infor_type[k] = 0; ++k;  ++f.s.size;
         j = i + 1;
       }        
     } else {
       j = i + 1;
     }
    }
   
    add_fcs_to_frame(&f);  
    frames[ite] = f;
  }
  return frames;
}

short unnumbered[][8] = {
          {1, 1, 1, 1, 0, 1, 1, 0}, /*SABME*/
          {1, 1, 0, 0, 0, 1, 1, 0}, /*UA*/
          {1, 1, 0, 0, 0, 0, 1, 0} /*DISC*/  };

enum FRAME_TYPE get_frame_type(Frame *p_frame) {
  
  if (0 == p_frame ->control[0]) {
    return INFOR; 
  }

  if (0 == p_frame ->control[1]) {
    if (0 == p_frame ->control[3]) return RR;
    else return REJ;
  } else {
    if (1 == p_frame ->control[2]) return SABME;
    else {
      if (1 == p_frame ->control[5]) return UA;
      else return DISC;
    }
  }
  
  fprintf(stdout, "Inteprete Frame type error!\n");
  return INFOR;
}

void get_address(Frame *p_frame, short *storage) {
  
  int i;
  for (i = 0; i < 8; ++i) {
    storage[i] = p_frame ->address[i];
  }
}

/* Only for INFOR, */
short get_send_number(Frame *p_frame) {
  
  int num;
  num = p_frame ->control[1] * 4 + p_frame ->control[2] * 2 + p_frame ->control[3];
  return num;
}
/* For INFOR, RR, REJ.*/
short get_expect_number(Frame *p_frame) {
  
  int num;
  num = p_frame ->control[5] * 4 + p_frame ->control[6] * 2 + p_frame ->control[7];
  return num;
}

/* Only for INFOR.*/
void get_infor(Frame *p_frame, short *storage, int *size) {
  int i;
  *size = 0;

  for (i = 0; i < p_frame ->s.size; ++i) {
    if (1 == p_frame ->s.infor_type[i]) {
      storage[*size] = p_frame ->s.information[i];
      ++(*size);
    }
  }
}





/*Here using fcs to judge whether the data is right.*/
bool is_fcs_right(Frame *p_frame) {

  short seq[1024];
  int size = 0;
  int non_zero;
  int i;

  convert_frame_to_seq(p_frame, seq, &size);

  non_zero = seq_fcs(seq, size);
  if (-1 == non_zero) {
    return 1;
  }
  
  return 0;
}



