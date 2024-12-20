/**
 Handle multiple socket connections with select and fd set on Linux
 */

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD SET, FD ISSET, FD ZERO macros

#define TRUE 1
#define FALSE 0
#define PORT 8000

struct client_info
{
    char *pseudo;
    char *password;
    int socket;
};

int max_clients = 30, new_socket;
struct client_info clients[30];



int command(char *buffer, int socket)
{
    char *command = strtok(buffer, " ");
    if (strcmp(command, "/nickname") == 0)
    {
        char *pseudo = strtok(NULL, " ");
        char *password = strtok(NULL, " ");
        if (pseudo == NULL)
        {
            return 0; // Invalid pseudo
        }

        if (password == NULL)
        {
            // Change pseudo for non-registered user
            for (int i = 0; i < max_clients; i++)
            {
                if (clients[i].pseudo != NULL && strcmp(clients[i].pseudo, pseudo) == 0)
                {
                    char *error_message = "Pseudo already taken, please choose another one.";
                    send(socket, error_message, strlen(error_message), 0);
                    return 0; // pseudo already taken
                }
            }
            clients[socket].pseudo = strdup(pseudo);
            printf("pseudo %s assigned to socket %d\n", pseudo, socket);
            return 1; // pseudo successfully assigned
        }
        else
        {
            // Check if the pseudo is registered
            for (int i = 0; i < max_clients; i++)
            {
                if (clients[i].pseudo != NULL && strcmp(clients[i].pseudo, pseudo) == 0)
                {
                    if (strcmp(clients[i].password, password) == 0)
                    {
                        // Pseudo and password match, assign pseudo to the socket
                        clients[socket].pseudo = strdup(pseudo);
                        printf("pseudo %s assigned to socket %d\n", pseudo, socket);
                        return 1; // pseudo successfully assigned
                    }
                    else
                    {
                        return 0; // Incorrect password
                    }
                }
            }

            // If pseudo is not registered, force the user to change pseudo
            for (int i = 0; i < max_clients; i++)
            {
                if (clients[i].socket != 0 && clients[i].pseudo != NULL && strcmp(clients[i].pseudo, pseudo) == 0)
                {
                    char *error_message = "Pseudo already taken, please choose another one.";
                    send(socket, error_message, strlen(error_message), 0);
                    return 0; // pseudo already taken
                }
            }

            // Store the pseudo
            clients[socket].pseudo = strdup(pseudo);
            printf("pseudo %s assigned to socket %d\n", pseudo, socket);
            return 1; // pseudo successfully assigned
        }
    }
    if (strcmp(command, "/register") == 0)
    {
        char *pseudo = strtok(NULL, " ");
        char *password = strtok(NULL, " ");
        if (pseudo == NULL || password == NULL)
        {
            return 0; // Invalid pseudo or password
        }

        // Check if the pseudo is unique
        for (int i = 0; i < max_clients; i++)
        {
            if (clients[i].pseudo != NULL && strcmp(clients[i].pseudo, pseudo) == 0)
            {
                return 0; // pseudo already taken
            }
        }

        // Store the pseudo and password
        clients[socket].pseudo = strdup(pseudo);
        clients[socket].password = strdup(password);
        printf("pseudo %s with password assigned to socket %d\n", pseudo, socket);
        return 1; // pseudo successfully assigned
    }
    if (strcmp(command, "/unregister") == 0)
    {
        char *pseudo = strtok(NULL, " ");
        char *password = strtok(NULL, " ");
        if (pseudo == NULL || password == NULL)
        {
            return 0; // Invalid pseudo or password
        }

        // Check if the pseudo and password match
        for (int i = 0; i < max_clients; i++)
        {
            if (clients[i].pseudo != NULL && strcmp(clients[i].pseudo, pseudo) == 0)
            {
                if (strcmp(clients[i].password, password) == 0)
                {
                    // Pseudo and password match, unregister the pseudo
                    free(clients[i].pseudo);
                    free(clients[i].password);
                    clients[i].pseudo = NULL;
                    clients[i].password = NULL;
                    printf("pseudo %s unregistered from socket %d\n", pseudo, socket);
                    return 1; // pseudo successfully unregistered
                }
                else
                {
                    return 0; // Incorrect password
                }
            }
        }
        return 0; // pseudo not found
    }
    if (strcmp(command, "/mp") == 0)
    {
        char *target_pseudo = strtok(NULL, " ");
        char *message = strtok(NULL, "\0");
        if (target_pseudo == NULL || message == NULL)
        {
            return 0; // Invalid target pseudo or message
        }

        // Find the target user
        for (int i = 0; i < max_clients; i++)
        {
            if (clients[i].pseudo != NULL && strcmp(clients[i].pseudo, target_pseudo) == 0)
            {
                char private_message[1024];
                snprintf(private_message, sizeof(private_message), "private message from %s: %s", clients[socket].pseudo, message);
                send(clients[i].socket, private_message, strlen(private_message), 0);
                return 1; // Message successfully sent
            }
        }
        return 0; // Target pseudo not found
    }
    if (strcmp(command, "/alerte") == 0)
    {
        char *target_pseudo = strtok(NULL, " ");
        char *message = strtok(NULL, "\0");
        if (target_pseudo == NULL)
        {
            // Notify all users
            if (message == NULL)
            {
                return 0; // Invalid message
            }
            char alert_message[1024];
            snprintf(alert_message, sizeof(alert_message), "\a\033[1;31mALERT: %s\033[0m", message);
            for (int i = 0; i < max_clients; i++)
            {
                if (clients[i].socket != 0)
                {
                    send(clients[i].socket, alert_message, strlen(alert_message), 0);
                }
            }
            return 1; // Alert successfully sent to all users
        }
        else
        {
            // Notify specific user
            if (message == NULL)
            {
                return 0; // Invalid message
            }
            for (int i = 0; i < max_clients; i++)
            {
                if (clients[i].pseudo != NULL && strcmp(clients[i].pseudo, target_pseudo) == 0)
                {
                    char alert_message[1024];
                    snprintf(alert_message, sizeof(alert_message), "\a\033[1;31mALERT: %s\033[0m", message);
                    send(clients[i].socket, alert_message, strlen(alert_message), 0);
                    return 1; // Alert successfully sent to specific user
                }
            }
            return 0; // Target pseudo not found
        }
    }
    if (strcmp(command, "/send") == 0)
    {
        char *target_pseudo = strtok(NULL, " ");
        char *filename = strtok(NULL, " ");
        if (target_pseudo == NULL || filename == NULL)
        {
            return 0; // Invalid target pseudo or filename
        }

        // Find the target user
        int target_socket = -1;
        for (int i = 0; i < max_clients; i++)
        {
            if (clients[i].pseudo != NULL && strcmp(clients[i].pseudo, target_pseudo) == 0)
            {
                target_socket = clients[i].socket;
                break;
            }
        }

        if (target_socket == -1)
        {
            return 0; // Target pseudo not found
        }

        // Open the file
        FILE *file = fopen(filename, "r");
        if (file == NULL)
        {
            return 0; // File not found
        }

        // Read the file and send its content
        char file_buffer[1024];
        while (fgets(file_buffer, sizeof(file_buffer), file) != NULL)
        {
            send(target_socket, file_buffer, strlen(file_buffer), 0);
        }

        fclose(file);
        return 1; // File successfully sent
    }
    if (strcmp(command, "/red") == 0 || strcmp(command, "/blue") == 0 || strcmp(command, "/green") == 0)
    {
        char *message = strtok(NULL, "\0");
        if (message == NULL)
        {
            return 0; // Invalid message
        }

        char color_code[10];
        if (strcmp(command, "/red") == 0)
        {
            strcpy(color_code, "\033[1;31m");
        }
        else if (strcmp(command, "/blue") == 0)
        {
            strcpy(color_code, "\033[1;34m");
        }
        else if (strcmp(command, "/green") == 0)
        {
            strcpy(color_code, "\033[1;32m");
        }

        char colored_message[2048];
        snprintf(colored_message, sizeof(colored_message), "%s%s\033[0m", color_code, message);

        // Broadcast the colored message to all clients
        for (int i = 0; i < max_clients; i++)
        {
            if (clients[i].socket != 0)
            {
                send(clients[i].socket, colored_message, strlen(colored_message), 0);
            }
        }
        return 1; // Message successfully sent
    }
    if (strcmp(command, "/join") == 0)
    {
        char *channel = strtok(NULL, " ");
        if (channel == NULL)
        {
            return 0; // Invalid channel name
        }

        // Notify the user that they have joined the channel
        char join_message[1024];
        snprintf(join_message, sizeof(join_message), "You have joined the channel: %s", channel);
        send(socket, join_message, strlen(join_message), 0);

        // Notify all users in the channel about the new user
        char notify_message[1024];
        snprintf(notify_message, sizeof(notify_message), "%s has joined the channel: %s", clients[socket].pseudo, channel);
        for (int i = 0; i < max_clients; i++)
        {
            if (clients[i].socket != 0 && clients[i].socket != socket)
            {
                send(clients[i].socket, notify_message, strlen(notify_message), 0);
            }
        }
        return 1; // Successfully joined the channel
    }
    if (strcmp(command, "/join") == 0)
    {
        char *channel = strtok(NULL, " ");
        if (channel == NULL)
        {
            return 0; // Invalid channel name
        }

        // Check if the channel already exists
        int channel_exists = 0;
        for (int i = 0; i < max_clients; i++)
        {
            if (clients[i].pseudo != NULL && strcmp(clients[i].pseudo, channel) == 0)
            {
                channel_exists = 1;
                break;
            }
        }

        if (!channel_exists)
        {
            // Create the channel
            clients[socket].pseudo = strdup(channel);
            printf("Channel %s created by socket %d\n", channel, socket);
        }

        // Notify the user that they have joined the channel
        char join_message[1024];
        snprintf(join_message, sizeof(join_message), "You have joined the channel: %s", channel);
        send(socket, join_message, strlen(join_message), 0);

        // Notify all users in the channel about the new user
        char notify_message[1024];
        snprintf(notify_message, sizeof(notify_message), "%s has joined the channel: %s", clients[socket].pseudo, channel);
        for (int i = 0; i < max_clients; i++)
        {
            if (clients[i].socket != 0 && clients[i].socket != socket)
            {
                send(clients[i].socket, notify_message, strlen(notify_message), 0);
            }
        }
        return 1; // Successfully joined the channel
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int opt = TRUE;
    int master_socket, addrlen, new_socket, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[1025]; // data buffer of 1K

    // set of socket descriptors
    fd_set readfds;

    // a message
    char *message = "ECHO Daemon v1.0 \r\n";

    // initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        clients[i].socket = 0;
    }

    // create a master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set master socket to allow multiple connections , this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind the socket to localhost port 8000
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    // try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while (TRUE)
    {
        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        // add child sockets to set
        for (i = 0; i < max_clients; i++)
        {
            // socket descriptor
            sd = clients[i].socket;

            // if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        // wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        // If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // inform user of socket number − used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // send new connection greeting message
            if (send(new_socket, message, strlen(message), 0) != strlen(message))
            {
                perror("send");
            }
            puts("Welcome message sent successfully");

            // add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                // if position is empty
                if (clients[i].socket == 0)
                {
                    clients[i].socket = new_socket;
                    printf("Adding to list of sockets as %d\n", i);

                    break;
                }
            }
            // Ask for pseudo
            char pseudo[1024];
            memset(pseudo, 0, sizeof(pseudo));
            if (recv(new_socket, pseudo, sizeof(pseudo), 0) > 0)
            {
                printf("pseudo: %s\n", pseudo);
            }

            // Check if pseudo is already taken
            int pseudo_taken = 0;
            for (i = 0; i < max_clients; i++)
            {
                if (clients[i].socket != 0 && clients[i].pseudo != NULL && strcmp(pseudo, clients[i].pseudo) == 0)
                {
                    pseudo_taken = 1;
                    break;
                }
            }

            if (pseudo_taken)
            {
                char *error_message = "0";
                send(new_socket, error_message, strlen(error_message), 0);
                close(new_socket);
            }
            else
            {
                // Store the pseudo
                char *success_message = "1";
                send(new_socket, success_message, strlen(success_message), 0);
                clients[new_socket].pseudo = strdup(pseudo);
                printf("pseudo %s assigned to socket %d\n", pseudo, new_socket);
            }
        }

        // else its some IO operation on some other socket :)
        for (i = 0; i < max_clients; i++)
        {
            sd = clients[i].socket;

            if (FD_ISSET(sd, &readfds))
            {

                // Check if it was for closing , and also read the incoming message
                if ((valread = read(sd, buffer, 1024)) == 0)
                {
                    // Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    // Close the socket and mark as 0 in list for reuse
                    close(sd);
                    clients[i].socket = 0;
                }

                // Echo back the message that came in
                else
                {
                    // set the string terminating NULL byte on the end of the data read
                    buffer[valread] = '\0';
                    // Process the command
                    if (command(buffer, sd) == 0)
                    {
                        // If command failed, send an error message
                        char *error_message = "Invalid command or parameters.";
                        send(sd, error_message, strlen(error_message), 0);
                    }
                    for (i = 0; i < max_clients; i++)
                    {
                        if (clients[i].socket != sd)
                        {
                            send(clients[i].socket, buffer, strlen(buffer), 0);
                        }
                    }
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    return 0;
}