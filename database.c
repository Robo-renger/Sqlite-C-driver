#include <stdio.h>
#include <math.h>
#include <sqlite3.h>
#include <string.h>
#include "database.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#define MAX_LENGTH 100

          char *my_strdup(const char *str)
{
    if (str == NULL)
    {
        return NULL;
    }

    size_t len = strlen(str) + 1;
    char *dup = malloc(len);

    if (dup != NULL)
    {
        memcpy(dup, str, len);
    }

    return dup;
}

int createTable(sqlite3 *db)
{
    char *err_msg;
    const char *create_table_sql = "CREATE TABLE IF NOT EXISTS accounts (\
    id INTEGER PRIMARY KEY AUTOINCREMENT,\
    account_number INTEGER,\
    name TEXT,\
    mobile TEXT,\
    email_address TEXT,\
    balance FLOAT,\
    date_opened TEXT\
);\
CREATE TABLE IF NOT EXISTS transactions (\
    id INTEGER PRIMARY KEY AUTOINCREMENT,\
    account_number INTEGER,\
    price FLOAT,\
    type TEXT\
);\
CREATE TABLE IF NOT EXISTS users (\
    id INTEGER PRIMARY KEY AUTOINCREMENT,\
    username TEXT UNIQUE,\
    password TEXT UNIQUE\
);";

    char sql[500];      strcpy(sql, create_table_sql);

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
        const char *sql = "INSERT INTO transactions (account_number,price,type) VALUES (?,?,?);";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }
        sqlite3_bind_int64(stmt, 1, entity.transaction.account_number);
        sqlite3_bind_double(stmt, 2, entity.transaction.price);
        sqlite3_bind_text(stmt, 3, entity.transaction.type, -1, SQLITE_STATIC);
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

struct EntityList get(sqlite3 *db, long account_number, enum EntityType entity_type)
{
    struct EntityList entityList = {.entities = NULL, .size = 0};

    char *err_msg = 0;
    sqlite3_stmt *stmt;
    int rc;
    if (entity_type == TRANSACTION)
    {
        const char *sql = "SELECT * FROM transactions WHERE account_number = ?";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return entityList;          }

        sqlite3_bind_int64(stmt, 1, account_number);
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            struct Entity entity;
            entity.entity_type = TRANSACTION;

            entity.transaction.id = sqlite3_column_int(stmt, 0);
            entity.transaction.account_number = sqlite3_column_int64(stmt, 1);
            entity.transaction.price = sqlite3_column_double(stmt, 2);
            const char *type = (const char *)sqlite3_column_text(stmt, 3);
                         entity.transaction.type = my_strdup(type);

                         entityList.entities = realloc(entityList.entities, (entityList.size + 1) * sizeof(struct Entity));
            entityList.entities[entityList.size++] = entity;
        }
    }
    else if (entity_type == ACCOUNT)
    {
        const char *sql = "SELECT * FROM accounts WHERE account_number = ?";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return entityList;          }
        sqlite3_bind_int64(stmt, 1, account_number);

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            struct Entity entity;
            entity.entity_type = ACCOUNT;

            entity.account.id = sqlite3_column_int(stmt, 0);
            entity.account.account_number = sqlite3_column_int64(stmt, 1);
            entity.account.balance = sqlite3_column_int(stmt, 5);
                                      const char *name = (const char *)sqlite3_column_text(stmt, 2);
            const char *mobile = (const char *)sqlite3_column_text(stmt, 3);
            const char *email_address = (const char *)sqlite3_column_text(stmt, 4);
            const char *dateOpened = (const char *)sqlite3_column_text(stmt, 6);
            sscanf(dateOpened, "%d-%d", &entity.account.date_opened.month, &entity.account.date_opened.year);
                         entity.account.name = my_strdup(name);
            entity.account.mobile = my_strdup(mobile);
            entity.account.email_address = my_strdup(email_address);

                         entityList.entities = realloc(entityList.entities, (entityList.size + 1) * sizeof(struct Entity));
            entityList.entities[entityList.size++] = entity;
                     }
    }
    else
    {
        fprintf(stderr, "Invalid entity type\n");
        return entityList;      }

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);

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
        return entityList;      }

    sqlite3_finalize(stmt);
    return entityList;
}

struct EntityList getAll(sqlite3 *db, enum EntityType entity_type)
{
    struct EntityList entityList = {.entities = NULL, .size = 0};

    char *err_msg = 0;
    sqlite3_stmt *stmt;
    int rc;

    const char *tableName = (entity_type == TRANSACTION) ? "Transactions" : (entity_type == ACCOUNT) ? "accounts"
                                                                                                     : NULL;

    if (tableName == NULL)
    {
        fprintf(stderr, "Invalid entity type\n");
        return entityList;      }

