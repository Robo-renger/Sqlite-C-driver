#include <stdio.h>
#include <math.h>
#include <sqlite3.h>
#include <string.h>
#include "database.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h> 

char *my_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup != NULL)
    {
        memcpy(dup, s, len);
    }
    return dup;
}
int createTable(sqlite3 *db)
{
    char *err_msg;
const char *create_table_sql = "CREATE TABLE IF NOT EXISTS accounts (\
    id INTEGER PRIMARY KEY AUTOINCREMENT,\
    name TEXT,\
    mobile TEXT,\
    email_address TEXT,\
    balance FLOAT,\
    date_opened TEXT\
);\
CREATE TABLE IF NOT EXISTS transactions (\
    id INTEGER PRIMARY KEY AUTOINCREMENT,\
    account_id INTEGER,\
    price FLOAT,\
    type TEXT\
);\
CREATE TABLE IF NOT EXISTS users (\
    id INTEGER PRIMARY KEY AUTOINCREMENT,\
    username TEXT,\
    password TEXT\
);";


    char sql[500]; // Adjust the size based on your SQL statement
    strcpy(sql, create_table_sql);

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }
    return 0;
}

int insert(sqlite3 *db, struct Entity entity)
{
    char *err_msg = 0;
    sqlite3_stmt *stmt;
    int rc;
    if (entity.entity_type == ACCOUNT)
    {
        const char *sql = "INSERT INTO accounts (name ,mobile, email_address, balance, date_opened) VALUES (?, ? ,?, ?, ?);";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }
        sqlite3_bind_text(stmt, 1, entity.account.name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, entity.account.mobile, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, entity.account.email_address, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, entity.account.balance);
        char date_string[10];
        snprintf(date_string, sizeof(date_string), "%d-%d", entity.account.date_opened.month, entity.account.date_opened.year);
        sqlite3_bind_text(stmt, 5, date_string, -1, SQLITE_STATIC);
    }
    else if (entity.entity_type == TRANSACTION)
    {
        const char *sql = "INSERT INTO transactions (account_id,price,type) VALUES (?,?,?);";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }
        sqlite3_bind_int(stmt, 1, entity.transaction.account_id);
        sqlite3_bind_double(stmt, 2, entity.transaction.price);
        sqlite3_bind_text(stmt, 3, entity.transaction.type,-1,SQLITE_STATIC);
    }
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_finalize(stmt);
    return 0;
}



struct EntityList get(sqlite3 *db, int account_id, enum EntityType entity_type)
{
    struct EntityList entityList = {.entities = NULL, .size = 0};

    char *err_msg = 0;
    sqlite3_stmt *stmt;
    int rc;

    if (entity_type == TRANSACTION)
    {
        const char *sql = "SELECT * FROM Transactions WHERE account_id = ?";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return entityList; // Return an empty list on error
        }

        sqlite3_bind_int(stmt, 1, account_id);

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            struct Entity entity;
            entity.entity_type = TRANSACTION;

            entity.transaction.id = sqlite3_column_int(stmt, 0);
            entity.transaction.account_id = sqlite3_column_int(stmt, 1);
            entity.transaction.price = sqlite3_column_double(stmt, 2);

            // Add the entity to the list
            entityList.entities = realloc(entityList.entities, (entityList.size + 1) * sizeof(struct Entity));
            entityList.entities[entityList.size++] = entity;
        }
    }
    else if (entity_type == ACCOUNT)
    {
        const char *sql = "SELECT * FROM accounts WHERE id = ?";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return entityList; // Return an empty list on error
        }

        sqlite3_bind_int(stmt, 1, account_id);

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            struct Entity entity;
            entity.entity_type = ACCOUNT;

            entity.account.id = sqlite3_column_int(stmt, 0);
            entity.account.balance = sqlite3_column_int(stmt, 4);

            // Note: Assuming 'name', 'mobile', and 'email_address' are TEXT fields
            // Retrieve other fields for the User entity
            const char *name = (const char *)sqlite3_column_text(stmt, 1);
            const char *mobile = (const char *)sqlite3_column_text(stmt, 2);
            const char *email_address = (const char *)sqlite3_column_text(stmt, 3);

            // Allocate memory and copy the strings
            entity.account.name = my_strdup(name);
            entity.account.mobile = my_strdup(mobile);
            entity.account.email_address = my_strdup(email_address);

            // Add the entity to the list
            entityList.entities = realloc(entityList.entities, (entityList.size + 1) * sizeof(struct Entity));
            entityList.entities[entityList.size++] = entity;
        }
    }
    else
    {
        fprintf(stderr, "Invalid entity type\n");
        return entityList; // Return an empty list for an invalid entity type
    }

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);

        // Free the allocated memory before returning
        for (size_t i = 0; i < entityList.size; ++i)
        {
            if (entityList.entities[i].entity_type == ACCOUNT)
            {
                free(entityList.entities[i].account.name);
                free(entityList.entities[i].account.mobile);
                free(entityList.entities[i].account.email_address);
            }
        }
        free(entityList.entities);

        return entityList; // Return an empty list on error
    }

    sqlite3_finalize(stmt);
    return entityList;
}
struct EntityList getAll(sqlite3 *db, enum EntityType entity_type)
{
    struct EntityList entityList = {.entities = NULL, .size = 0};

    char *err_msg = 0;
    sqlite3_stmt *stmt;
    int rc;

