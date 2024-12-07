#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define MAX 80
#define PORT 1234
#define BUF_SIZE 512
#define SA struct sockaddr

# define NTP_TIMESTAMP_DELTA 2208988800ull
#define SET_LI(packet,li) (uint8_t) (packet.li_vn_mode|=(li<<6))
#define SET_VN(packet,vn) (uint8_t) (packet.li_vn_mode|=(vn<<3))
#define SET_MODE(packet,mode) (uint8_t) (packet.li_vn_mode|=(mode<<0)) 

extern int h_errno;

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

} ntp_packet;              // Total: 384 bits or 48 bytes.


void requete_ntp(){
     ntp_packet packet;

    bzero(&packet,sizeof(ntp_packet));

    SET_LI(packet,0);
    SET_VN(packet,3);
    SET_MODE(packet,3);
    int sockntp=socket(AF_INET,SOCK_DGRAM,0);
    if(sockntp==-1){
        perror("pas de descripteur");
    }
    char* host_name="fr.pool.ntp.org";
    struct hostent *server=gethostbyname(host_name);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr,sizeof(struct sockaddr_in));

    serv_addr.sin_family=AF_INET;

    bcopy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    int portno=123;
    serv_addr.sin_port=htons(portno);
    printf("ok\n");
    int e=connect(sockntp,(struct sockaddr*) &serv_addr,sizeof(serv_addr));
    if(e<0){
        perror("pas de connection\n");
    }
    e=write(sockntp,&packet,sizeof(ntp_packet));
    if(e<0){
        perror("pas de write\n");
    }
    e=read(sockntp,&packet,sizeof(ntp_packet));
    if(e<0){
        perror("read\n");
    }
    printf("Timestamp: %u\n",ntohl(packet.txTm_s));
    close(sockntp);

    

}

int main(){
    int sockfd;
    struct sockaddr_in servaddr ;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
    char * pseudo;
    char buffer[1];
    *buffer=0;
    while (buffer==0){
        printf("Enter pseudo:");
        scanf("%s",pseudo);
        send(sockfd,pseudo,BUF_SIZE,0);
        recv(sockfd,buffer,1,0);

    }
    
    
    char buff[BUF_SIZE];
    int n;
    while (1){
        bzero(buff, sizeof(buff));
        printf("Enter the string:");
        n = 0;
        while ((buff[n++] = getchar()) != '\n');
        requete_ntp();
        send(sockfd, buff, sizeof(buff), 0);
        bzero(buff, sizeof(buff));
        recv(sockfd, buff, sizeof(buff), 0);
        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }

    }


   

}