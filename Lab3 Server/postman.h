#ifndef POSTMAN_H
#define POSTMAN_H

typedef struct Mail {
	char* msg;
} Mail;
int readThreadFunc();
int writeThreadFunc();

#endif