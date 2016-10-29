#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write

#include<pthread.h> //for threading , link with lpthread

#include "kv511.h"

#include<mutex>
#include<shared_mutex>

#define PORT_LISTEN 9088

std::mutex mutex;
node_t *heads = initialize_hashtable();

void *pthread_function(void *);

int get_or_put(const char* buf){	// return 0 if get, 1 if put, -1 if N/A
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

int main(int argc , char *argv[])
{
    int socket_descriptor, new_socket, c, *new_socket_thread;
    struct sockaddr_in server, client;

    //Create socket
    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor == -1){
        printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_LISTEN);

    //Bind
    if(bind(socket_descriptor,(struct sockaddr *)&server, sizeof(server)) < 0){
        printf("port binding failed\n");
        return 1;
    }
    printf("port binding done\n");

    //Listen
    listen(socket_descriptor, 3);

    //Accept an incoming connection
    c = sizeof(struct sockaddr_in);
    while((new_socket = accept(socket_descriptor, (struct sockaddr *)&client, (socklen_t*)&c))){
        printf("client connected\n");

        pthread_t sniffer_thread;
        new_socket_thread = (int*)malloc(sizeof(int));
        *new_socket_thread = new_socket;

        if(pthread_create(&sniffer_thread, NULL,  pthread_function, (void*)new_socket_thread) < 0){
            perror("could not create thread");
            return 1;
        }
    }

    if (new_socket<0){
        perror("accept failed");
        return 1;
    }

    pthread_exit(NULL);
    return 0;
}

void *pthread_function(void *socket_descriptor)
{
    //Get the socket descriptor
    int sock = *(int*)socket_descriptor;
    int read_size;
    char msg[BUFSIZE];
    char buf[BUFSIZE];

    int parse;
    k_t key;
    node_t* res;

    //Receive a msg from client
    while((read_size = recv(sock, buf, BUFSIZE, 0)) > 0)
    {
        parse = get_or_put(buf);
        if(parse == 0){ 	// get
            key = buf[4];
            mutex.lock();
            res = get_node(key, heads);
            mutex.unlock();
            if(res == NULL){
                memset(msg,'\0',80);
                sprintf(msg,"value of key %c not found",key);
                mutex.lock();
                write(sock, msg, strlen(msg));
                mutex.unlock();
            }
            else{
                memset(msg,'\0',80);
                sprintf(msg,"value of key %c found: %c",key,res->val);
                mutex.lock();
                write(sock, msg, strlen(msg));
                mutex.unlock();
            }
        }
        else if(parse == 1){ //put
            mutex.lock();
            put_node(buf[4],buf[5],heads);
            mutex.unlock();
            memset(msg,'\0',80);
            sprintf(msg,"<key,value> pair updated: <%c,%c>",buf[4],buf[5]);
            mutex.lock();
            write(sock, msg, strlen(msg));
            mutex.unlock();
        }
        else{
            memset(msg,'\0',80);
            sprintf(msg,"invalid command! <%c,%c>",buf[4],buf[5]);
            mutex.lock();
            write(sock, msg, strlen(msg));
            mutex.unlock();
        }
    }

    if(read_size == 0){
        printf("client disconnected\n");
        fflush(stdout);
    }
    else if(read_size == -1){
        perror("recv failed");
    }

    free(socket_descriptor);
    return 0;
}
