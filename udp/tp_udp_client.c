/*
 ============================================================================
 Name        : tp_udp_client.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "4950"    // the port users will be connecting to

int main(int argc, char *argv[])
{
    int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	char *buffer;
	unsigned long fileLen;
	FILE *file;
	size_t result;

	file = fopen("cat.jpg", "rb");
	if (file == NULL) {
		printf("error opening file");
		exit(-1);
	}

	// obtain file size:
	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
	rewind(file);

	//Allocate memory
	buffer = (char*) malloc(sizeof(char) * fileLen);
	if (buffer == NULL) {
		fputs("Memory error", stderr);
		exit(2);
	}

	result = fread(buffer, 1, fileLen, file);
	if (result != fileLen) {
		fputs("Reading error", stderr);
		exit(3);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo("127.0.0.1", SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    int total_written = 0;
        while (1) {

            void *t = buffer;
            while (total_written < fileLen) {
                int bytes_written = sendto(sockfd, t, fileLen - total_written, 0,
                        p->ai_addr, p->ai_addrlen);
                if (bytes_written <= 0) {
                    perror("Send:");
                    exit(-1);
                }
                total_written += bytes_written;
                t += bytes_written;
            }
            if (total_written >= fileLen)
            	break;
        }

        close(sockfd);

        return 0;
}
