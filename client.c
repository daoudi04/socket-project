#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void)
{
    int socket_client;

    socket_client = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_client == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3000);

    if (inet_pton(AF_INET,
                  "127.0.0.1",
                  &server_addr.sin_addr) != 1)
    {
        perror("inet_pton");
        close(socket_client);
        return EXIT_FAILURE;
    }

    if (connect(socket_client,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(socket_client);
        return EXIT_FAILURE;
    }

    printf("Connecté au serveur.\n");

    while (1)
    {
        char message[1024];

        printf("Client : ");

        if (fgets(message, sizeof(message), stdin) == NULL)
        {
            break;
        }

        ssize_t bytes_sent =
            send(socket_client,
                 message,
                 strlen(message),
                 0);

        if (bytes_sent == -1)
        {
            perror("send");
            break;
        }

        if (strcmp(message, "bye\n") == 0)
        {
            printf("Fermeture du client.\n");
            break;
        }

        char buffer[1024];

        ssize_t bytes_received =
            recv(socket_client,
                 buffer,
                 sizeof(buffer) - 1,
                 0);

        if (bytes_received == -1)
        {
            perror("recv");
            break;
        }

        if (bytes_received == 0)
        {
            printf("Le serveur a fermé la connexion.\n");
            break;
        }

        buffer[bytes_received] = '\0';

        printf("Serveur : %s", buffer);

        if (strcmp(buffer, "bye\n") == 0)
        {
            printf("Le serveur quitte.\n");
            break;
        }
    }

    close(socket_client);

    return EXIT_SUCCESS;
}