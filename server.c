/* 
  This file is the main process of server.
  Author : Li Pan
  Date : 2012/12/8
  Version : 0.3
 */

#include "declares.h"
#include "net.h"
#include "frame.h"

#include <stdlib.h>
#include <stdio.h>

extern int time_out;

/*Set block length for server.*/
int block_length = 5;

short data[10000];
char error[256];

int main(int argc, char *argv[]) {
 
  int port;
  int socket_fd;
  int new_fd;
  
  Frame frame;
  int start_position = 0;
  int start_frame_number = 0;
  int wanted_frame_number = 0;
  short addr[] = {0, 0, 0, 0, 0, 0, 0, 1};
 
  int state;
  if (2 != argc) {
    fprintf(stderr, "Usage : %s port\n", argv[0]);
    exit(1);
  }
  if ((port = atoi(argv[1])) < 0) {
    fprintf(stderr, "Port should not be negative : %d\n", port);
    exit(1);
  }
  if (-1 == (socket_fd = hdlc_serve(time_out, port, block_length))) {
    fprintf(stderr, "Server can not provide service!\n");
    exit(1);
  }
   
   fprintf(stdout, "Waiting for connection.\n");
   /*Accept*/
   if (-1 == (new_fd = hdlc_accept(error, socket_fd))) {
     fprintf(stderr, error);
     exit(1);
   }    
   
   fprintf(stdout, "Connection established.\n");
   fill_data();

   /*Sending SABME frame*/
   send_SABME(addr, 1, new_fd);
   fprintf(stdout, "Sending SABME frame.\n");
   while (1) {
    
     state = simulate(addr, data, &start_position, &start_frame_number, new_fd, &wanted_frame_number);    
     if (1 == state) {
       break;
     }
   }

   hdlc_close(new_fd);
   fprintf(stdout, "Close new_fd\n");
   hdlc_close(socket_fd);
   fprintf(stdout, "Close socket_fd\n");
  return 1;

}

/* Fill data error.*/
void fill_data() {
  int i;
  for (i = 0; i < 10000; ++i) {
    data[i] = rand() % 2;
  }
}

void send_SABME(short addr[], bool p_f, int socket_fd) {
  
  Frame frame = create_unnumbered_frame(addr, SABME, 1);
  int i;

  hdlc_write((void *)(&frame), sizeof(Frame), 1, socket_fd);
}
