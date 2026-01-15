#include "commands.h"

void handle_ping(int client_fd, char* parameters) {
    send(client_fd, "pong\n", 5, 0);
}

void handle_help(int client_fd, char* parameters) {
    const char* help_text =
        "\n\n"
        "  _                          _                       _  _\n "
        "| |_  ___  _ __  _ __ ___  (_) _ __           __ _ | || |\n"
        " | __|/ _ \\| '__|| '_ ` _ \\ | || '_ \\  _____  / _` || || |\n"
        " | |_|  __/| |   | | | | | || || | | ||_____|| (_| || || |\n"
        " \\__|\\___||_|   |_| |_| |_||_||_| |_|        \\__,_||_||_|\n\n"
        "-0- -0- Available commands -0- -0-\n\n"
        "> help : Figure out what to do.\n\n"
        "> signup <username> <password> : Join the \"platform\".\n"
        "> login <username> <password> : Access your account.\n"
        "> logout : End your session.\n\n"
        "> post \"content\" <level> : Share (termin-)all your thoughts.\n"
        "(Legend for post levels: 0 - global, 1 - connections, 2 - friends, 3 - close friends.)\n"
        "> delete_post \"content of post\" : Take back your words.\n"
        "> view_posts : View (termin-)all posts available to you.\n"
        "> view_posts <username> : View posts from a specific user.\n\n"
        "> add_friend <username> <level> : Make a connection with a user on termin-all. Levels from post apply here.\n"
        "> remove_friend <username> : Burn the bridge (you can always add them back).\n\n"
        "> message \"text\" <username> (<username2> ...) : Send a private text to one or multiple users.\n"
        "> inbox : Check what users sent you messages and how many.\n"
        "> inbox <username> : See messages sent from a specific user.\n\n"
        "> exit : You're done. Automatically logs you out.\n";

    send(client_fd, help_text, strlen(help_text), 0);
}

void handle_exit(int client_fd, char* parameters) {
    send(client_fd, "exiting...\n", 11, 0);
}

void handle_unknown(int client_fd, char* parameters) {
    send(client_fd, "unknown command\n", 16,0);
}