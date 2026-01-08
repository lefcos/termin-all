#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

void send_command(int socket, char* command) {
    send(socket, command, strlen(command), 0);
}

int catch_response(int socket, char* buffer, int buffer_size) {
    memset(buffer, 0, buffer_size);
    int bytes_read = recv(socket, buffer, buffer_size, 0);
    return bytes_read;
}

void command_loop(int socket) {
    char command[1024];
    char response[1024];
    while (1) {
        if (fgets(command, 1024, stdin) == NULL) {
            break;
        }

        command[strlen(command) - 1] = '\0';
        if (strlen(command) == 0) {
            continue;
        }

        send_command(socket, command);

        if (strcmp(command, "exit") == 0) {
            catch_response(socket, response, 1024);
            printf("%s", response);
            break;
        }
        int bytes = catch_response(socket, response, 1024);
        if (bytes < 0) {
            printf("disconnected");
            break;
        }

        printf("%s", response);
        if (response[strlen(response) - 1] == '\n') {
            printf("\n");
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("ERROR: wrong number of arguments\n");
        return 1;
    }

    char* host = argv[1];
    int port = atoi(argv[2]);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("ERROR opening socket");
        return 1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    printf("connecting to server %s: %d\n", host, port);
    if (connect(sock, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("ERROR connecting");
        close(sock);
        return 1;
    }

    printf("connected to server %s:%d!\n", host, port);
    command_loop(sock);
    close(sock);
    printf("disconnected from server\n");
    return 0;
}