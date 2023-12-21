#ifndef DATABASE_H
#define DATABASE_H

struct Date
{
    int month;
    int year;
};

struct Transaction
{
    int id;
    int account_id;
    float price;
};

struct Account
{
    int id;
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
struct EntityList get(sqlite3 *, int account_id, enum EntityType entity_type);
int delete(sqlite3 *, int account_id);
int edit(sqlite3 *, struct Account);

#endif