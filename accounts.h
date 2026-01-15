#ifndef TERMIN_ALL_ACCOUNTS_H
#define TERMIN_ALL_ACCOUNTS_H
#include <stdbool.h>
#include "accounts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <stdbool.h>
#include "sessions.h"

void handle_signup(int client_fd, char* parameters);
void handle_login(int client_fd, char* parameters);
void handle_logout(int client_fd, char* parameters);
bool is_profile_private(const char* username);
void handle_set_privacy(int client_fd, char* parameters);
bool is_admin(const char* username);

#endif //TERMIN_ALL_ACCOUNTS_H