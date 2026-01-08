#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

void command_catch(char* input, char* command, char* parameters) {
    char* space = strchr(input, ' ');

    if (space != NULL) {
        int length_cmd = space - input;
        if (length_cmd == 0) return;
        strncpy(command, input, length_cmd);
        command[length_cmd] = '\0';
    }
    else {
        strcpy(command, input);
    }
}

void send_response(int client_socket, const char* response) {
    send(client_socket, response, strlen(response), 0);
}

void handle_client(int client_socket) {
    char buffer[1024];
    char command[1024];
    char parameters[1024];
    printf("Server: connected!");

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received == 0) {
            printf("Server: disconnected.");
            break;
        }

        buffer[strcspn(buffer, "\n")] = '\0';
        command_catch(buffer, command, parameters);

        if (strcmp(command, "ping") == 0) {
            send_response(client_socket, "pong\n");
        } else {
            send_response(client_socket, "wut\n");
        }
    }
    close(client_socket);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("ERROR: wrong number of arguments\n");
        return 1;
    }

    int port = atoi(argv[1]);
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("eroare la socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("ERROR: bind");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) < 0) {
        perror("ERROR: listen");
        close(server_socket);
        return 1;
    }
    printf("Server: started on port %d, waiting for connections.\n", port);

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);

        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_length);
        if (client_socket < 0) {
            perror("ERROR: accept");
            continue;
        }
        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}