/*
   This file gives global variables proper values;
   Author : Li Pan
   Date : 2012/12/9
 */

/*Length of sliding window.*/
int window_length = 3;

/*Set read time out for socket.*/
int time_out = 4;

/*Every information frame has at most $raw_data_length effective characteristics. */
int raw_data_length = 700;

/* frame number range #*/
int frame_number_range = 8; /* 0 ~ 7*/

/* maximum data transferred*/
int maximum_data_transferred = 7000;


