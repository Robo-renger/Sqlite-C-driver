#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "functions.h"

int main(void)
{
    test();
    sqlite3 *db;
    char *err_msg;

    int rc = sqlite3_open("zerbew.db", &db);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
     struct Account editedAccount = {
        .id = 6,               // Replace with the actual user ID you want to edit
        .name = "UpdatedName",   // Replace with the updated name
        .mobile = "UpdatedMobile", // Replace with the updated mobile
        // Add other fields as needed
    };

    // Call the edit function
    if (edit(db, editedAccount) == 0)
    {
        printf("Account record updated successfully.\n");
    }
    else
    {
        printf("Error updating account record.\n");
    }

    sqlite3_close(db);

    return 0;
}
