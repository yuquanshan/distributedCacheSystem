#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "kv511.h"

#define PORT "9088"

void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_or_put(const char* buf){	// return 0 if get, 1 if put, -1 if N/A
	int i;
	char *getstr = "GET";
	char *putstr = "PUT";
	char box[4];
	box[3] = '\0';
	strncpy(box,buf,3);
	if(strcmp(box,getstr)==0){
		return 0;
	}else if(strcmp(box,putstr)==0){
		return 1;
	}
	return -1;
}

int main(int argc, char const *argv[]){
	char msg[80];
	char *invalid_cmd = "invalid command";
	fd_set master;
	fd_set read_fds;
	int fdmax;

	int listener;
	int newfd;
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;

	char buf[BUFSIZE];
	int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

	int yes = 1;
	int i, j, rv;

	struct addrinfo hints, *ai, *p;

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0){
		fprintf(stderr,"selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}
	for(p = ai; p != NULL; p = p->ai_next){	// find a suitable socket among those available socket types
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);	// why not specify socket address as well?
		if(listener < 0){
			continue;
		}
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		if(bind(listener, p->ai_addr, p->ai_addrlen)<0){
			close(listener);
			continue;
		}
		break;
	}
	if (p == NULL){
		fprintf(stderr,"selectserver: failed to bind\n");
		exit(2);
	}
	freeaddrinfo(ai);
	if(listen(listener, 10) == -1){
		printf("first descriptor: %d\n",listener);
		perror("listen");
		exit(3);
	}

	FD_SET(listener, &master);
	fdmax = listener;

	node_t *heads = initialize_hashtable();

	while(1){
		read_fds = master;
		if(select(fdmax+1,&read_fds,NULL,NULL,NULL) == -1){
			perror("select");
			exit(4);
		}
		for(i = 0; i <= fdmax; i++){
			memset(buf,'\0',BUFSIZE);
			if(FD_ISSET(i, &read_fds)){
				if(i == listener){	// new connection request
					addrlen = sizeof remoteaddr;
					newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
					if(newfd == -1){
						perror("accept");
					}else{
						FD_SET(newfd, &master);
						if(newfd > fdmax){
							fdmax = newfd;
						}
						printf("selectserver: new connection from %s on socket %d\n", inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN), newfd);
					}
				}else{
					if((nbytes = recv(i,buf,sizeof buf, 0)) <= 0){
						if(nbytes == 0){
							printf("selectserver: socket %d hung up\n",i);
						}else{
							perror("recv");
						}
						close(i);
						FD_CLR(i,&master);
					}else{
						/*if(send(i,buf,nbytes,0) == -1){
							perror("send");
						}*/
						int parse = get_or_put(buf);
						if(parse == 0){ 	// get
							k_t key = buf[4];
							node_t* res = get_node(key, heads);
							if(res == NULL){
								memset(msg,'\0',80);
								sprintf(msg,"value of key %c not found",key);
								if(send(i,msg,strlen(msg),0) == -1){
									perror("send");
								}
							}else{
								memset(msg,'\0',80);
								sprintf(msg,"value of key %c found: %c",key,res->val);
								if(send(i,msg,strlen(msg),0) == -1){
									perror("send");
								}
							}
						}else if(parse == 1){ //put
							put_node(buf[4],buf[5],heads);
							memset(msg,'\0',80);
							sprintf(msg,"<key,value> pair updated: <%c,%c>",buf[4],buf[5]);
							if(send(i,msg,strlen(msg),0) == -1){
								perror("send");
							}
						}else{
							if(send(i,invalid_cmd,15,0) == -1){
								perror("send");
							}
						}
					}
				}
			}
		}
	}

	return 0;
}