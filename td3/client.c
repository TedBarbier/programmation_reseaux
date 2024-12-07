#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT 8000
#define BUFFER_SIZE 1024

int main() {
    int sockfd, numbytes;
    char buf[BUFFER_SIZE];
    struct hostent *he;
    struct sockaddr_in server;

    // Résolution de l'adresse IP
    if ((he = gethostbyname("127.0.0.1")) == NULL) { 
        perror("gethostbyname");
        exit(1);
    }

    // Création de la socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // Configuration de l'adresse du serveur
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(server.sin_zero), '\0', 8);

    // Connexion au serveur
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("connect");
        exit(1);
    }

    // Ouverture du fichier pour écrire les données reçues
    FILE *fp = fopen("received_file.txt", "w");
    if (fp == NULL) {
        perror("fopen");
        close(sockfd);
        exit(1);
    }

    // Réception des données
    while ((numbytes = recv(sockfd, buf, BUFFER_SIZE, 0)) > 0) {
        fwrite(buf, 1, numbytes, fp);
    }

    // Gestion des erreurs de réception
    if (numbytes < 0) {
        perror("recv");
        fclose(fp);
        close(sockfd);
        exit(1);
    }

    // Assurez-vous que le tampon du fichier est bien vidé
    fflush(fp);

    // Fermeture des ressources
    fclose(fp);
    close(sockfd);

    printf("Fichier reçu et sauvegardé avec succès dans 'received_file.txt'.\n");

    return 0;
}
