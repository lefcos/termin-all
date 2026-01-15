#ifndef TERMIN_ALL_CONNECTIONS_H
#define TERMIN_ALL_CONNECTIONS_H
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/file.h>
#include "sessions.h"

extern int username_exists(const char *username);
int connection_exists(const char *username1, const char *username2);
int connection_level(const char *username1, const char *username2);
void handle_addfriend(int client_fd, char* parameters);
void handle_removefriend(int client_fd, char* parameters);
#endif //TERMIN_ALL_CONNECTIONS_H