#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>


#define SET_LI(packet, li) (uint8_t)(packet.li_vn_mode |= (li << 6))
#define SET_VN(packet, vn) (uint8_t)(packet.li_vn_mode |= (vn << 3))
#define SET_MODE(packet, mode) (uint8_t)(packet.li_vn_mode |= (mode<<0))
#define NTP_TIMESTAMP_DELTA 2208988800ull


void error( char* msg )
{
    perror( msg );
    exit( EXIT_FAILURE );
}

typedef struct
{

  uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                           // li.   Two bits.   Leap indicator.
                           // vn.   Three bits. Version number of the protocol.
                           // mode. Three bits. Client will pick mode 3 for client.

  uint8_t stratum;         // Eight bits. Stratum level of the local clock.
  uint8_t poll;            // Eight bits. Maximum interval between successive messages.
  uint8_t precision;       // Eight bits. Precision of the local clock.

  uint32_t rootDelay;      // 32 bits. Total round trip delay time.
  uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
  uint32_t refId;          // 32 bits. Reference clock identifier.

  uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
  uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

  uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
  uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

  uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
  uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

  uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
  uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;

int main(int argc, char* argv[]){
    ntp_packet packet;
    bzero(&packet, sizeof(ntp_packet));
    SET_LI(packet, 0);
    SET_VN(packet, 3);
    SET_MODE(packet, 3);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
    char* host_name = "fr.pool.ntp.org";
    if (argc == 2){
        printf("Using %s as NTP server\n", argv[1]);
        char* host_name = argv[1];
    }
    struct hostent* server = gethostbyname(host_name);
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(123);
    /*if (connect(sockfd, (struct sockaddr * ) &serv_addr, sizeof(serv_addr)) == -1){
        error("ERROR connecting");
    }*/
    if (sendto(sockfd, &packet, sizeof(ntp_packet), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1){
        error("ERROR writing to socket");
    }
    socklen_t size_addr;
    if (recvfrom(sockfd, &packet, sizeof(ntp_packet), 0, (struct sockaddr *) &serv_addr, size_addr) == -1){
        error("ERROR reading from socket");
    }
    printf("Time: %llu\n", htonl(packet.txTm_s)-NTP_TIMESTAMP_DELTA);
    printf("Date: %s", ctime((time_t*)(htonl(packet.txTm_s) - NTP_TIMESTAMP_DELTA)));
}