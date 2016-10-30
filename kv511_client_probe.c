#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include "kv511.h"

#define PORT "9088"
//#define BUFSIZE 100

char* char_pool = "1234567890!@#$^&*()abcdefghijklmnopqrstuvwsyzABCDEFGHIJKLMNOPQISTUVWXYZ,./;':<>?";

void *get_in_addr(struct sockaddr *sa){	// get IP address
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

v_t *get(k_t *key, int sockfd){
	char msg[BUFSIZE];
	char buf[BUFSIZE];
	v_t *res = malloc(1);
	int numbytes;
	memset(msg,'\0',80);
	sprintf(msg,"GET:%c",*key);
	if((numbytes = send(sockfd,msg,strlen(msg),0)) == -1){
		perror("send");
		exit(1);
	}
	if((numbytes = recv(sockfd,buf,BUFSIZE-1,0)) == -1){
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';
	printf("client: received '%s'\n", buf);
	*res = buf[numbytes-1];
	return res;
}

int put(k_t *key, v_t *val, int sockfd){
	char msg[BUFSIZE];
	char buf[BUFSIZE];
	int numbytes;
	memset(msg,'\0',80);
	sprintf(msg,"PUT:%c%c",*key,*val);
	if((numbytes = send(sockfd,msg,strlen(msg),0)) == -1){
		perror("send");
		exit(1);
	}
	if((numbytes = recv(sockfd,buf,BUFSIZE-1,0)) == -1){
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';
	printf("client: received '%s'\n", buf);
	return 0;
}

/**
*	argv[1]: dest ip addr; 
* 	argv[2]: number of client threads;
* 	argv[3]: session arrival rate for each of the threads (lambda)
* 	argv[4]: number of sessions for each thread
*	argv[5]: number of request per session
*/

int main(int argc, char const *argv[])
{	
	if (argc != 3){
		printf("usage: client_probe <server_ip> <cmd>\n");
	}
	k_t *key = malloc(1);
	v_t *val = malloc(1);
	int i,j,rv;
	int sockfd;
	char buf[BUFSIZE];
	char s[INET_ADDRSTRLEN];
	const char* addr = argv[1];
	struct addrinfo hints, *p, *servinfo;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if((rv = getaddrinfo(addr,PORT,&hints,&servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		pthread_exit(NULL);
	}
	for(p = servinfo; p != NULL; p = p->ai_next){	// pick first type of socket available
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("client: socket");
			continue;
		}
		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}
	if(p == NULL){
		fprintf(stderr, "client: failed to connect\n");
	}	
	inet_ntop(p->ai_family,get_in_addr((struct sockaddr *)p->ai_addr),s,sizeof s);
	printf("client: connecting to %s\n", s);
	freeaddrinfo(servinfo);
	
	if(get_or_put(argv[2])==1){	// PUT command	
		memcpy(key,argv[2]+4,1);
		memcpy(val,argv[2]+5,1);
		put(key,val,sockfd);
		printf("put request: put <%c,%c>\n", *key, *val);
	}else{				// GET command
		memcpy(key,argv[2]+4,1);
		printf("get request: get %c\n", *key);
		v_t *tmpval = get(key,sockfd);
	}
	
	close(sockfd);
	return 0;

}