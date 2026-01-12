#include "posts.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sessions.h"
#include "connections.h"

int get_post_content(const char* input, char* content, char* rest) {
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
    while (*end == '"') end++;
    strcpy(rest, end);
    return 1;
}