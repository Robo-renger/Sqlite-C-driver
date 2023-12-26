#include "functions.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <stddef.h>
#include <time.h>

#define MAX_LENGTH 100

long generateAccountNumber(struct Account *account)
{

    int year = account->date_opened.year;
    int month = account->date_opened.month;
    int id = account->id;

    printf("Debug: year=%d, month=%d, id=%d\n", year, month, id);

    long uniqueNumber = ((long)year * 10000000000) + ((long)month * 100000000) + id;

    return uniqueNumber;
}

void getCurrentDate(struct Date *currentDate)
{
    time_t t = time(NULL);
    struct tm *localTime = localtime(&t);

    if (localTime != NULL)
    {
        currentDate->month = localTime->tm_mon + 1;
        currentDate->year = localTime->tm_year + 1900;
    }
    else
    {
        fprintf(stderr, "Error getting current date.\n");
        exit(EXIT_FAILURE);
    }
}

void freeEntityList(struct EntityList *entityList)
{
    if (entityList->entities != NULL)
    {
        for (size_t i = 0; i < entityList->size; ++i)
        {
            if (entityList->entities[i].entity_type == ACCOUNT)
            {
                if (entityList->entities[i].account.name != NULL)
                {
                    free(entityList->entities[i].account.name);
                }
                if (entityList->entities[i].account.mobile != NULL)
                {
                    free(entityList->entities[i].account.mobile);
                }
                if (entityList->entities[i].account.email_address != NULL)
                {
                    free(entityList->entities[i].account.email_address);
                }
            }
        }
        free(entityList->entities);
    }
}

