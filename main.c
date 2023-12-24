#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"
#include "database.h"
#include <stddef.h> // Add this line

int main(int argc, char *argv[]) {
    const char *databasePath = "zerbew.db";

    // Open the database
    sqlite3 *db;
    int rc = sqlite3_open(databasePath, &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // Check if the program is called with an argument
    if (argc > 1) {
        // Check if the argument is "getAllAccounts"
        if (strcmp(argv[1], "getAllAccounts") == 0) {
            // Call the getAllAccounts function
            getAllAccounts(db);
        } else {
            fprintf(stderr, "Unknown command: %s\n", argv[1]);
        }
    } else {
        fprintf(stderr, "No command specified. Usage: ./main <command>\n");
    }

    // Close the SQLite database
    sqlite3_close(db);

    return 0;
}