#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "commands.h"
#include "accounts.h"
#include "connections.h"
#include "sessions.h"
#include "message.h"
#include "posts.h"

//**command table**
typedef void (*commandHandler)(int client_fd, char* parameters);

typedef struct {
    char* name;
    commandHandler handler;
} commandEntry;

commandEntry commandTable[] = {
    {"ping", handle_ping},
    {"help", handle_help},
    {"login", handle_login},
    {"signup", handle_signup},
    {"logout", handle_logout},
    {"add_friend", handle_addfriend},
    {"remove_friend", handle_removefriend},
    {"exit", handle_exit},
    {"message", handle_send_message},
    {"inbox", handle_inbox},
    {"post", handle_post},
    {"view_posts", handle_viewposts},
    {"delete_post", handle_delete_post},

    {NULL, handle_unknown}
};

void receive_command(char* input, char* command, char* parameters) {
    char* space = strchr(input, ' ');

    if (space != NULL) {
        int length_cmd = space - input;
        if (length_cmd == 0) return;
        strncpy(command, input, length_cmd);
        command[length_cmd] = '\0';
        strcpy(parameters, space+1);
    }
    else {
        strcpy(command, input);
        parameters[0] = '\0';
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("ERROR: wrong number of arguments\n");
        return 1;
    }

    start_sessions();
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

    fd_set active_fds, read_fds;
    FD_ZERO(&active_fds);
    FD_SET(server_socket, &active_fds);
    int max_fd = server_socket;

    while (1) {
        read_fds = active_fds;

        if (select(max_fd + 1, &read_fds, NULL,NULL,NULL)<0) {
            perror("ERROR: select");
            break;
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            struct sockaddr_in client_address;
            socklen_t client_address_length = sizeof(client_address);

            int client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_length);

            if (client_socket >= 0) {
                FD_SET(client_socket, &active_fds);
                if (client_socket > max_fd)
                    max_fd = client_socket;
                create_client_session(client_socket);
                printf("Server: Client connected! fd = %d\n", client_socket);
            }
        }

        for (int i = 0; i <= max_fd; i++) {
            if (i != server_socket && FD_ISSET(i, &read_fds)) {
                char buffer[4000];
                char command[1024];
                char parameters[1024];

                memset(buffer, 0, sizeof(buffer));
                int bytes = recv(i, buffer, sizeof(buffer), 0);

                if (bytes <= 0) {
                    const char* username = get_session_username(i);
                    if (username != NULL) {
                        char usernamecopy[16];
                        strncpy(usernamecopy, username, 15);
                        usernamecopy[15] = '\0';

                        extern void change_user_status(const char* username, int status);
                        change_user_status(usernamecopy, 0);
                    }
                    printf("Server: Client disconnected. fd = %d\n", i);
                    end_client_session(i);
                    close(i);
                    FD_CLR(i, &active_fds);
                } else {
                    buffer[bytes] = '\0';
                    receive_command(buffer, command, parameters);

                    commandHandler handler = handle_unknown;
                    for (int j = 0; commandTable[j].name != NULL; j++) {
                        if (strcmp(command, commandTable[j].name) == 0) {
                            handler = commandTable[j].handler;
                            break;
                        }
                    }

                    handler(i, parameters);
                }
            }
        }
    }
    close(server_socket);
    return 0;
}