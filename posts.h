#ifndef TERMIN_ALL_POSTS_H
#define TERMIN_ALL_POSTS_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <time.h>
#include "sessions.h"
#include "connections.h"
#include "accounts.h"

int get_post_content(const char* input, char* content, char* restriction);
void handle_post(int client_fd, char* parameters);
void handle_viewposts(int client_fd, char* parameters);
void handle_delete_post(const int client_fd, char* parameters);
int check_view_permission(const char* viewer, const char* poster, int post_permission);
#endif //TERMIN_ALL_POSTS_H