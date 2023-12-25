#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "database.h"
#include <sqlite3.h>
#include <stddef.h>

void freeEntityList(struct EntityList *entityList);

void createAccount(sqlite3* db);

void createAccount(sqlite3* db);

void getAllAccounts(sqlite3* db);

int generateUniqueID(const struct Account *account);

void getCurrentDate(struct Date *currentDate);

void Menu(sqlite3 *db);

void Withdraw(sqlite3 *db);

void Deposit(sqlite3 *db);

void Transfer(sqlite3 *db);

void getAllTransactions(sqlite3 *db);

void getTransactions(sqlite3 *db);
void Print(struct EntityList *entityList);

void SortByName(struct EntityList *entityList);

void SortByBalance(struct EntityList *entityList);

void SortByDate(struct EntityList *entityList); 

#endif  