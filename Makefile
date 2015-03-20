all : client.out server.out
client.out : client.o  hdlc_simulator.o frame.o net.o configure.o
	 gcc client.o  hdlc_simulator.o frame.o net.o configure.o -o client.out
client.o : client.c
	gcc -c client.c
server.out : server.o hdlc_simulator.o frame.o net.o  configure.o
	gcc server.o hdlc_simulator.o frame.o net.o  configure.o -o server.out
server.o : server.c
	gcc -c server.c

hdlc_simulator.o : hdlc_simulator.c
	gcc -c hdlc_simulator.c
frame.o : frame.c
	gcc -c frame.c
net.o : net.c
	gcc -c net.c
configure.o : configure.c
	gcc -c configure.c

