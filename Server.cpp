#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write

#include<pthread.h> //for threading , link with lpthread

#include<unordered_map>
#include<string>

#define MAXMSG 100

std::unordered_map<std::string, std::string> map;

void *connection_handler(void *);

int main(int argc , char *argv[])
{
    int socket_desc, new_socket, c , *new_sock;
    struct sockaddr_in server, client;
    char *message;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9088);

    //Bind
    if(bind(socket_desc,(struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("bind failed");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc, 3);

    //Accept an incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)))
    {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = (int*)malloc(sizeof(int));
        *new_sock = new_socket;

        if(pthread_create(&sniffer_thread, NULL,  connection_handler, (void*)new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join(sniffer_thread , NULL);
        puts("Handler assigned");
    }

    if (new_socket<0)
    {
        perror("accept failed");
        return 1;
    }

    pthread_exit(NULL);
    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char message[MAXMSG];
    char client_message[MAXMSG];


    //Receive a message from client
    while((read_size = recv(sock, client_message, MAXMSG, 0)) > 0)
    {
        //Send the message back to client
        std::string content(client_message, strlen(client_message));
        puts(content.c_str());

        std::string key(1, content[4]);
        std::string val(1, content[5]);

        if (content[0] == 'G') { // get
            if (map.find(key) != map.end()) {
                val = map[key];
                strcpy(message, val.c_str());
                write(sock , message , strlen(message));
            }
            else {
                strcpy(message, "Key error in Get()");
                write(sock , message , strlen(message));
            }
        }
        else { // put
            map[key] = val;
            strcpy(message, "Success in Put()");
            write(sock , message , strlen(message));
        }
    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    //Free the socket pointer
    free(socket_desc);

    return 0;
}
