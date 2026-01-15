#include "connections.h"

int connection_exists(const char *username1, const char *username2) {
    FILE* file = fopen("database/connections.txt", "r");
    if (file == NULL) {
        return 0;
    }

    char line[1000];
    while (fgets(line, 1000, file) != NULL) {
        char entered_username1[16];
        char entered_username2[16];

        sscanf(line, "%[^/]/%[^/]", entered_username1, entered_username2);
        if (strcmp(username1, entered_username1) == 0 && strcmp(username2, entered_username2) == 0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

int connection_level(const char *username1, const char *username2) {
    FILE* file = fopen("database/connections.txt", "r");
    if (file == NULL) {
        return 0;
    }

    char line[1000];
    while (fgets(line, 1000, file) != NULL) {
        char entered_username1[100];
        char entered_username2[100];
        int level;

        sscanf(line,"%[^/]/%[^/]/%d", entered_username1, entered_username2, &level);
        if (strcmp(username1, entered_username1) == 0 && strcmp(username2, entered_username2) == 0) {
            fclose(file);
            return level;
        }
    }

    fclose(file);
    return 0;
}

void handle_addfriend(int client_fd, char* parameters) {
    FILE* file = fopen("database/connections.txt", "a");
    char connection_username[100];
    int level;
    char error_message[1000];
    const char* user = get_session_username(client_fd);
    int parsed = sscanf(parameters, "%s %d", connection_username, &level);

    if (user == NULL) {
        strcpy(error_message, "Error: you must be logged in.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (parsed != 2) {
        strcpy(error_message, "Error: wrong command format. Try: add_friend <username> <1/2/3>\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (level < 1 || level > 3) {
        strcpy(error_message, "Error: wrong level. Try: 1 - acquaintance; 2 - friend; 3 - close friend.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (!username_exists(connection_username)) {
        strcpy(error_message, "Error: username does not exist.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (strcmp(user, connection_username) == 0) {
        strcpy(error_message, "Error: you can't friend yourself.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (connection_exists(user, connection_username)) {
        strcpy(error_message, "Error: you are already connected to this user.\n");
        send(client_fd, error_message, strlen(error_message),0);
        return;
    }
    if (file == NULL) {
        strcpy(error_message, "Error: connection file doesn't exist.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    int fd = fileno(file);
    flock(fd, LOCK_EX);
    fprintf(file, "%s/%s/%d\n", user, connection_username, level);
    flock(fd, LOCK_UN);
    fclose(file);

    char response[256];
    char level_name[20];
    if (level == 1) strcpy(level_name, "connection");
    else if (level == 2)strcpy(level_name, "friend");
    else strcpy(level_name, "good friend");
    sprintf(response, "Success: %s added as a %s!\n", connection_username, level_name);
    send(client_fd, response, strlen(response), 0);
}

void handle_removefriend(int client_fd, char* parameters) {
    FILE* file = fopen("database/connections.txt", "r");
    char connection_username[100];
    char error_message[1000];
    const char* user = get_session_username(client_fd);
    int parsed = sscanf(parameters, "%s", connection_username);

    if (user == NULL) {
        strcpy(error_message, "Error: you must be logged in.");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (parsed != 1) {
        strcpy(error_message, "Error: wrong command format. Try: remove_friend <username>\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (!username_exists(connection_username)) {
        strcpy(error_message, "Error: username does not exist.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (file == NULL) {
        strcpy(error_message, "Error: connection file doesn't exist.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    char lines[100][1000];
    int line_cnt=0;
    int found = 0;
    while (fgets(lines[line_cnt], 1000, file) != NULL) {
        char entered_username1[100];
        char entered_username2[100];

        sscanf(lines[line_cnt], "%[^/]/%[^/]", entered_username1, entered_username2);
        if (strcmp(user, entered_username1) == 0 && strcmp(connection_username, entered_username2) == 0 || strcmp(connection_username,entered_username1) == 0 && strcmp(user, entered_username2) == 0) {
            found = 1;
            continue;
        }
        line_cnt++;
    }
    fclose(file);

    if (!found) {
        strcpy(error_message, "Error: connection does not exist.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    file = fopen("database/connections.txt", "w");
    if (file != NULL) {
        int fd = fileno(file);
        flock(fd, LOCK_EX);
        for (int i = 0; i < line_cnt; i++) {
            fprintf(file, "%s", lines[i]);
        }
        flock(fd, LOCK_UN);
        fclose(file);
    }

    char response[256];
    sprintf(response, "%s removed from friends.\n", connection_username);
    send(client_fd, response, strlen(response), 0);
}