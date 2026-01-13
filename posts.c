#include "posts.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <time.h>
#include "sessions.h"
#include "connections.h"

int get_post_content(const char* input, char* content, char* restriction) {
    const char* start = strchr(input, '"');
    if (start == NULL) return 0;
    start++;

    const char* end = strchr(start, '"');
    if (end == NULL) return 0;

    int length = end - start;
    if (length > 140) return -1;

    strncpy(content, start, length);
    content[length] = '\0';

    end++;
    while (*end == '"' || *end == ' ') end++;
    strcpy(restriction, end);
    return 1;
}

void handle_post(int client_fd, char* parameters) {
    char content[141], restriction[5], error_message[250];
    const char* poster = get_session_username(client_fd);
    if (poster == NULL) {
        strcpy(error_message, "Error: you must be logged in.");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    int extracted_post = get_post_content(parameters, content, restriction);
    if ( extracted_post == -1 ) {
        strcpy(error_message, "Error: wrong format. Try: post \"content\" <list>\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    int list = 0;
    if (strlen(restriction)>0) list = atoi(restriction);

    if (list < 0 || list > 3) {
        strcpy(error_message, "Error: you must specify list. Try: 0 - global / 1 - acquaintances / 2 - friends / 3 - close friends.");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    char timestamp[30];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M", t);

    FILE* file = fopen("database/posts.txt", "a");
    if (file == NULL) {
        strcpy(error_message, "Error: can't find posts file.");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    int fd = fileno(file);
    flock(fd, LOCK_EX);
    fprintf(file, "%s/%s/%d/%s\n", poster, content, list, timestamp);
    flock(fd, LOCK_UN);
    fclose(file);

    char response[100];
    sprintf(response, "Success: post created!");
    send(client_fd, response, strlen(response), 0);
}

int check_view_permission(const char* viewer, const char* poster, int post_permission) {
    if (strcmp(viewer,poster) == 0) {
        return 1;
    }
    if (post_permission == 0) {
        return 1;
    }

    int friendship_level = connection_level(viewer, poster);
    if (friendship_level == 0) {
        return 0;
    }

    if (post_permission == 3) return friendship_level >= 3;
    else if (post_permission == 2) return friendship_level >= 2;
    else if (post_permission == 1) return friendship_level >= 1;

    return 0;
}

void handle_viewposts(int client_fd, char* parameters) {
    char error_message[100], target_user[16];
    const char* viewer = get_session_username(client_fd);
    int is_logged_in = (viewer != NULL);

    if (parameters != NULL) {
        sscanf(parameters, "%15s", target_user);
    }

    FILE* file = fopen("database/posts.txt", "r");
    if (file == NULL) {
        strcpy(error_message, "No posts yet.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    char timeline[4096] = "TIMELINE\n";
    char line[1000];
    int post_cnt = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        char poster[16], content[140], timestamp[30];
        int permission;

        char* token1 = strtok(line, "/");
        char* token2 = strtok(NULL, "/");
        char* token3 = strtok(NULL, "/");
        char* token4 = strtok(NULL, "\n");

        if (token1 && token2 && token3 && token4) {
            strcpy(poster, token1);
            strcpy(content, token2);
            permission = strtol(token3,NULL,10);
            strcpy(timestamp, token4);

            if (strlen(target_user) > 0 && strcmp(poster, target_user) != 0) {
                continue;
            }

            int can_view = 0;
            if (!is_logged_in) {
                if (permission == 0) can_view = 1;
                else can_view = 0;
            } else {
                can_view = check_view_permission(viewer, poster, permission);
            }

            if (can_view) {
                char post_entry[500];
                sprintf(post_entry, "\n[%s] %s:\n%s\n", timestamp, poster, content);

                if (strlen(timeline) + strlen(post_entry) < sizeof(timeline) - 100) {
                    strcat(timeline, post_entry);
                    post_cnt++;
                }
            }
        }
    }
    fclose(file);
    send(client_fd, timeline, strlen(timeline), 0);
}