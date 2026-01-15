#include "accounts.h"

typedef struct {
    char username[16];
    char password[20];
    bool user_type;  // 0=user 1=admin
    bool loggedin; // 0=offline 1=online
}User;

int parse_line(char* line, char* tokens[]) {
    int i = 0;
    char* token = strtok(line, "/\n");

    while (token != NULL && i < 10) {
        tokens[i++] = token;
        token = strtok(NULL, "/\n");
    }
    return i;
}

int username_exists(const char* username) {
    FILE* file = fopen("database/users.txt", "r");
    if (file == NULL) {
        return 0;
    }

    char line[1000];
    while (fgets(line, 1000, file) != NULL) {
        char stored_username[100];
        sscanf(line, "%[^/]", stored_username);

        if (strcmp(stored_username, username) == 0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

int valid_signup(const char* username, const char* password, char* error_msg) {
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
    FILE* file = fopen("database/users.txt", "a");
    char username[16], password[20], admin_keyword[10], error_message[256];
    int user_type = 0;
    int parsed = sscanf(parameters, "%s %s %s", username, password, admin_keyword);

    if (parsed < 2) {
        strcpy(error_message, "Error: wrong command format. Try: signup <username> <password>\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (!valid_signup(username, password, error_message)) {
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (username_exists(username)) {
        strcpy(error_message, "Erro: Username taken, try again.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (file == NULL) {
        strcpy(error_message, "Error: 'database/users.txt' not found.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (parsed == 3 && strcmp("admin", admin_keyword) == 0) {
        user_type = 1;
    }

    int fd = fileno(file);
    flock(fd, LOCK_EX);
    fprintf(file, "%s/%s/%d/0\n", username, password, user_type);
    flock(fd, LOCK_UN);
    fclose(file);

    char response[256];
    sprintf(response, "Account created. Welcome to termin-all, %s!\n", username);
    send(client_fd, response, strlen(response), 0);
}

int check_login(const char* username, const char* password) {
    FILE* file = fopen("database/users.txt", "r");
    if (file == NULL) return 0;

    char line[1000];
    while (fgets(line, sizeof(line), file)) {
        char entered_username[16];
        char entered_password[20];

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
    FILE* file = fopen("database/users.txt", "r");
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
        char entered_username[16];
        char entered_password[20];
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

    file = fopen("database/users.txt", "w");
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

void handle_login(int client_fd, char* parameters) {
    char username[16], password[20], error_message[256];
    const char* user = get_session_username(client_fd);
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
    if (is_user_logged_in(username)) {
        strcpy(error_message, "Error: user is already logged in.\n");
        send(client_fd, error_message, strlen(error_message),0);
        return;
    }
    if (user != NULL) {
        sprintf(error_message, "Error: you are already logged in.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    set_session_username(client_fd, username);
    change_user_status(username, 1);

    char response[256];
    sprintf(response, "Logged in. Welcome, %s!\n", username);
    send(client_fd, response, strlen(response), 0);
}

void handle_logout(int client_fd, char* parameters) {
    const char* username = get_session_username(client_fd);
    char error_message[256];

    if (username == NULL) {
        strcpy(error_message, "Error: you are not logged in.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    char usernamecopy[16];
    strncpy(usernamecopy, username, sizeof(usernamecopy)-1);
    usernamecopy[sizeof(usernamecopy)-1] = '\0';

    clear_session_username(client_fd);
    change_user_status(usernamecopy, 0);

    char response[256];
    sprintf(response, "Account logged out. Goodbye, %s!\n", usernamecopy);
    send(client_fd, response, strlen(response), 0);
}

bool is_admin(const char* username) {
    FILE* file = fopen("database/users.txt", "r");
    if (file == NULL) {
        return 0;
    }
    char stored_username[16], stored_password[20], stored_type[10];

    char line[1000];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^/]/%[^/]/%[^/]", stored_username, stored_password, stored_type);
        if (strcmp(stored_username, username) == 0) {
            bool is_admin = (strcmp(stored_type, "1") == 0);
            fclose(file);
            return is_admin;
        }
    }

    printf("this worked is_admin");
    fclose(file);
    return 0;
}

bool is_profile_private(const char* username) {
    FILE* file = fopen("database/privateprofiles.txt", "r");
    if (file == NULL) return 0;

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, username) == 0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

void handle_set_privacy(int client_fd, char* parameters) {
    const char* user = get_session_username(client_fd);
    FILE* file = fopen("database/privateprofiles.txt", "r");
    int mode = -1;
    char error_message[256];

    if (parameters == NULL || sscanf(parameters, "%d", &mode) != 1 || (mode != 0 && mode != 1)) {
        strcpy(error_message, "Usage: set_privacy <1/0> (1=Private, 0=Public)\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    char private_users[1000][16];
    int count = 0;
    if (file) {
        char line[100];
        while (fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = 0;
            if (strcmp(line, user) != 0) {
                strcpy(private_users[count], line);
            }
        }
        fclose(file);
    }

    if (mode == 1) {
        count++;
        strcpy(private_users[count], user);
    }

    file = fopen("database/privateprofiles.txt", "w");
    if (file) {
        int fd = fileno(file);
        flock(fd, LOCK_EX);
        for (int i = 0; i < count; i++) {
            fprintf(file, "%s\n", private_users[i]);
        }
        flock(fd, LOCK_UN);
        fclose(file);
    }

    char response[256];
    sprintf(response, "Private profiles changed.\n");
    send(client_fd, response, strlen(response), 0);
}