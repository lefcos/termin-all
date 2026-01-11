#include "accounts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>

typedef struct {
    char username[16];
    char password[20];
    char type[10];  // user/admin
    int loggedin; // 0=offline / 1=online
}User;

int username_exists(const char* username) {
    FILE* file = fopen("users.txt", "r");
    if (file == NULL) {
        return 0;
    }

    char line[1000];
    while (fgets(line, 1000, file) != NULL) {
        char stored_username[100];
        sscanf(line, "%[^/]", stored_username); //extrage usernameul din linie pana la primul |

        if (strcmp(stored_username, username) == 0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

int credentials_check(const char* username, const char* password, char* error_msg) {

    if (strlen(username) < 3 || strlen(username) > 15) {
        strcpy(error_msg, "Error: username must be between 3 and 15 characters.\n");
        return 0;
    }

    if (strchr(username, '/') != NULL) {
        strcpy(error_msg, "Error: username cannot contain '/'\n");
        return 0;
    }

    if (strlen(password) < 3 || strlen(password) > 19) {
        strcpy(error_msg, "Error: password must be between 3 and 19 characters.\n");
        return 0;
    }

    if (strchr(password, '/') != NULL) {
        strcpy(error_msg, "Error: password cannot contain '/'.\n");
        return 0;
    }

    return 1;
}

void handle_signup(int client_fd, char* parameters) {
    char username[16];
    char password[20];
    char error_message[256];

    int parsed = sscanf(parameters, "%s %s", username, password);
    if (parsed != 2) {
        strcpy(error_message, "Error: wrong command format. Try: signup <username> <password>\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    if (!credentials_check(username, password, error_message)) {
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    if (username_exists(username)) {
        strcpy(error_message, "Erro: Username taken, try again.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    FILE* file = fopen("users.txt", "a");
    if (file == NULL) {
        strcpy(error_message, "Error: 'users.txt' not found.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    int fd = fileno(file);
    flock(fd, LOCK_EX);
    fprintf(file, "%s/%s/user/0\n", username, password);
    flock(fd, LOCK_UN);
    fclose(file);

    char response[256];
    sprintf(response, "Account created. Welcome, %s!\n", username);
    send(client_fd, response, strlen(response), 0);
    printf("Server: user '%s' signed up\n", username);
}

int check_login(const char* username, const char* password) {
    FILE* file = fopen("users.txt", "r");

    if (file == NULL) return 0;
    char line[1000];

    while (fgets(line, sizeof(line), file)) {
        char entered_username[100];
        char entered_password[100];

        sscanf(line, "%[^/]/%[^/]", entered_username, entered_password);

        if (strcmp(entered_username, username) == 0) {
            if (strcmp(entered_password, password) == 0) {
                fclose(file);
                return 1;
            } else {
                fclose(file);
                return 0;
            }
        }
    }
    fclose(file);
    return 0;
}

void change_user_status(const char* username, int status) {
    FILE* file = fopen("users.txt", "r");
    if (file == NULL) {
        return;
    }

    char lines[100][1000];
    int line_cnt = 0;
    while (fgets(lines[line_cnt], 1000, file) && line_cnt < 100) {
        line_cnt++;
    }
    fclose(file);

    for (int i = 0; i < line_cnt; i++) {
        char entered_username[100];
        char entered_password[100];
        char entered_type[10];

        sscanf(lines[i], "%[^/]/%[^/]/%[^/]", entered_username, entered_password, entered_type);
        if (strcmp(entered_username, username) == 0) {
            if (status == 1)
                sprintf(lines[i], "%s/%s/%s/1\n", entered_username, entered_password, entered_type);
            else if (status == 0)
                sprintf(lines[i], "%s/%s/%s/0\n", entered_username, entered_password, entered_type);
            break;
        }
    }

    file = fopen("users.txt", "w");
    if (file != NULL) {
        int fd = fileno(file);
        flock(fd, LOCK_EX);

        for (int i = 0; i < line_cnt; i++) {
            fprintf(file, "%s", lines[i]);
        }

        flock(fd, LOCK_UN);
        fclose(file);
    }
}

int check_user_status(const char* username) {
    FILE* file = fopen("users.txt", "r");
    if (file == NULL) {
        return 0;
    }

    char line[1000];

    while (fgets(line, sizeof(line), file)) {
        char entered_username[100];
        char entered_password[100];
        char entered_type[10];
        int loggedin;

        sscanf(line, "%[^/]/%[^/]/%[^/]/%d", entered_username, entered_password, entered_type, &loggedin);

        if (strcmp(entered_username, username) == 0) {
            fclose(file);
            return loggedin;
        }
    }

    fclose(file);
    return 0;
}

void handle_login(int client_fd, char* parameters) {
    char username[16];
    char password[20];
    char error_message[256];

    int parsed = sscanf(parameters, "%s %s", username, password);
    if (parsed != 2) {
        strcpy(error_message, "Error: wrong command format. Try: signup <username> <password>\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    if (check_login(username, password) != 1) {
        strcpy(error_message, "Error: Invalid username or password.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    if (check_user_status(username)) {
        strcpy(error_message, "Error: User is already logged in.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    change_user_status(username, 1);

    char response[256];
    sprintf(response, "Account logged in. Welcome, %s!\n", username);
    send(client_fd, response, strlen(response), 0);
    printf("Server: user %s logged in\n", username);
}

void handle_logout(int client_fd, char* parameters) {
    char username[16];
    char error_message[256];

    int parsed = sscanf(parameters, "%s", username);
    if (parsed != 1) {
        strcpy(error_message, "Eror: wrong command format. Try: logout <username>\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    if (!username_exists(username)) {
        strcpy(error_message, "Error: Username does not exist.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    if (check_user_status(username) == 0) {
        strcpy(error_message, "Error: User is not logged in.\n");
        send (client_fd, error_message, strlen(error_message), 0);
        return;
    }

    change_user_status(username, 0);
    char response[256];
    sprintf(response, "Account logged out. Goodbye, %s!\n", username);
    send(client_fd, response, strlen(response), 0);
    printf("Server: user '%s' logged out\n", username);
}