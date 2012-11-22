/*
 ============================================================================
 Name        : tp_udp_server.c
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

#define MYPORT "4950"    // the port users will be connecting to

#define MAXBUFLEN 1024

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

	int read_bytes = -1;
	int total_bytes = 0;
	int byte_counter = 0;
	char buffer[1000];
	time_t start;
	time_t last_counter;
	time_t now;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	time(&start);
	time(&last_counter);
	while (read_bytes != 0) {
		read_bytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		        (struct sockaddr *)&their_addr, &addr_len);
		if (read_bytes != -1) {
			perror("Recv: ");
			exit(-1);
		}
		total_bytes += read_bytes;
		byte_counter += read_bytes;
		if (byte_counter >= 262144) {
			byte_counter = 0;
			//Store the throughput for this 256KB
			time(&now);
			time_t difference = now - last_counter;
			time(&last_counter);
			printf("256KB transferred in %u seconds", difference);
		}
	}
	time(&now);
	time_t total_time = now - start;
	printf("Transferred %u bytes in %u seconds", total_bytes, total_time);

	close(sockfd);

	return 0;
}
