#ifndef DATABASE_H
#define DATABASE_H
#include <sqlite3.h>
#include <stddef.h>

struct Date
{
    int month;
    int year;
};

struct Transaction
{
    int id;
    int account_id;
    long account_number;
    float price;
    char *type;
};

struct Account
{
    int id;
    long account_number;
    int balance;
    char *name;
    char *mobile;
    char *email_address;
    struct Date date_opened;
};
struct User
{
    int id;
    char *username;
    char *password;
};
enum EntityType
{
    USER,
    TRANSACTION,
    ACCOUNT
};
struct EntityList
{
    struct Entity *entities;
    size_t size;
};
struct Entity
{
    enum EntityType entity_type;
    struct User user;
    struct Transaction transaction;
    struct Account account;
};
int createTable(sqlite3 *);
int insert(sqlite3 *, struct Entity);
struct EntityList get(sqlite3 *, long account_number, enum EntityType entity_type);
struct EntityList getAll(sqlite3 *,enum EntityType entity_type);
int delete(sqlite3 *, long account_number);
int edit(sqlite3 *, struct Account);
int login(sqlite3*, struct User);
struct EntityList searchAccounts(sqlite3* db,const char* keyword);
struct EntityList searchColumn(sqlite3 *db, const char *column, const char *keyword);
struct Account getLastInsertedAccount(sqlite3 *db);

#endif