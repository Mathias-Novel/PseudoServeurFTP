CC = gcc
FLAGS = -W -Wall -g -pthread

all : serveur client

serveur : ServeurFTP.c ConstanteFTP.c csapp.c readcmd.c
	$(CC) $(FLAGS) -o $@ $^

client : ClientFTP.c ConstanteFTP.c csapp.c readcmd.c
	$(CC) $(FLAGS) -o $@ $^

clean :
	rm -rf *.o serveur client