void createAccount(sqlite3 *db)
{
    char name[MAX_LENGTH];
    char mobile[MAX_LENGTH];
    char email[MAX_LENGTH];
    char date[MAX_LENGTH];
    double balance;
    struct Date currentDate;

    getCurrentDate(&currentDate);
    
    
    printf("Enter name of the account: ");
    if (fgets(name, sizeof(name), stdin) == NULL)
    {

        fprintf(stderr, "Error reading input.\n");
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(name);
    if (len > 0 && name[len - 1] == '\n')
    {
        name[len - 1] = '\0';
    }

    printf("Enter mobile: ");
    if (fgets(mobile, sizeof(mobile), stdin) == NULL)
    {
        fprintf(stderr, "Error reading input.\n");
        exit(EXIT_FAILURE);
    }
    len = strlen(mobile);
    if (len > 0 && mobile[len - 1] == '\n')
    {
        mobile[len - 1] = '\0';
    }

    printf("Enter email: ");
    if (fgets(email, sizeof(email), stdin) == NULL)
    {
        fprintf(stderr, "Error reading input.\n");
        exit(EXIT_FAILURE);
    }
    len = strlen(email);
    if (len > 0 && email[len - 1] == '\n')
    {
        email[len - 1] = '\0';
    }

    printf("Enter balance: ");
    if (scanf("%lf", &balance) != 1)
    {
        fprintf(stderr, "Invalid input for balance.\n");
        exit(EXIT_FAILURE);
    }

    printf("Current Date: %d-%02d\n", currentDate.month, currentDate.year);
    struct Entity accountEntity;
    accountEntity.entity_type = ACCOUNT;
    accountEntity.account.name = name;
    accountEntity.account.mobile = mobile;
    accountEntity.account.email_address = email;
    accountEntity.account.balance = balance;
    accountEntity.account.date_opened = currentDate;
    insert(db, accountEntity);
    struct Account lastAccount = getLastInsertedAccount(db);
    lastAccount.account_number = generateAccountNumber(&lastAccount);
    printf("%ld", lastAccount.account_number);
    
    edit(db, lastAccount);
    /*
    struct Account accounts[] = {lastAccount};
    struct EntityList accountList[] = {get(db, lastAccount.id, ACCOUNT)};
    Save(db, accounts, accountList, 1);
    */
}

void getAllTransactions(sqlite3 *db)
{
    struct EntityList transactionList = getAll(db, TRANSACTION);

    printf("Transaction entities:\n");
    for (size_t i = 0; i < transactionList.size; ++i)
    {
        printf("Transaction ID: %d, Account ID: %d,Account Number: %ld,Price: %lf\n",
               transactionList.entities[i].transaction.id,
               transactionList.entities[i].transaction.account_id,
               transactionList.entities[i].transaction.account_number,
               transactionList.entities[i].transaction.price);
    }

    freeEntityList(&transactionList);
}

void getAllAccounts(sqlite3 *db)
{

    struct EntityList accountList = getAll(db, ACCOUNT);

    for (size_t i = 0; i < accountList.size; ++i)
    {
        printf("%d,%s,%s,%s,%d\n",
               accountList.entities[i].account.id,
               accountList.entities[i].account.name,
               accountList.entities[i].account.mobile,
               accountList.entities[i].account.email_address,
               accountList.entities[i].account.balance);
    }

    freeEntityList(&accountList);

    sqlite3_close(db);
}

void Menu(sqlite3 *db)
{
    struct EntityList accountList;

    int choice;
    printf("Choose an option:\n");
    printf("1. Add\n");
    printf("2. Delete\n");
    printf("3. Modify\n");
    printf("4. Search\n");
    printf("5. Advanced Search\n");
    printf("6. Withdraw\n");
    printf("7. Deposit\n");
    printf("8. Transfer\n");
    printf("9. Report\n");
    printf("10. Print\n");
    printf("11. Quit\n");
    printf("Enter the number corresponding to your choice: ");
    if (scanf("%d", &choice) != 1)
    {
        printf("Invalid input. Please enter a valid integer.\n");
        while (getchar() != '\n')
            ;
        Menu(db);
    }

    switch (choice)
    {
    case 1:
        createAccount(db);
        break;
    case 4:
        regularSearch(db);
        break;
    case 5:
        advancedSearch(db);
        break;
    case 6:
        Withdraw(db);
        break;
    case 7:
        Deposit(db);
        break;
    case 8:
        Transfer(db);
        break;
    case 10:
        accountList = getAll(db, ACCOUNT);
        Print(&accountList, db);
        freeEntityList(&accountList);
        break;
    case 11:
        exit(1);
    default:
        printf("Invalid choice, please try again.\n");
        Menu(db);
    }
}

void Save(sqlite3 *db, struct Account *accounts, struct EntityList *accountsList, int numberOfAccounts)
{
    int choice;
    if (numberOfAccounts == 0)
        Menu(db);

    printf("Process is successfully done and awaiting for you to save changes\n");
    printf("1. Confirm changes\n");
    printf("2. Discard changes\n");
    printf("Enter the number corresponding to your choice: ");
    if (scanf("%d", &choice) != 1)
    {
        printf("Invalid input. Please enter a valid integer.\n");
        while (getchar() != '\n')
            ;
        Save(db, accounts, accountsList, numberOfAccounts);
    }

    switch (choice)
    {
    case 1:
        for (int i = 0; i < numberOfAccounts; ++i)
        {
            if (edit(db, accounts[i]) != 1)
            {
                printf("Failed to save changes for account %d.\n", i + 1);
                freeEntityList(&accountsList[i]);
            }
        }
        printf("Changes saved successfully.\n");
        Menu(db);
        break;
    case 2:
        for (int i = 0; i < numberOfAccounts; ++i)
            freeEntityList(&accountsList[i]);

        printf("Changes discarded.\n");
        Menu(db);
        break;
    default:
        printf("Invalid choice, please try again.\n");
        Save(db, accounts, accountsList, numberOfAccounts);
    }
}

void Withdraw(sqlite3 *db)
{
    long account_number;
    double amount;

    printf("Please, enter your account number:");
    while (scanf("%ld", &account_number) != 1)
    {
        while (getchar() != '\n')
            ;
        printf("Invalid input for account number, please enter a valid number.\n");
        printf("Please, enter your account number:");
    }

    struct EntityList accountList = get(db, account_number, ACCOUNT);
    if (accountList.size == 0)
    {
        printf("Account number not found, please try again.\n");
        Withdraw(db);
    }

    struct Account account = accountList.entities[0].account;
    if (account.balance == 0)
    {
        printf("Transaction will not proceed as your balance is ZERO\n");
        Menu(db);
    }

    printf("Enter the withdrawal amount: ");
    while (scanf("%lf", &amount) != 1)
    {
        while (getchar() != '\n')
            ;
        printf("Invalid input for amount, please enter valid amount.\n");
        printf("Enter the withdrawal amount: ");
    }

    while (amount > 10000)
    {
        printf("Transaction is not completed, the maximum withdrawal limit for each transaction is 10,000$, please try again.\n");
        printf("Enter the withdrawal amount: ");
        scanf("%lf", &amount);
    }

    while (amount > account.balance)
    {
        printf("Withdrawal amount exceeds your current balance. Please enter a valid amount.\n");
        printf("Enter the withdrawal amount: ");
        scanf("%lf", &amount);
    }
    account.balance -= amount;

    Save(db, &account, &accountList, 1);
}

void Deposit(sqlite3 *db)
{
    long account_number;
    double amount;

    printf("Please, enter your account number:");
    while (scanf("%ld", &account_number) != 1)
    {
        while (getchar() != '\n')
            ;
        printf("Invalid input for account number, please enter a valid number.\n");
        printf("Please, enter your account number:");
    }

    struct EntityList accountList = get(db, account_number, ACCOUNT);
    if (accountList.size == 0)
    {
        printf("Account number not found, please try again.\n");
        Deposit(db);
    }

    struct Account account = accountList.entities[0].account;

    printf("Enter the deposit amount: ");
    while (scanf("%lf", &amount) != 1)
    {
        while (getchar() != '\n')
            ;
        printf("Invalid input for amount, please enter valid amount.\n");
        printf("Enter the deposit amount: ");
    }

    while (amount > 10000)
    {
        printf("Transaction is not completed, the maximum deposit limit for each transaction is 10,000$, please try again.\n");
        printf("Enter the deposit amount: ");
        scanf("%lf", &amount);
    }
    account.balance += amount;

    Save(db, &account, &accountList, 1);
}

void Transfer(sqlite3 *db)
{
    int sender_account_number, receiver_account_number;
    double amount;

    struct EntityList senderAccountList;
    do
    {
        printf("Please, enter sender account number:");

        while (scanf("%d", &sender_account_number) != 1)
        {
            while (getchar() != '\n')
                ;
            printf("Invalid input for account number, please enter a valid number.\n");
            printf("Please, enter sender account number:");
        }

        senderAccountList = get(db, sender_account_number, ACCOUNT);
        if (senderAccountList.size == 0)
            printf("Sender's account number not found, please try again.\n");

    } while (senderAccountList.size == 0);

    struct EntityList receiverAccountList;
    do
    {
        printf("Please, enter receiver account number:");

        while (scanf("%d", &receiver_account_number) != 1)
        {
            while (getchar() != '\n')
                ;
            printf("Invalid input for account number, please enter a valid number.\n");
            printf("Please, enter receiver account number:");
        }

        receiverAccountList = get(db, receiver_account_number, ACCOUNT);
        if (receiverAccountList.size == 0)
            printf("Receiver's account number not found, please try again.\n");

        if (sender_account_number == receiver_account_number)
            printf("Receiver's account number is the same as the sender account number, please try again.\n");

    } while (receiverAccountList.size == 0 || sender_account_number == receiver_account_number);

    struct Account sender = senderAccountList.entities[0].account;
    struct Account receiver = receiverAccountList.entities[0].account;
    if (sender.balance == 0)
    {
        printf("Transaction will not proceed as your balance is ZERO\n");
        Menu(db);
    }

    printf("Enter the amount you want to transfer: ");
    while (scanf("%lf", &amount) != 1)
    {
        while (getchar() != '\n')
            ;
        printf("Invalid input for amount, please enter valid amount.\n");
        printf("Enter the withdrawal amount: ");
    }

    while (amount > sender.balance)
    {
        printf("Transfer amount exceeds your current balance. Please enter a valid amount.\n");
        printf("Enter the amount you want to transfer: ");
        scanf("%lf", &amount);
    }

    sender.balance -= amount;
    receiver.balance += amount;

    struct Account accounts[] = {sender, receiver};
    struct EntityList accountsList[] = {senderAccountList, receiverAccountList};
    Save(db, accounts, accountsList, 2);
}

void Print(struct EntityList *entityList, sqlite3 *db)
{
    int choice;
    printf("Choose a sorting option:\n");
    printf("1. Sort by name\n");
    printf("2. Sort by balance\n");
    printf("3. Sort by date\n");
    printf("Enter the number corresponding to your choice: ");
    if (scanf("%d", &choice) != 1)
    {
        printf("Invalid input. Please enter a valid integer.\n");
        while (getchar() != '\n')
            ;
        Print(entityList, db);
    }

    switch (choice)
    {
    case 1:
        SortByName(entityList, db);
        break;
    case 2:
        SortByBalance(entityList, db);
        break;
    case 3:
        SortByDate(entityList, db);
        break;
    default:
        printf("Invalid choice, please try again.\n");
        Print(entityList, db);
    }
}

void SortByName(struct EntityList *entityList, sqlite3 *db)
{
    for (int i = 0; i < entityList->size - 1; i++)
    {
        for (int j = 0; j < entityList->size - i - 1; j++)
        {
            if (strcmp(entityList->entities[j].account.name, entityList->entities[j + 1].account.name) > 0)
            {
                struct Entity temp = entityList->entities[j + 1];
                entityList->entities[j + 1] = entityList->entities[j];
                entityList->entities[j] = temp;
            }
        }
    }

    printf("Sorted Account List (Sorted by Name):\n");
    for (size_t i = 0; i < entityList->size; i++)
    {
        printf("Account Number: %d\n", entityList->entities[i].account.id);
        printf("Name: %s\n", entityList->entities[i].account.name);
        printf("Email: %s\n", entityList->entities[i].account.email_address);
        printf("Balance: %d\n", entityList->entities[i].account.balance);
        printf("Mobile: %s\n", entityList->entities[i].account.mobile);
        const char *months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
        printf("Date Opened: %s %d\n", months[entityList->entities[i].account.date_opened.month - 1], entityList->entities[i].account.date_opened.year);
        printf("\n");
    }
    Menu(db);
}

void SortByBalance(struct EntityList *entityList, sqlite3 *db)
{
    for (int i = 0; i < entityList->size - 1; i++)
    {
        for (int j = 0; j < entityList->size - i - 1; j++)
        {
            if (entityList->entities[j].account.balance > entityList->entities[j + 1].account.balance)
            {
                struct Entity temp = entityList->entities[j + 1];
                entityList->entities[j + 1] = entityList->entities[j];
                entityList->entities[j] = temp;
            }
        }
    }

    printf("Sorted Account List (Sorted by Balance):\n");
    for (size_t i = 0; i < entityList->size; i++)
    {
        printf("Account Number: %d\n", entityList->entities[i].account.id);
        printf("Name: %s\n", entityList->entities[i].account.name);
        printf("Email: %s\n", entityList->entities[i].account.email_address);
        printf("Balance: %d\n", entityList->entities[i].account.balance);
        printf("Mobile: %s\n", entityList->entities[i].account.mobile);
        const char *months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
        printf("Date Opened: %s %d\n", months[entityList->entities[i].account.date_opened.month - 1], entityList->entities[i].account.date_opened.year);
        printf("\n");
    }
    Menu(db);
}

void SortByDate(struct EntityList *entityList, sqlite3 *db)
{
    for (int i = 0; i < entityList->size - 1; i++)
    {
        for (int j = 0; j < entityList->size - i - 1; j++)
        {
            int date1 = entityList->entities[j].account.date_opened.year * 10 +
                        entityList->entities[j].account.date_opened.month;

            int date2 = entityList->entities[j + 1].account.date_opened.year * 10 +
                        entityList->entities[j + 1].account.date_opened.month;

            if (date1 > date2)
            {
                struct Entity temp = entityList->entities[j + 1];
                entityList->entities[j + 1] = entityList->entities[j];
                entityList->entities[j] = temp;
            }
        }
    }

    printf("Sorted Account List (Sorted by Date):\n");
    for (size_t i = 0; i < entityList->size; i++)
    {
        printf("Account Number: %d\n", entityList->entities[i].account.id);
        printf("Name: %s\n", entityList->entities[i].account.name);
        printf("Email: %s\n", entityList->entities[i].account.email_address);
        printf("Balance: %d\n", entityList->entities[i].account.balance);
        printf("Mobile: %s\n", entityList->entities[i].account.mobile);

        const char *months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
        printf("Date Opened: %s %d\n", months[entityList->entities[i].account.date_opened.month - 1], entityList->entities[i].account.date_opened.year);
        printf("\n");
    }
    Menu(db);
}

int loginUser(sqlite3 *db)
{
    char username[MAX_LENGTH];
    char password[MAX_LENGTH];

    struct User user;

    printf("Please Enter your username: ");
    if (fgets(username, sizeof(username), stdin) == NULL)
    {
        fprintf(stderr, "Error reading input.\n");
        exit(EXIT_FAILURE);
    }
    size_t len = strlen(username);
    if (len > 0 && username[len - 1] == '\n')
    {
        username[len - 1] = '\0';
    }
    printf("Please Enter your password: ");
    if (fgets(password, sizeof(password), stdin) == NULL)
    {
        fprintf(stderr, "Error reading input.\n");
        exit(EXIT_FAILURE);
    }
    len = strlen(password);
    if (len > 0 && password[len - 1] == '\n')
    {
        password[len - 1] = '\0';
    }
    user.username = username;
    user.password = password;
    if (login(db, user))
    {
        printf("Logged in successfuly\n");
        Menu(db);
    }
    else
    {
        printf("Invalid uername or password\n");
        loginUser(db);
    }
}

void advancedSearch(sqlite3 *db)
{
    char keyword[MAX_LENGTH];

    printf("Please Enter a keyword: ");
    if (fgets(keyword, sizeof(keyword), stdin) == NULL)
    {
        fprintf(stderr, "Error reading input.\n");
        exit(EXIT_FAILURE);
    }
    size_t len = strlen(keyword);
    if (len > 0 && keyword[len - 1] == '\n')
    {
        keyword[len - 1] = '\0';
    }
    struct EntityList accountList = searchAccounts(db, keyword);

    if (accountList.size == 0)
    {
        printf("No matching accounts found.\n");
    }
    else
    {

        for (size_t i = 0; i < accountList.size; ++i)
        {
            struct Entity entity = accountList.entities[i];

            if (entity.entity_type == ACCOUNT)
            {
                printf("Account ID: %d\n", entity.account.id);
                printf("Account Number: %ld\n", entity.account.account_number);
                printf("Name: %s\n", entity.account.name);
                printf("Mobile: %s\n", entity.account.mobile);
                printf("Email Address: %s\n", entity.account.email_address);
                printf("Balance: %d\n", entity.account.balance);
                printf("Date Opened: %d-%d\n", entity.account.date_opened.month, entity.account.date_opened.year);
                printf("\n");
            }
        }

        for (size_t i = 0; i < accountList.size; ++i)
        {
            if (accountList.entities[i].entity_type == ACCOUNT)
            {
                free(accountList.entities[i].account.name);
                free(accountList.entities[i].account.mobile);
                free(accountList.entities[i].account.email_address);
            }
        }
        free(accountList.entities);
    }
}

void regularSearch(sqlite3 *db)
{
    char column[MAX_LENGTH];
    char keyword[MAX_LENGTH];
    printf("Please Enter the column's name: ");
    if (fgets(column, sizeof(column), stdin) == NULL)
    {
        fprintf(stderr, "Error reading input.\n");
        exit(EXIT_FAILURE);
    }
    size_t columnLen = strlen(column);
    if (columnLen > 0 && column[columnLen - 1] == '\n')
    {
        column[columnLen - 1] = '\0';
    }
    printf("Please Enter a keyword: ");
    if (fgets(keyword, sizeof(keyword), stdin) == NULL)
    {
        fprintf(stderr, "Error reading input.\n");
        exit(EXIT_FAILURE);
    }
    size_t keywordLen = strlen(keyword);
    if (keywordLen > 0 && keyword[keywordLen - 1] == '\n')
    {
        keyword[keywordLen - 1] = '\0';
    }

    struct EntityList accountList = searchColumn(db, column, keyword);

    if (accountList.size == 0)
    {
        printf("No matching accounts found.\n");
    }
    else
    {

        for (size_t i = 0; i < accountList.size; ++i)
        {
            struct Entity entity = accountList.entities[i];

            if (entity.entity_type == ACCOUNT)
            {
                printf("Account ID: %d\n", entity.account.id);
                printf("Account Number: %ld\n", entity.account.account_number);
                printf("Name: %s\n", entity.account.name);
                printf("Mobile: %s\n", entity.account.mobile);
                printf("Email Address: %s\n", entity.account.email_address);
                printf("Balance: %d\n", entity.account.balance);
                printf("Date Opened: %d-%d\n", entity.account.date_opened.month, entity.account.date_opened.year);
                printf("\n");
            }
        }

        for (size_t i = 0; i < accountList.size; ++i)
        {
            if (accountList.entities[i].entity_type == ACCOUNT)
            {
                free(accountList.entities[i].account.name);
                free(accountList.entities[i].account.mobile);
                free(accountList.entities[i].account.email_address);
            }
        }
        free(accountList.entities);
    }
}