         char sql[100];
    snprintf(sql, sizeof(sql), "SELECT * FROM %s", tableName);

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return entityList;      }

         while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {

        struct Entity entity;
        entity.entity_type = entity_type;

                 if (entity_type == TRANSACTION)
        {
            const char *type = (const char *)sqlite3_column_text(stmt, 3);

            entity.transaction.id = sqlite3_column_int(stmt, 0);
            entity.transaction.account_number = sqlite3_column_int(stmt, 1);
            entity.transaction.type = (type != NULL) ? my_strdup(type) : NULL;
            
                         if (sqlite3_column_type(stmt, 2) == SQLITE_NULL)
            {
                entity.transaction.price = 0.0;              }
            else
            {
                entity.transaction.price = sqlite3_column_double(stmt, 2);
            }
        }
        else if (entity_type == ACCOUNT)
        {
            entity.account.id = sqlite3_column_int(stmt, 0);

                         if (sqlite3_column_type(stmt, 5) == SQLITE_NULL || sqlite3_column_type(stmt, 6) == SQLITE_NULL)
            {
                entity.account.balance = 0;                  entity.account.date_opened.month = 1;
                entity.account.date_opened.year = 2000;
            }
            else
            {
                entity.account.balance = sqlite3_column_int(stmt, 5);
                const char *dateOpened = (const char *)sqlite3_column_text(stmt, 6);
                sscanf(dateOpened, "%d-%d", &entity.account.date_opened.month, &entity.account.date_opened.year);
            }

                                      entity.account.account_number = sqlite3_column_int64(stmt, 1);
            const char *name = (const char *)sqlite3_column_text(stmt, 2);
            const char *mobile = (const char *)sqlite3_column_text(stmt, 3);
            const char *email_address = (const char *)sqlite3_column_text(stmt, 4);

                         entity.account.name = (name != NULL) ? my_strdup(name) : NULL;
            entity.account.mobile = (mobile != NULL) ? my_strdup(mobile) : NULL;
            entity.account.email_address = (email_address != NULL) ? my_strdup(email_address) : NULL;
        }
        else
        {
            fprintf(stderr, "Invalid entity type\n");
            return entityList;          }

                 entityList.entities = realloc(entityList.entities, (entityList.size + 1) * sizeof(struct Entity));
        entityList.entities[entityList.size++] = entity;
    }

    if (rc != SQLITE_DONE && rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);

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

        return entityList;      }

    sqlite3_finalize(stmt);
    return entityList;
}

int delete(sqlite3 *db, long account_number)
{
    char *err_msg = 0;

         const char *delete_transactions_sql = "DELETE FROM transactions WHERE account_number = ?";
    sqlite3_stmt *stmt_transactions;
    int rc_transactions = sqlite3_prepare_v2(db, delete_transactions_sql, -1, &stmt_transactions, 0);

    if (rc_transactions != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error (transactions deletion): %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_int64(stmt_transactions, 1, account_number);
    rc_transactions = sqlite3_step(stmt_transactions);

    if (rc_transactions != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error (transactions deletion): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt_transactions);
        return 1;
    }

    sqlite3_finalize(stmt_transactions);

         const char *delete_account_sql = "DELETE FROM accounts WHERE account_number = ?";
    sqlite3_stmt *stmt_account;
    int rc_account = sqlite3_prepare_v2(db, delete_account_sql, -1, &stmt_account, 0);

    if (rc_account != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error (account deletion): %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_int64(stmt_account, 1, account_number);
    rc_account = sqlite3_step(stmt_account);

    if (rc_account != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error (account deletion): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt_account);
        return 1;
    }

    sqlite3_finalize(stmt_account);

    return 0;  }


int edit(sqlite3 *db, struct Account editedAccount)
{
    char *err_msg = 0;
    sqlite3_stmt *stmt;
    int rc;

         const char *update_account_sql = "UPDATE accounts SET account_number = ?, name = ?, mobile = ?,email_address = ?, balance = ?, date_opened =? WHERE id = ?";
    rc = sqlite3_prepare_v2(db, update_account_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error (account update): %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int64(stmt, 1, editedAccount.account_number);
    sqlite3_bind_text(stmt, 2, editedAccount.name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, editedAccount.mobile, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, editedAccount.email_address, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, editedAccount.balance);
    char *date_opened_str = sqlite3_mprintf("%02d-%d", editedAccount.date_opened.month, editedAccount.date_opened.year);
    sqlite3_bind_text(stmt, 6, date_opened_str, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, editedAccount.id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error (account update): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);

    sqlite3_free((void *)date_opened_str);
    return 1;
}

int login(sqlite3 *db, struct User user)
{
    char *err_msg = 0;

         const char *sql = "SELECT id FROM users WHERE username = ? AND password = ?";
    sqlite3_stmt *stmt;

         int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        return 0;      }

         sqlite3_bind_text(stmt, 1, user.username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user.password, -1, SQLITE_STATIC);

         rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW)
    {
                 int user_id = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return 1;      }
    else if (rc != SQLITE_DONE)
    {
                 fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
    }

         sqlite3_finalize(stmt);
    return 0;
}
struct EntityList searchAccounts(sqlite3 *db, const char *keyword)
{
    struct EntityList resultList = {.entities = NULL, .size = 0};

