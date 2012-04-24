all: libmfs.so server

libmfs.so:
	gcc -shared -fpic -g -o libmfs.so mfs.c udp.c

client: libmfs.so
	gcc -g -L. -lmfs -o client client.c

server: 
	gcc -o server server.c lfs.c udp.c

clean: 
	rm -rf libmfs.so client server
