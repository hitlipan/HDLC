/* 
  This file implements client.
  Author : Li Pan
  Date : 2012/12/8
 */

#include "net.h"
#include "declares.h"

#include <stdio.h>
#include <stdlib.h>
extern time_out;

short data[10000];

int main(int argc, char *argv[]) {
  
  int socket_fd;
  int port;
  char *host;

  Frame frame;
  int start_position = 0;
  int start_frame_number = 0;
  int wanted_frame_number = 0;
  short addr[] = {0, 0, 0, 0, 0, 0, 0, 1};
  
  int state;
  if (3 != argc) {
    fprintf(stderr, "Usage : %s hostname port\n", argv[0]);
    exit(1);
  }
  if ((port = atoi(argv[2])) < 0) {
    fprintf(stderr, "Port error\n");
    exit(1);
  }
  host = argv[1];
  if (-1 == (socket_fd = hdlc_connect(time_out, port, host))) {
    fprintf(stderr, "Client connect error!\n");
    exit(1);
  }
  fprintf(stdout, "Connection established\n");  
  fill_data();

  while(1) {
    /*simulate();*/
    state = simulate(addr, data, &start_position, &start_frame_number, socket_fd, &wanted_frame_number); 
    if (1 == state) {
      break;
    }
  }
  hdlc_close(socket_fd);
  fprintf(stdout, "Close socket_fd\n");

  return 1;
}


/* Fill data*/
void fill_data() {
  int i;
  for (i = 0; i < 10000; ++i) {
    data[i] = rand() % 2;
  }
}