         char *searchKeyword = sqlite3_mprintf("%%%s%%", keyword);

    const char *sql = "SELECT * FROM accounts WHERE name LIKE ? COLLATE NOCASE OR mobile LIKE ? COLLATE NOCASE OR email_address LIKE ? COLLATE NOCASE OR date_opened LIKE ? COLLATE NOCASE";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        sqlite3_free(searchKeyword);
        return resultList;      }

         rc = sqlite3_bind_text(stmt, 1, searchKeyword, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_free(searchKeyword);
        return resultList;      }

    rc = sqlite3_bind_text(stmt, 2, searchKeyword, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_free(searchKeyword);
        return resultList;      }

    rc = sqlite3_bind_text(stmt, 3, searchKeyword, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_free(searchKeyword);
        return resultList;      }

    rc = sqlite3_bind_text(stmt, 4, searchKeyword, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_free(searchKeyword);
        return resultList;      }
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        struct Entity entity;
        entity.entity_type = ACCOUNT;

        entity.account.id = sqlite3_column_int(stmt, 0);
        entity.account.account_number = sqlite3_column_int64(stmt, 1);
        const char *name = (const char *)sqlite3_column_text(stmt, 2);
        const char *mobile = (const char *)sqlite3_column_text(stmt, 3);
        const char *email_address = (const char *)sqlite3_column_text(stmt, 4);
        entity.account.balance = sqlite3_column_int(stmt, 5);

        const char *date_opened_str = (const char *)sqlite3_column_text(stmt, 6);

        entity.account.name = my_strdup(name);
        entity.account.mobile = my_strdup(mobile);
        entity.account.email_address = my_strdup(email_address);

                 sscanf(date_opened_str, "%d-%d", &entity.account.date_opened.month, &entity.account.date_opened.year);

                 resultList.entities = realloc(resultList.entities, (resultList.size + 1) * sizeof(struct Entity));
        resultList.entities[resultList.size++] = entity;
    }

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return resultList;
}
struct EntityList searchColumn(sqlite3 *db, const char *column, const char *keyword)
{
    struct EntityList resultList = {.entities = NULL, .size = 0};

         const char *sqlTemplate = "SELECT * FROM accounts WHERE %s LIKE ? COLLATE NOCASE";
    char *sql = sqlite3_mprintf(sqlTemplate, column);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    sqlite3_free(sql);  
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        return resultList;      }

         char searchPattern[MAX_LENGTH];
    snprintf(searchPattern, sizeof(searchPattern), "%%%s%%", keyword);

         rc = sqlite3_bind_text(stmt, 1, searchPattern, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return resultList;      }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        struct Entity entity;
        entity.entity_type = ACCOUNT;

        entity.account.id = sqlite3_column_int(stmt, 0);
        entity.account.account_number = sqlite3_column_int64(stmt, 1);
        const char *name = (const char *)sqlite3_column_text(stmt, 2);
        const char *mobile = (const char *)sqlite3_column_text(stmt, 3);
        const char *email_address = (const char *)sqlite3_column_text(stmt, 4);
        entity.account.balance = sqlite3_column_int(stmt, 5);

        const char *date_opened_str = (const char *)sqlite3_column_text(stmt, 6);

        entity.account.name = my_strdup(name);
        entity.account.mobile = my_strdup(mobile);
        entity.account.email_address = my_strdup(email_address);

                 sscanf(date_opened_str, "%d-%d", &entity.account.date_opened.month, &entity.account.date_opened.year);

                 resultList.entities = realloc(resultList.entities, (resultList.size + 1) * sizeof(struct Entity));
        resultList.entities[resultList.size++] = entity;
    }

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return resultList;
}
struct Account getLastInsertedAccount(sqlite3 *db)
{
    struct Account account;

         const char *sql = "SELECT * FROM accounts ORDER BY id DESC LIMIT 1";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
                 exit(EXIT_FAILURE);
    }

         rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW)
    {
                 account.id = sqlite3_column_int(stmt, 0);
        account.account_number = sqlite3_column_int(stmt, 1);
        account.balance = sqlite3_column_int(stmt, 5);

                 account.name = my_strdup((const char *)sqlite3_column_text(stmt, 2));
        account.mobile = my_strdup((const char *)sqlite3_column_text(stmt, 3));
        account.email_address = my_strdup((const char *)sqlite3_column_text(stmt, 4));

                 sscanf((const char *)sqlite3_column_text(stmt, 6), "%d-%d", &account.date_opened.month, &account.date_opened.year);
    }
    else
    {
        fprintf(stderr, "No records found.\n");
                 exit(EXIT_FAILURE);
    }

         sqlite3_finalize(stmt);

    return account;
}