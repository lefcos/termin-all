#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "commands.h"

void handle_ping(int client_fd, char* parameters) {
    send(client_fd, "pong\n", 5, 0);
}

void handle_help(int client_fd, char* parameters) {
    const char* help_text =
        "available commands:\n"
        "> ping\n"
        "> help\n"
        "> login\n"
        "> signup\n"
        "> logout\n"
        "> viewprofile\n"
        "> setprofile\n"
        "> addfriend\n"
        "> post\n"
        "> viewposts\n"
        "> exit\n";

    send(client_fd, help_text, strlen(help_text), 0);
}

void handle_viewprofile(int client_fd, char* parameters) {
    send(client_fd, "viewprofile\n", 12, 0);
}

void handle_setprofile(int client_fd, char* parameters) {
    send(client_fd, "setprofile\n", 11, 0);
}

void handle_post(int client_fd, char* parameters) {
    send(client_fd, "post\n", 6, 0);
}

void handle_viewposts(int client_fd, char* parameters) {
    send(client_fd, "viewposts\n", 12, 0);
}

void handle_exit(int client_fd, char* parameters) {
    send(client_fd, "exiting...\n", 11, 0);
}

void handle_unknown(int client_fd, char* parameters) {
    send(client_fd, "unknown command\n", 16,0);
}