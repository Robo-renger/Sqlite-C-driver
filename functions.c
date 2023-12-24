#include "functions.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <stddef.h>

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

void Menu(sqlite3 *db)
{
    int choice;
    printf("Choose an option:\n");
    printf("1. Withdraw\n");
    printf("2. Deposit\n");
    printf("3. Exit\n");
    printf("Enter the number corresponding to your choice: ");
    scanf("%d", &choice);

    switch (choice)
    {
        case 1:
            Withdraw(db);
            break;
        case 2:
            Deposit(db);
            break;
        case 3:
            exit(1);
        default:
            printf("Invalid choice, please try again.\n");
            Menu(db);
    }
}

void Withdraw(sqlite3 *db)
{
    int account_id;
    double amount;

    printf("Please, enter your account number:");
    while (scanf("%d",&account_id) != 1)
    {
        while(getchar()!='\n');
        printf("Invalid input for account number, please enter a valid number.\n");
        printf("Please, enter your account number:");
    }
    
    struct EntityList accountList = get(db, account_id, ACCOUNT);
    if(accountList.size == 0)
    {
        printf("Account number not found, please try again.\n");
        Withdraw(db);
    }

    struct Account account = accountList.entities[0].account;
    if(account.balance == 0)
    {
        printf("Transaction will not proceed as your balance is ZERO\n");
        Menu(db);
    }

    printf("Enter the withdrawal amount: ");
    while (scanf("%lf",&amount) != 1)
    {
        while(getchar()!='\n');
        printf("Invalid input for amount, please enter valid amount.\n");
        printf("Enter the withdrawal amount: ");
    }

    while(amount > 10000)
    {
        printf("Transaction is not completed, the maximum withdrawal limit for each transaction is 10,000$, please try again.\n");
        printf("Enter the withdrawal amount: ");
        scanf("%lf", &amount);
    }

    while(amount > account.balance)
    {
        printf("Withdrawal amount exceeds your current balance. Please enter a valid amount.\n");
        printf("Enter the withdrawal amount: ");
        scanf("%lf", &amount);
    }
    
    account.balance -= amount;
    int ed = edit(db,account);
    if (ed == 1)
    {
        printf("Transaction is completed successfully\n");
        Menu(db);
    }
}

void Deposit(sqlite3 *db)
{
    int account_id;
    double amount;

    printf("Please, enter your account number:");
    while (scanf("%d",&account_id) != 1)
    {
        while(getchar()!='\n');
        printf("Invalid input for account number, please enter a valid number.\n");
        printf("Please, enter your account number:");
    }
    
    struct EntityList accountList = get(db, account_id, ACCOUNT);
    if(accountList.size == 0)
    {
        printf("Account number not found, please try again.\n");
        Deposit(db);
    }

    struct Account account = accountList.entities[0].account;
    
    printf("Enter the deposit amount: ");
    while (scanf("%lf",&amount) != 1)
    {
        while(getchar()!='\n');
        printf("Invalid input for amount, please enter valid amount.\n");
        printf("Enter the deposit amount: ");
    }

    while(amount > 10000)
    {
        printf("Transaction is not completed, the maximum deposit limit for each transaction is 10,000$, please try again.\n");
        printf("Enter the deposit amount: ");
        scanf("%lf", &amount);
    }
    
    account.balance += amount;
    int ed = edit(db,account);
    if (ed == 1)
    {
        printf("Transaction is completed successfully\n");
        Menu(db);
    }
}
