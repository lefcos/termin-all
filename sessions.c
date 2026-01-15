#include "sessions.h"

typedef struct {
    int socket_fd;
    char username[16];
    int logged_in; //0-nu 1-da
}clientSession;

clientSession clientSessions[1000];

void start_sessions() {
    for (int i = 0; i < 1000; i++) {
        clientSessions[i].socket_fd = -1;
        clientSessions[i].username[0] = '\0';
        clientSessions[i].logged_in = 0;
    }
}

void create_client_session(int socket_fd) {
    if (socket_fd >= 0 && socket_fd < 1000) {
        clientSessions[socket_fd].logged_in = 0;
        clientSessions[socket_fd].socket_fd = socket_fd;
        clientSessions[socket_fd].username[0] = '\0';
    }
}

void end_client_session(int socket_fd) {
    if (socket_fd >= 0 && socket_fd < 1000) {
        clientSessions[socket_fd].logged_in = 0;
        clientSessions[socket_fd].socket_fd = -1;
        clientSessions[socket_fd].username[0] = '\0';
    }
}

void set_session_username(int socket_fd, const char *username) {
    if (socket_fd >= 0 && socket_fd < 1000) {
        clientSessions[socket_fd].logged_in = 1;
        strncpy(clientSessions[socket_fd].username, username, sizeof(clientSessions[socket_fd].username));
        clientSessions[socket_fd].username[sizeof(clientSessions[socket_fd].username) - 1] = '\0';
    }
}

void clear_session_username(int socket_fd) {
    if (socket_fd >= 0 && socket_fd < 1000) {
        clientSessions[socket_fd].logged_in = 0;
        clientSessions[socket_fd].username[0] = '\0';
    }
}

const char *get_session_username(int socket_fd) {
    if (socket_fd >= 0 && socket_fd < 1000) {
        if (clientSessions[socket_fd].logged_in) {
            return clientSessions[socket_fd].username;
        }
    }
    return NULL;
}

int is_user_logged_in(const char *username) {
    for (int i = 0; i < 1000; i++) {
        if (clientSessions[i].socket_fd != -1 && strcmp(clientSessions[i].username, username) == 0 && clientSessions[i].logged_in == 1) {
            return 1;
        }
    }
    return 0;
}