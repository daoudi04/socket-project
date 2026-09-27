#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

int main(void)
{
    int socket_serveur;

    socket_serveur = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_serveur == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    int opt = 1;

    if (setsockopt(socket_serveur,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &opt,
                   sizeof(opt)) == -1)
    {
        perror("setsockopt");
        close(socket_serveur);
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(3000);

    if (bind(socket_serveur,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(socket_serveur);
        return EXIT_FAILURE;
    }

    if (listen(socket_serveur, 5) == -1)
    {
        perror("listen");
        close(socket_serveur);
        return EXIT_FAILURE;
    }

    printf("Serveur en attente sur le port 3000...\n");

    int client_socket;

    client_socket = accept(socket_serveur, NULL, NULL);

    if (client_socket == -1)
    {
        perror("accept");
        close(socket_serveur);
        return EXIT_FAILURE;
    }

    printf("Client connecté !\n");

    while (1)
    {
        char buffer[1024];

        ssize_t bytes_received =
            recv(client_socket,
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
            printf("Le client a fermé la connexion.\n");
            break;
        }

        buffer[bytes_received] = '\0';

        printf("Client : %s", buffer);

        if (strcmp(buffer, "bye\n") == 0)
        {
            printf("Le client quitte.\n");
            break;
        }

        char message[1024];

        printf("Serveur : ");

        if (fgets(message, sizeof(message), stdin) == NULL)
        {
            break;
        }

        ssize_t bytes_sent =
            send(client_socket,
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
            printf("Fermeture du serveur.\n");
            break;
        }
    }

    close(client_socket);
    close(socket_serveur);

    return EXIT_SUCCESS;
}