    const char *tableName = (entity_type == TRANSACTION) ? "Transactions" : 
                                               (entity_type == ACCOUNT) ? "accounts" : NULL;

    if (tableName == NULL)
    {
        fprintf(stderr, "Invalid entity type\n");
        return entityList; // Return an empty list for an invalid entity type
    }

    // Construct the SQL query
    char sql[100];
    snprintf(sql, sizeof(sql), "SELECT * FROM %s", tableName);

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return entityList; // Return an empty list on error
    }

// Example print statements for debugging
while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
{

    struct Entity entity;
    entity.entity_type = entity_type;

    // Adjust the data retrieval based on the entity type
    if (entity_type == TRANSACTION)
    {
        entity.transaction.id = sqlite3_column_int(stmt, 0);
        entity.transaction.account_id = sqlite3_column_int(stmt, 1);

        // Check for NULL value in 'price' column
        if (sqlite3_column_type(stmt, 2) == SQLITE_NULL) {
            entity.transaction.price = 0.0;  // Or set it to a default value
        } else {
            entity.transaction.price = sqlite3_column_double(stmt, 2);
        }
    }
    else if (entity_type == ACCOUNT)
    {
        entity.account.id = sqlite3_column_int(stmt, 0);

        // Check for NULL values in 'balance' column
        if (sqlite3_column_type(stmt, 4) == SQLITE_NULL || sqlite3_column_type(stmt, 5) == SQLITE_NULL) {
            entity.account.balance = 0;  // Or set it to a default value
            entity.account.date_opened.month = 1;
            entity.account.date_opened.year = 2000;
        } else {
            entity.account.balance = sqlite3_column_int(stmt, 4);
            const char *dateOpened = (const char *)sqlite3_column_text(stmt, 5);
            sscanf(dateOpened, "%d-%d", &entity.account.date_opened.month, &entity.account.date_opened.year);
        }

        // Note: Assuming 'name', 'mobile', and 'email_address' are TEXT fields
        // Retrieve other fields for the User entity
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        const char *mobile = (const char *)sqlite3_column_text(stmt, 2);
        const char *email_address = (const char *)sqlite3_column_text(stmt, 3);

        // Check for NULL values and allocate memory only if not NULL
        entity.account.name = (name != NULL) ? my_strdup(name) : NULL;
        entity.account.mobile = (mobile != NULL) ? my_strdup(mobile) : NULL;
        entity.account.email_address = (email_address != NULL) ? my_strdup(email_address) : NULL;
    }
    else
    {
        fprintf(stderr, "Invalid entity type\n");
        return entityList; // Return an empty list for an invalid entity type
    }

    // Add the entity to the list
    entityList.entities = realloc(entityList.entities, (entityList.size + 1) * sizeof(struct Entity));
    entityList.entities[entityList.size++] = entity;

}


    if (rc != SQLITE_DONE && rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);

        // Free the allocated memory before returning
        if (entityList.entities != NULL)
        {
            for (size_t i = 0; i < entityList.size; ++i)
            {
                if (entityList.entities[i].entity_type == ACCOUNT)
                {
                    if (entityList.entities[i].account.name != NULL)
                    {
                        free(entityList.entities[i].account.name);
                    }
                    if (entityList.entities[i].account.mobile != NULL)
                    {
                        free(entityList.entities[i].account.mobile);
                    }
                    if (entityList.entities[i].account.email_address != NULL)
                    {
                        free(entityList.entities[i].account.email_address);
                    }
                }
            }
            free(entityList.entities);
        }

        return entityList; // Return an empty list on error
    }

    sqlite3_finalize(stmt);
    return entityList;
}



int delete(sqlite3 *db,int account_id)
{
    char *err_msg = 0;

    // Delete transactions associated with the account_id
    const char *delete_transactions_sql = "DELETE FROM transactions WHERE account_id = ?";
    sqlite3_stmt *stmt_transactions;
    int rc_transactions = sqlite3_prepare_v2(db, delete_transactions_sql, -1, &stmt_transactions, 0);

    if (rc_transactions != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error (transactions deletion): %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_int(stmt_transactions, 1, account_id);
    rc_transactions = sqlite3_step(stmt_transactions);

    if (rc_transactions != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error (transactions deletion): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt_transactions);
        return 1;
    }

    sqlite3_finalize(stmt_transactions);

    // Delete the account with the given account_id
    const char *delete_account_sql = "DELETE FROM accounts WHERE id = ?";
    sqlite3_stmt *stmt_account;
    int rc_account = sqlite3_prepare_v2(db, delete_account_sql, -1, &stmt_account, 0);

    if (rc_account != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error (account deletion): %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_int(stmt_account, 1, account_id);
    rc_account = sqlite3_step(stmt_account);

    if (rc_account != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error (account deletion): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt_account);
        return 1;
    }

    sqlite3_finalize(stmt_account);

    return 0; // Success
}

int edit(sqlite3 *db, struct Account editedAccount)
{
    char *err_msg = 0;
    sqlite3_stmt *stmt;
    int rc;

    // Update the account record in the "accounts" table
    const char *update_account_sql = "UPDATE accounts SET balance = ?, name = ?, mobile = ?, email_address = ? WHERE id = ?";
    rc = sqlite3_prepare_v2(db, update_account_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error (account update): %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, editedAccount.balance);
    sqlite3_bind_text(stmt, 2, editedAccount.name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, editedAccount.mobile, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, editedAccount.email_address, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, editedAccount.id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error (account update): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);

    return 1; // Success
}