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

void Save(sqlite3 *db,struct Account *accounts,struct EntityList *accountsList,int numberOfAccounts);

void Menu(sqlite3 *db);

void Withdraw(sqlite3 *db);

void Deposit(sqlite3 *db);

void Transfer(sqlite3 *db);

void getAllTransactions(sqlite3 *db);

void getTransactions(sqlite3 *db);

void Print(struct EntityList *entityList,sqlite3 *db);

void SortByName(struct EntityList *entityList,sqlite3 *db);

void SortByBalance(struct EntityList *entityList,sqlite3 *db);

void SortByDate(struct EntityList *entityList,sqlite3 *db);

int loginUser(sqlite3* db);

void advancedSearch(sqlite3*db);

void regularSearch(sqlite3 *db);

long generateAccountNumber(struct Account *account);

#endif  