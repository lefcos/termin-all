#ifndef TERMIN_ALL_MESSAGE_H
#define TERMIN_ALL_MESSAGE_H
#include "posts.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <time.h>
#include "sessions.h"

void handle_send_message(int client_fd, char* parameters);
void handle_inbox(int client_fd, char* parameters);

#endif