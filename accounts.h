#ifndef TERMIN_ALL_ACCOUNTS_H
#define TERMIN_ALL_ACCOUNTS_H

void handle_signup(int client_fd, char* parameters);
void handle_login(int client_fd, char* parameters);
void handle_logout(int client_fd, char* parameters);

#endif //TERMIN_ALL_ACCOUNTS_H