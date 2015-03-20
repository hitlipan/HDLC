/* 
   This file implements the methods declared in net.h
   Author : Li Pan
   Data : 2012/12/7
 */
#include "net.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

/*--------------For Client and Server methods.----------------*/
static void netError(char *err, const char *format, ...) {
  
  va_list ap;
  if (!err) {
    return;
  }
  va_start(ap, format);
  vsnprintf(err, 256, format, ap);
  va_end(ap);
}

static int hdlc_socket(char *err, int delay_seconds) {
  int socket_fd;
  struct timeval tv;

  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    netError(err, "Socket error : %s\n", strerror(errno));  
    return -1;
  } 
  tv.tv_sec = delay_seconds;
  tv.tv_usec = 0;  
  /*Here set socket option to check receive time out!*/
  setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  return socket_fd;
}

int hdlc_read(void *data, int entity_length, int count, int socket_fd) {
   
   int read_num;
   //FILE *p_file = fdopen(socket_fd, "r+");
   //read_num = fread(data, entity_length, count, p_file);
   //fclose(p_file); /* Close stream, also close socket_fd.*/
   read_num = read(socket_fd, data, entity_length * count);
   read_num /= entity_length;
   return read_num;
}

int hdlc_write(void *data, int entity_length, int count, int socket_fd) {
  //FILE *p_file = fdopen(socket_fd, "w+");
  int write_num;

  write_num = write(socket_fd, data, entity_length * count);
  //write_num = fwrite(data, entity_length, count, p_file); /*Here is wrong!--------------------------------*/
  write_num /= entity_length;
  //fclose(p_file);
  return write_num;
}

/*-----------------For Server-------------------*/
static int hdlc_bind(char *err, int socket_fd, int port) {
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    if (-1 == (bind(socket_fd, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr)))) {
      netError(err, "Bind error : %s\n", strerror(errno));
      return -1;
    }
    return 1;
}
 
static int hdlc_listen(char *err, int socket_fd, int block_length) {
  if (-1 == listen(socket_fd, block_length)) {
    
    netError(err, "Listen error : %s\n", strerror(errno));
    return -1;
  }
  return 1; /*Listen success!*/
}

/*-----------------For Client-------------------*/
static int hdlc_only_connect(char *err, int socket_fd, int port, char *server) {
  
  struct hostent *host;
  struct sockaddr_in server_addr;
  if (NULL == (host = gethostbyname(server))) {
    netError(err, "Get host by name error : %s\n", strerror(errno));
    return -1;
  }
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr = *((struct in_addr*)host -> h_addr);
  
  if (-1 == (connect(socket_fd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)))) {
    netError(err, "Client connect error : %s\n", strerror(errno));
    return -1;
  }

  return 1;/*Connect success!*/
}

/*Integrate the server methods above.*/
int hdlc_serve(int time_out, int port, int block_length) {
  char err[256] = {'\0'};
  int sock_fd;
  if (-1 == (sock_fd = hdlc_socket(err, time_out))) {
    fprintf(stderr, err);
    return -1;
  }
  if (-1 == hdlc_bind(err, sock_fd, port)) {
    fprintf(stderr, err);
    return -1;
  }
  
  if (-1 == hdlc_listen(err, sock_fd, block_length)) {
    fprintf(stderr, err);
    return -1;
  }
  return sock_fd;
}


int hdlc_accept(char *err, int socket_fd) {
  int new_fd;
  struct sockaddr_in client_addr;
  int sin_size;
  sin_size = sizeof(struct sockaddr_in);
  if (-1 == (new_fd = accept(socket_fd, (struct sockaddr *)(&client_addr), &sin_size))) {
    return -1;
  }
  fprintf(stdout, "Server gets connection from %s.\n", inet_ntoa(client_addr.sin_addr));
  return new_fd;;
}


/*Integrate the client methods above.*/
int hdlc_connect(int time_out, int port, char *server_addr) {
  
  char error[256];
  int socket_fd;
  if (-1 == (socket_fd = hdlc_socket(error, time_out))) {
    fprintf(stderr, error);
    return -1;
  }

  if (-1 == hdlc_only_connect(error, socket_fd, port, server_addr)) {
    fprintf(stderr, error);
    return -1;
  }
  return socket_fd;
}


void hdlc_close(int socket_fd) {
  close(socket_fd);
}

