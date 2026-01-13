#include "posts.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <time.h>
#include "sessions.h"
#include "message.h"
#include "posts.h"

void handle_send_message(int client_fd, char* parameters) {
    char content[200], receivers[256], error_message[250];
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

    char timestamp[30];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M", t);

    FILE* file = fopen("database/messages.txt", "a");
    if (file == NULL) {
        strcpy(error_message, "Error: could not find \"messages.txt\" file"".\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    int receiver_cnt = 0;
    char* receiver = strtok(receivers, ",");

    int fd = fileno(file);
    flock(fd, LOCK_EX);
    while (receiver != NULL) {
        if (strlen(receiver) > 0) {
            fprintf(file, "%s/%s/%s/%s\n", sender, content, receiver, timestamp);
            receiver_cnt++;
        }
        receiver = strtok(NULL, ",");
    }
    flock(fd, LOCK_UN);
    fclose(file);

    if (receiver_cnt == 0) {
        strcpy(error_message, "Error: no recipients specified. Try: message \"content\" <username> (,<username2>, ...)\n");
        send(client_fd, error_message, strlen(error_message), 0);
    } else {
        char success[] = "Message sent!\n";
        send(client_fd, success, strlen(success), 0);
    }
}

void handle_inbox(int client_fd, char* parameters) {
    char error_message[250];
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

    char target_sender[100];
    int show_messages = 0;

    if (strlen(parameters)>0) {
        sscanf(parameters, "%s", target_sender);
        show_messages = 1;
    }

    if (show_messages == 0) {
        char senders[100][16];
        int message_counts[100] = {0};
        int sender_cnt = 0;
        char line[1000];

        while (fgets(line, sizeof(line), file) != NULL) {
            char sender[16], receiver[16];
            char* token_sender = strtok(line, "/");
            char* token_content = strtok(NULL, "/");
            char* token_receiver = strtok(NULL, "/");

            if (token_sender && token_receiver) {
                strcpy(sender, token_sender);
                strcpy(receiver, token_receiver);

                if (strcmp(receiver,user) == 0) {
                    int found = 0;
                    for (int i = 0; i < sender_cnt; i++) {
                        if (strcmp(sender, senders[i]) == 0) {
                            message_counts[i]++;
                            found = 1;
                            break;
                        }
                    }

                    if (found==0 && sender_cnt < 100) {
                        strcpy(senders[sender_cnt], sender);
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
            char inbox[2048] = "INBOX\n";
            for (int i = 0; i < sender_cnt; i++) {
                char line_entry[100];
                sprintf(line_entry, "%s - %d messages\n", senders[i], message_counts[i]);
                strcat(inbox, line_entry);
            }
            strcat(inbox, "\nUse >inbox <username> to read messages.\n");
            send(client_fd, inbox, strlen(inbox), 0);
        }

    } else {
        char inbox[4096];
        sprintf(inbox, "Messages from %s:\n", target_sender);
        char line[1000];
        int count = 0;

        while (fgets(line, sizeof(line), file) != NULL) {
            char sender[16], receiver[16], content[201], timestamp[30];

            char* token_sender = strtok(line, "/");
            char* token_content = strtok(NULL, "/");
            char* token_receiver = strtok(NULL, "/");
            char* token_time = strtok(NULL, "\n");

            if (token_sender && token_content && token_receiver && token_time) {
                strcpy(sender, token_sender);
                strcpy(receiver, token_receiver);
                strcpy(content, token_content);
                strcpy(timestamp, token_time);

                if (strcmp(receiver, user) == 0 && strcmp(sender, target_sender) == 0) {
                    char message_line[350];
                    sprintf(message_line, "[%s] %s: %s\n", timestamp, sender, content);

                    if (strlen(inbox) + strlen(message_line) < sizeof(inbox) - 100) {
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