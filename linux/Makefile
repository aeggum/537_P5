all: libmfs.so server client

libmfs.so:
	gcc -shared -fpic -g -Wall -Werror -o libmfs.so mfs.c lfs.c sendPacket.c listenServer.c udp.c

client: libmfs.so
	gcc -g -L. -lmfs -Wall -Werror -o client client.c

server: 
	gcc -Wall -Werror -o server server.c listenServer.c lfs.c udp.c

clean: 
	rm -rf libmfs.so client server *.o
