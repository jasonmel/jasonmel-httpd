CC = gcc

httpd: passivesock.o connectsock.o errexit.o
	$(CC) -o httpd passivesock.o connectsock.o errexit.o httpd.c

passivesock.o:
	$(CC) -c passivesock.c

connectsock.o:
	$(CC) -c connectsock.c

errexit.o:
	$(CC) -c errexit.c

install:
	cp httpd ../

clean:
	rm *.o
	rm httpd
