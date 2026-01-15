#include "message.h"

void handle_send_message(int client_fd, char* parameters) {
    char content[200], receivers[256], error_message[250];
    FILE* file = fopen("database/messages.txt", "a");
    const char* sender = get_session_username(client_fd);

    if (sender == NULL) {
        strcpy(error_message, "Error: you must be logged in to send a message.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    int extracted = get_post_content(parameters, content, receivers);
    if (extracted != 1) {
        strcpy(error_message, "Error: wrong format. Try: message \"content\" <username> (,<username2>, ...)\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (strlen(receivers) == 0) {
        strcpy(error_message, "Error: no receivers specified.\n");
        send(client_fd, error_message,strlen(error_message),0);
        return;
    }

    // checks if all receivers are real
    char receivers_copy[256];
    strncpy(receivers_copy, receivers, sizeof(receivers_copy) - 1);
    receivers_copy[sizeof(receivers_copy)-1] = '\0';
    char* token = strtok(receivers_copy, ",");
    while (token!=NULL) {
        if (!username_exists(token)) {
            sprintf(error_message, "Error: username \"%s\" does not exist.\n", token);
            send(client_fd, error_message, strlen(error_message), 0);
            return;
        }
        token = strtok(NULL, ",");
    }

    char timestamp[30];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M", t);

    if (file == NULL) {
        strcpy(error_message, "Error: could not find \"messages.txt\" file"".\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    int fd = fileno(file);
    flock(fd, LOCK_EX);
    fprintf(file, "%s/%s/%s/%s\n", sender, content, receivers, timestamp);
    flock(fd, LOCK_UN);
    fclose(file);

    char success[100];
    strcpy(success, "Message sent!\n");
    send(client_fd, success, strlen(success), 0);
}

int in_receivers_list(const char* list, const char* user) {
    const char* ptr = strstr(list, user);
    size_t len = strlen(user);

    while (ptr != NULL) {
        int left_ok  = (ptr == list || *(ptr - 1) == ',');
        int right_ok = (*(ptr + len) == ',' || *(ptr + len) == '\0');

        if (left_ok && right_ok)
            return 1;

        ptr = strstr(ptr + 1, user);
    }
    return 0;
}


void handle_inbox(int client_fd, char* parameters) {
    char error_message[250], target_sender[16] = {0};
    const char* user = get_session_username(client_fd);
    FILE* file = fopen("database/messages.txt", "r");

    if (user == NULL) {
        strcpy(error_message, "Error: you have to log in to use this command.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    if (file == NULL) {
        strcpy(error_message, "Error: could not find \"messages.txt\" file"".\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    if (strlen(parameters)>0) {
        sscanf(parameters, "%15s", target_sender);
    }

    char line[1000];
    if (target_sender[0] == '\0') { //inbox
        char senders[100][16];
        int message_counts[100] = {0};
        int sender_cnt = 0;

        while (fgets(line, sizeof(line), file) != NULL) {
            char sender[16]={0}, text[201]={0}, receivers[256]={0};
            if (sscanf(line, "%[^/]/%[^/]/%[^/]", sender, text, receivers) == 3) {
                if (in_receivers_list(receivers, user)){
                    int found = 0;
                    for (int i = 0; i < sender_cnt; i++) {
                        if (strcmp(sender, senders[i]) == 0) {
                            message_counts[i]++;
                            found = 1;
                            break;
                        }
                    }
                    if (!found && sender_cnt < 100) {
                        strncpy(senders[sender_cnt], sender, 15);
                        senders[sender_cnt][15] = '\0';
                        message_counts[sender_cnt] = 1;
                        sender_cnt++;
                    }
                }
            }
        }
        fclose(file);

        if (sender_cnt == 0) {
            send(client_fd, "Inbox is empty.\n", 16, 0);
        } else {
            char inbox[2048] = "-0- -0- INBOX -0- -0-\n";
            for (int i = 0; i < sender_cnt; i++) {
                char line_entry[100];
                sprintf(line_entry, "%s - %d new messages\n", senders[i], message_counts[i]);
                strcat(inbox, line_entry);
            }
            strcat(inbox, "\nUse >inbox <username> to read messages.\n");
            send(client_fd, inbox, strlen(inbox), 0);
        }
    }
    else{ //inbox <username>
        char inbox[4096];
        sprintf(inbox, "-0- -0- Messages from %s -0- -0-\n", target_sender);
        char line[1000];
        int count = 0;

        rewind(file);

        while (fgets(line, sizeof(line), file) != NULL) {
            char sender[16], receiver[16], content[201], timestamp[30];

            if (sscanf(line, "%[^/]/%[^/]/%[^/]/%[^\n]", sender, content, receiver, timestamp) >= 3) {
                if (strcmp(sender, target_sender) == 0 && in_receivers_list(receiver, user)) {
                    char message_line[350];
                    sprintf(message_line, "[%s] %s: %s\n", timestamp, sender, content);

                    if (strlen(inbox) + strlen(message_line) < 3900) {
                        strcat(inbox, message_line);
                        count++;
                    }
                }
            }
        }
        fclose(file);
        if (count == 0) {
            strcat(inbox, "No messages from this user.\n");
        }
        send(client_fd, inbox, strlen(inbox), 0);
    }
}