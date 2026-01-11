#ifndef TERMIN_ALL_CLASSES_H
#define TERMIN_ALL_CLASSES_H
#include "accounts.h"

void handle_ping(int client_fd, char* parameters);
void handle_help(int client_fd, char* parameters);
void handle_viewprofile(int client_fd, char* parameters);
void handle_setprofile(int client_fd, char* parameters);
void handle_addfriend(int client_fd, char* parameters);
void handle_post(int client_fd, char* parameters);
void handle_viewposts(int client_fd, char* parameters);
void handle_exit(int client_fd, char* parameters);
void handle_unknown(int client_fd, char* parameters);

#endif //TERMIN_ALL_CLASSES_H