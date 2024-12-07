#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>

void error( char* msg )
{
    perror( msg );
    exit( EXIT_FAILURE );
}

int sockfd;

int main() {
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("socket creation failed");
		return 1;
	}
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(1234);
    int bind_result = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    char* data = calloc(204, sizeof(char));
    socklen_t size_addr;
    while(1){
    recvfrom(sockfd, data, 2048, MSG_WAITALL, (struct sockaddr *)&server_addr, &size_addr);
    printf("%s\n",data);
    printf("server : %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    sleep(1);
    sendto(sockfd, "PONG", sizeof("PONG"), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
    }
    return EXIT_SUCCESS;
}