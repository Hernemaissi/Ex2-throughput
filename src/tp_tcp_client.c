

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to
#define IP "127.0.0.1"

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Wrong number of arguments\n");
		exit(-1);
	}
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char * buffer;
    unsigned long fileLen;
    FILE *file;
    size_t result;

    file = fopen("time_results_micro.txt", "rb");
    if (file == NULL) {
    	printf("error opening file");
    	exit(-1);
    }

    // obtain file size:
     fseek (file , 0 , SEEK_END);
     fileLen = ftell (file);
     rewind (file);

    //Allocate memory
    buffer = (char*) malloc (sizeof(char)*fileLen);
    if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

    result = fread (buffer,1,fileLen,file);
    if (result != fileLen) {fputs ("Reading error",stderr); exit (3);}


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    int total_written = 0;
    while (1) {

        void *p = buffer;
        while (total_written < fileLen) {
            int bytes_written = send(sockfd, p, fileLen - total_written, 0);
            if (bytes_written <= 0) {
                perror("Send:");
                exit(-1);
            }
            total_written += bytes_written;
            p += bytes_written;
        }
        if (total_written >= fileLen)
        	break;
    }

    close(sockfd);
    free(buffer);
    return 0;
}
