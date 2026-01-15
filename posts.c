#include "posts.h"

int get_post_content(const char* input, char* content, char* restriction) {
    const char* start = strchr(input, '"');
    if (start == NULL) return 0;
    start++;

    const char* end = strchr(start + 1, '"');
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
        strcpy(error_message, "Error: you must be logged in.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    int extracted_post = get_post_content(parameters, content, restriction);
    if ( extracted_post == -1 || extracted_post == 0) {
        strcpy(error_message, "Error: wrong format. Try: post \"content\" <list>\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    int list = 0;
    if (strlen(restriction)>0) list = strtol(restriction,NULL,10);

    if (list < 0 || list > 3) {
        strcpy(error_message, "Error: you must specify list. Try: 0 - global / 1 - acquaintances / 2 - friends / 3 - close friends.\n");
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
    sprintf(response, "Success: post created!\n");
    send(client_fd, response, strlen(response), 0);
}

int check_view_permission(const char* viewer, const char* poster, int post_permission) {
    if (strcmp(viewer,poster) == 0) {
        return 1;
    }
    if (post_permission == 0) {
        return 1;
    }

    FILE* file = fopen("database/connections.txt", "r");
    if (file == NULL) return 0;

    int level_assigned_by_poster = 0;
    char line[1000];
    while (fgets(line, sizeof(line), file)) {
        char p[16], v[16];
        int level;

        if (sscanf(line, "%[^/]/%[^/]/%d", p, v, &level) == 3) {
            if (strcmp(p, poster) == 0 && strcmp(v, viewer) == 0) {
                level_assigned_by_poster = level;
                break;
            }
        }
    }
    fclose(file);

    if (level_assigned_by_poster >= post_permission && level_assigned_by_poster > 0)
        return 1;

    return 0;
}

void handle_viewposts(int client_fd, char* parameters) {
    char error_message[100], target_user[16] = {0};
    const char* viewer = get_session_username(client_fd);
    int is_logged_in = (viewer != NULL);

    if (parameters != NULL && strlen(parameters) > 0) {
        if (sscanf(parameters, "%15s", target_user) != 1) {
         target_user[0] = '\0';
        }
    }else {
        target_user[0] = '\0';
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

        int parsed = sscanf(line, "%[^/]/%[^/]/%d/%[^\n]", poster, content, &permission, timestamp);

        if (parsed == 4) {
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
                sprintf(post_entry, "\n(%s) @%s\n%s\n", timestamp, poster, content);

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

void handle_delete_post(const int client_fd, char* parameters) {
    char error_message[256];

    const char* user = get_session_username(client_fd);
    if (user == NULL) {
        strcpy(error_message, "Error: you must be logged in to delete posts.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    char target_author[100];
    char target_content[141];
    const char* p = parameters;

    if (is_admin(user) && parameters[0] != '"') {
        sscanf(parameters, "%s", target_author);
        p = strchr(parameters, '"');
    } else {
        strcpy(target_author, user);
        p = strchr(parameters, '"');
    }

    if (!p) {
        strcpy(error_message, "Error: Content must be in quotes. Try: deletepost \"content\"\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    char* end = strchr(p + 1, '"');
    if (!end) {
        strcpy(error_message, "Error: Content must be in quotes. Try: deletepost \"content\"\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }
    int len = end - p - 1;
    if (len <= 0 || len > 140) {
        strcpy(error_message, "Error: Invalid content length.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    strncpy(target_content, p + 1, len);
    target_content[len] = '\0';

    FILE* file = fopen("database/posts.txt", "r");
    if (file == NULL) {
        strcpy(error_message, "Error with posts.txt file.\n");
        send(client_fd, error_message, strlen(error_message), 0);
        return;
    }

    char lines_to_keep[1000][500];
    int keep_cnt = 0;
    int deleted_cnt = 0;
    char line[500];

    while (fgets(line,sizeof(line), file) && keep_cnt < 1000) {
        char line_copy[500];
        strcpy(line_copy, line);

        char* tokens[5];
        char* t_poster = strtok(line_copy, "/");
        char* t_content = strtok(NULL, "/");

        if (t_poster && t_content) {
            if (strcmp(t_poster, target_author) == 0 && strcmp(t_content, target_content) == 0) {
                deleted_cnt++;
                continue;
            }
        }
        strcpy(lines_to_keep[keep_cnt++], line);
    }
    fclose(file);

    if (deleted_cnt > 0) {
        file = fopen("database/posts.txt", "w");
        if (file) {
            int fd = fileno(file);
            flock(fd, LOCK_EX);
            for (int i = 0; i < keep_cnt; i++) {
                fprintf(file, "%s", lines_to_keep[i]);
            }
            flock(fd, LOCK_UN);
            fclose(file);
        }

        char success[256];
        sprintf(success, "Post deleted.\n");
        send(client_fd, success, strlen(success), 0);
    }
}