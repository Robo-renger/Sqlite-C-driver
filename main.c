#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"
#include "database.h"
#include <stddef.h>

int main(int argc, char *argv[])
{
    const char *databasePath = "zerbew.db";

    // Open the database
    sqlite3 *db;
    int rc = sqlite3_open(databasePath, &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    Menu(db);
    // createTable(db);
    //createAccount(db);
    // loginUser(db);
    // Withdraw(db);
    // getTransactions(db);
    // struct Account account;
    // struct Date account_date;
    // account_date.year = 2023;
    // account_date.month = 2; // Set month to 2 for February
    // account.id = 22;
    // account.name = "adsas";
    // account.date_opened = account_date;

    // long account_number = generateAccountNumber(&account);
    // printf("%ld", account_number);

    // regularSearch(db);

    // Close the SQLite database
    sqlite3_close(db);

    return 0;
}
