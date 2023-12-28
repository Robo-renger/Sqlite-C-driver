#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "database.h"
#include <sqlite3.h>
#include <stddef.h>

void freeEntityList(struct EntityList *entityList);

void createAccount(sqlite3* db);

void getAllAccounts(sqlite3* db);

int generateUniqueID(const struct Account *account);

void getCurrentDate(struct Date *currentDate);

int save(sqlite3 *db,struct Account *accounts,struct EntityList *accountsList,int numberOfAccounts);

void menu(sqlite3 *db);

void withdraw(sqlite3 *db);

void deposit(sqlite3 *db);

void transfer(sqlite3 *db);

void makeTransaction(sqlite3 *db, struct Account account, char *transaction_type,double amount);

void getAllTransactions(sqlite3 *db);

void getTransactions(sqlite3 *db);

void print(struct EntityList *entityList,sqlite3 *db);

void sortByName(struct EntityList *entityList,sqlite3 *db);

void sortByBalance(struct EntityList *entityList,sqlite3 *db);

void sortByDate(struct EntityList *entityList,sqlite3 *db);

int loginUser(sqlite3* db);

void advancedSearch(sqlite3*db);

void regularSearch(sqlite3 *db);

long generateAccountNumber(struct Account *account);

int isValidName(const char *name);

int isValidEmail(const char *email);

int isValidPhoneNumber(const char *mobile);

int isBlank(const char *str);


#endif  