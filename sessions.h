#ifndef TERMIN_ALL_SESSIONS_H
#define TERMIN_ALL_SESSIONS_H

void start_sessions();
void create_client_session(int socket_fd);
void end_client_session(int socket_fd);
void set_session_username(int socket_fd, const char *username);
void clear_session_username(int socket_fd);
const char *get_session_username(int socket_fd);
int is_session_logged_in(int socket_fd);
int get_socket_by_name(const char *username);
int is_user_logged_in(const char *username);
#endif //TERMIN_ALL_SESSIONS_H