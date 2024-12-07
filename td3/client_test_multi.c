#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8000
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    // Création du socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Initialisation de la structure à zéro
    memset(&server_address, 0, sizeof(server_address));

    // Configuration du serveur
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        perror("Adresse invalide ou non supportée");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Connexion au serveur échouée");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connecté au serveur %s:%d\n", SERVER_IP, SERVER_PORT);

    // Communication en boucle
    while (1) {
        printf("Entrez un message : ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Supprimer le retour à la ligne

        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            perror("Erreur lors de l'envoi du message");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        ssize_t valread = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (valread == -1) {
            perror("Erreur lors de la réception de la réponse");
            break;
        } else if (valread == 0) {
            printf("Le serveur a fermé la connexion.\n");
            break;
        }

        printf("Réponse du serveur : %s\n", buffer);
    }

    close(sock);
    return 0;
}
