#include "functions.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <stddef.h> // Add this line
void freeEntityList(struct EntityList *entityList) {
    if (entityList->entities != NULL) {
        for (size_t i = 0; i < entityList->size; ++i) {
            if (entityList->entities[i].entity_type == ACCOUNT) {
                if (entityList->entities[i].account.name != NULL) {
                    free(entityList->entities[i].account.name);
                }
                if (entityList->entities[i].account.mobile != NULL) {
                    free(entityList->entities[i].account.mobile);
                }
                if (entityList->entities[i].account.email_address != NULL) {
                    free(entityList->entities[i].account.email_address);
                }
            }
        }
        free(entityList->entities);
    }
}
void getAllTransactions(sqlite3* db){
    struct EntityList transactionList = getAll(db, TRANSACTION);

    // Process or print the retrieved entities (example)
    printf("Transaction entities:\n");
    for (size_t i = 0; i < transactionList.size; ++i)
    {
        printf("Transaction ID: %d, Account ID: %d, Price: %lf\n",
               transactionList.entities[i].transaction.id,
               transactionList.entities[i].transaction.account_id,
               transactionList.entities[i].transaction.price);
    }

    // Free allocated memory for transaction entities
    freeEntityList(&transactionList);

}
void getAllAccounts(sqlite3 *db)
{

    // Retrieve entities of type ACCOUNT
    struct EntityList accountList = getAll(db, ACCOUNT);

    // Process or print the retrieved entities (example)
    for (size_t i = 0; i < accountList.size; ++i)
    {
        printf("%d,%s,%s,%s,%d\n",
               accountList.entities[i].account.id,
               accountList.entities[i].account.name,
               accountList.entities[i].account.mobile,
               accountList.entities[i].account.email_address,
               accountList.entities[i].account.balance
               );
    }

    // Free allocated memory for account entities
    freeEntityList(&accountList);

    // Close the database
    sqlite3_close(db);
}