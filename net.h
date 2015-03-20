/*
   This file declares net connections methods. 
   To simulate transferring data from one node to 
     another, I use socket technology.
   Author : Li Pan(lipan at hit dot edu dot cn)
   Date : 2012/12/7
 */

#ifndef _NET_H
#define _NET_H
/*
int hdlc_socket(char *err, int delay_seconds);
int hdlc_read(char *err, int socket_fd, int *buffer, int buffer_size);
int hdlc_write(char *err, int socket_fd, int buffer[], int buffer_size);
*/
/*-------------------for server------------------------------*/
/*
‌int hdlc_bind(char *err, int socket_fd, int port);
int hdlc_listen(char *err, int socket_fd, int block_length);
int hdlc_accept(char *err, int socket_fd);
*/
/*--------------------for client-----------------------------*/
/*int hdlc_client_connect(char *err, int socket_fd, int port, char *server_addr);*/

int hdlc_read(void *data, int entity_length, int count, int socket_fd);
int hdlc_write(void *data, int entity_length, int count, int socket_fd);

/*int hdlc_write(char *err, int socket_fd, int buffer[], int buffer_size);*/

/*---------------For server--------------------*/
int hdlc_accept(char *err, int socket_fd);
int hdlc_serve(int time_out, int port, int block_length);

/*-------------For client-------------------------*/
int hdlc_connect(int time_out, int port, char *server_addr);


void hdlc_close(int socket_fd);
#endif//_NET_H

