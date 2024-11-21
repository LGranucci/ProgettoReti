#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 8080

void handle_client_message(int client_sock, fd_set *active_fds) {
    char buffer[BUFFER_SIZE];
    int bytes_read = recv(client_sock, buffer, sizeof(buffer), 0);
    
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            printf("Client %d disconnected.\n", client_sock);
        } else {
            perror("recv");
        }
        close(client_sock);
        FD_CLR(client_sock, active_fds);
    } else {
        buffer[bytes_read] = '\0';
        printf("Client %d sent: %s\n", client_sock, buffer);
        // Simulazione risposta
        char response[] = "Message received.\n";
        send(client_sock, response, strlen(response), 0);
    }
}

int main() {
    int server_sock, client_sock, max_sd, activity, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    fd_set active_fds, read_fds;

    // Creazione socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Binding
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 3) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Inizializzazione del set di file descriptor
    FD_ZERO(&active_fds);
    FD_SET(server_sock, &active_fds);
    max_sd = server_sock;

    while (1) {
        read_fds = active_fds;

        // Uso di select per gestire I/O multiplexing
        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("Select error");
        }

        // Verifica nuove connessioni
        if (FD_ISSET(server_sock, &read_fds)) {
            if ((new_socket = accept(server_sock, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("Accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, ip is: %s, port: %d\n",
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Aggiungere il nuovo socket al set
            FD_SET(new_socket, &active_fds);
            if (new_socket > max_sd) {
                max_sd = new_socket;
            }
        }

        // Gestione di tutti i client connessi
        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i != server_sock) {
                    handle_client_message(i, &active_fds);
                }
            }
        }
    }

    return 0;
}
