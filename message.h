#ifndef TERMIN_ALL_MESSAGE_H
#define TERMIN_ALL_MESSAGE_H

void handle_send_message(int client_fd, char* parameters);
void handle_inbox(int client_fd, char* parameters);

#endif