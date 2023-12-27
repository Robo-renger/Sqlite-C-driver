#include "functions.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>
#include <stddef.h>
#include <time.h>

#define MAX_LENGTH 100

long generateAccountNumber(struct Account *account)
{

    int year = account->date_opened.year;
    int month = account->date_opened.month;
    int id = account->id;

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

    getchar();
    do
    {
        printf("Enter name of the account: ");
        if (fgets(name, sizeof(name), stdin) == NULL)
        {
            fprintf(stderr, "\nError reading input.\n");
            printf("Please Input valid Name \n");
        }

        size_t len = strlen(name);
        if (len > 0 && name[len - 1] == '\n')
        {
            name[len - 1] = '\0';
        }

        if (isBlank(name))
            printf("Name cannot be blank. Please enter a valid name.\n");
        else if(!isValidName(name))
            printf("Invalid name. Name should consist of letters only.\n");
        
    }while(isBlank(name) || !isValidName(name));

    do
    {
        printf("Enter mobile: ");
        if (fgets(mobile, sizeof(mobile), stdin) == NULL)
        {
            fprintf(stderr, "\nError reading input.\n");
            printf("Please Input valid Mobile \n");
        }
        size_t len = strlen(mobile);
        if (len > 0 && mobile[len - 1] == '\n')
        {
            mobile[len - 1] = '\0';
        }
        
        if (isBlank(mobile))
            printf("Mobile number cannot be blank. Please enter a valid mobile number.\n");
        else if(!isValidPhoneNumber(mobile))
            printf("Invalid phone number. Phone number should consist of 11 integers.\n");
    }while(isBlank(mobile) || !isValidPhoneNumber(mobile));

    do
    {    
        printf("Enter email: ");
        if (fgets(email, sizeof(email), stdin) == NULL)
        {
            fprintf(stderr, "\nError reading input.\n");
            printf("Please Input valid Email \n");
        }
        size_t len = strlen(email);
        if (len > 0 && email[len - 1] == '\n')
        {
            email[len - 1] = '\0';
        }
        
        if (isBlank(email))
            printf("Email cannot be blank. Please enter a valid email address.\n");
        else if(!isValidEmail(email))
            printf("Invalid email address. Email address should contain '@'.\n");
    }while(isBlank(email) || !isValidEmail(email));

    do
    {
        printf("Enter balance: ");
        while(scanf("%lf", &balance) != 1)
        {
            while (getchar() != '\n');
            printf("Invalid input for balance. Please Input valid balance \n");
            printf("Enter balance: ");
        }
    }while(balance<0);

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
    edit(db, lastAccount);
    
    printf("Account created Successfully");
    Menu(db);
}

void getAllTransactions(sqlite3 *db)
{

    long account_number;

    printf("Please, enter your account number: ");
    while (scanf("%ld", &account_number) != 1)
    {
        while (getchar() != '\n');
        printf("Invalid input for account number, please enter a valid number.\n");
        printf("Please, enter your account number:");
    }

    struct EntityList transactionList = get(db,account_number,TRANSACTION);
    
    if (transactionList.size == 0)
        printf("No transactions found for account %ld.\n", account_number);
    else
    {
        printf("Last 5 transactions for account %ld:\n", account_number);
        size_t numTransactionsToPrint = (transactionList.size < 5) ? transactionList.size : 5;
        for (size_t i = transactionList.size - numTransactionsToPrint; i < transactionList.size; i++)
        {
            printf("Transaction ID: %d, Account Number: %ld, Price: %lf, Type: %s\n",
                   transactionList.entities[i].transaction.id,
                   transactionList.entities[i].transaction.account_number,
                   transactionList.entities[i].transaction.price,
                   transactionList.entities[i].transaction.type);
        }
    }
    freeEntityList(&transactionList);
    Menu(db);
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

int isValidName(const char *name)
{
    while(*name)
    {
        if (!isalpha(*name) && *name != ' ')
        {
            return 0;
        }
        name++;
    }
    return 1;
}

int isValidEmail(const char *email)
{
    while(*email)
    {
        if (*email == '@')
        {
            return 1; 
        }
        email++;
    }
    return 0;
}

int isValidPhoneNumber(const char *mobile)
{
    int count = 0;
    while(*mobile)
    {
        if(isdigit(*mobile))
        {
            count++;
        }
        else if(*mobile != ' ')
        {
            return 0;
        }
        mobile++;
    }
    return count == 11;
}

int isBlank(const char *str)
{
    while (*str)
    {
        if (!isspace((unsigned char)*str))
            return 0;
        str++;
    }
    return 1; 
}

void Menu(sqlite3 *db)
{
    struct EntityList accountList;
    char name[MAX_LENGTH],mobile[MAX_LENGTH],email[MAX_LENGTH];
    long account_number;
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
        while (getchar() != '\n');
        Menu(db);
    }

    switch (choice)
    {
    case 1:
        createAccount(db);
        break;
    case 2:
        printf("Please, enter the account number: ");
        while (scanf("%ld", &account_number) != 1)
        {
            while (getchar() != '\n');
            printf("Invalid input for account number, please enter a valid number.\n");
            printf("Please, enter your account number:");
        }

        accountList = get(db, account_number, ACCOUNT);
        if (accountList.size == 0)
        {
            printf("Account number not found, please try again.\n");
            Menu(db);
        }

        if (accountList.entities[0].account.balance != 0)
        {
            printf("ACCOUNT CANNOT BE DELETED, YOUR BALANCE IS NOT ZERO\n");
            Menu(db);
        }

        if (!delete (db, account_number))
        {
            printf("Account deleted successfully\n");
            Menu(db);
        }

        Menu(db);
        break;
    case 3:
        printf("Please, enter the account number: ");
        while (scanf("%ld", &account_number) != 1)
        {
            while (getchar() != '\n');
            printf("Invalid input for account number, please enter a valid number.\n");
            printf("Please, enter your account number:");
        }
        
        accountList = get(db, account_number, ACCOUNT);
        if (accountList.size == 0)
        {
            printf("Account number not found, please try again.\n");
            Menu(db);
        }

        getchar();
        printf("If you don't want to edit a field just press ENTER\n");
        
        do
        {
            printf("Update Name field:");
            if (fgets(name, sizeof(name), stdin) == NULL)
            {
                fprintf(stderr, "Error reading input.\n");
                exit(EXIT_FAILURE);
            }
            size_t lenn = strlen(name);
            if (lenn > 0 && name[lenn - 1] == '\n')
            {
                name[lenn - 1] = '\0';
            }

            if(isBlank(name))
                break;
            if(!isValidName(name))
                printf("Invalid name. Name should consist of letters only.\n");
        }while(!isValidName(name));

        do
        {
            printf("Update Phone number field:");
            if (fgets(mobile, sizeof(mobile), stdin) == NULL)
            {
                fprintf(stderr, "Error reading input.\n");
                exit(EXIT_FAILURE);
            }
            size_t lenm = strlen(mobile);
            if (lenm > 0 && mobile[lenm - 1] == '\n')
            {
                mobile[lenm - 1] = '\0';
            }

            if(isBlank(mobile))
                break;
            if(!isValidPhoneNumber(mobile))
                printf("Invalid phone number. Phone number should consist of 11 integers.\n");
        }while(!isValidPhoneNumber(mobile));
        
        do
        {
            printf("Update E-mail field:");
            if (fgets(email, sizeof(email), stdin) == NULL)
            {
                fprintf(stderr, "Error reading input.\n");
                exit(EXIT_FAILURE);
            }
            size_t lene = strlen(email);
            if (lene > 0 && email[lene - 1] == '\n')
            {
                email[lene - 1] = '\0';
            }
            
            if(isBlank(email))
                break;
            if(!isValidEmail(email))
                printf("Invalid email address. Email address should contain '@'.\n");
        }while(!isValidEmail(email));

        if (name[0] != '\0')
            strcpy(accountList.entities[0].account.name, name);
        if (email[0] != '\0')
            strcpy(accountList.entities[0].account.email_address, email);
        if (mobile[0] != '\0')
            strcpy(accountList.entities[0].account.mobile, mobile);
        
        struct Date currentDate;
        getCurrentDate(&currentDate);
        accountList.entities[0].account.date_opened = currentDate;

        if(Save(db,&accountList.entities[0].account,&accountList,1))
        {
            printf("Account edited successfully\n");
            Menu(db);
        }
        Menu(db);
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
    case 9:
        getAllTransactions(db);
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

int Save(sqlite3 *db, struct Account *accounts, struct EntityList *accountsList, int numberOfAccounts)
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
        return 1;
    case 2:
        for (int i = 0; i < numberOfAccounts; ++i)
            freeEntityList(&accountsList[i]);

        printf("Changes discarded.\n");
        return 0;
    default:
        printf("Invalid choice, please try again.\n");
        Save(db, accounts, accountsList, numberOfAccounts);
    }
}

void makeTransaction(sqlite3 *db, struct Account account, char *transaction_type, double amount)
{
    struct Entity transaction;

    transaction.transaction.account_number = account.account_number;
    transaction.transaction.price = amount;
    transaction.transaction.type = transaction_type;
    transaction.entity_type = TRANSACTION;
    insert(db, transaction);

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

    struct Date currentDate;
    getCurrentDate(&currentDate);
    account.date_opened = currentDate;

    if (Save(db, &account, &accountList, 1))
        makeTransaction(db, account, "withdraw", amount);
    
    Menu(db);
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

    struct Date currentDate;
    getCurrentDate(&currentDate);
    account.date_opened = currentDate;

    if (Save(db, &account, &accountList, 1))
        makeTransaction(db, account, "deposit", amount);

    Menu(db);
}

void Transfer(sqlite3 *db)
{
    long sender_account_number, receiver_account_number;
    double amount;

    struct EntityList senderAccountList;
    do
    {
        printf("Please, enter sender account number:");

        while (scanf("%ld", &sender_account_number) != 1)
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

        while (scanf("%ld", &receiver_account_number) != 1)
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

    struct Date currentDate;
    getCurrentDate(&currentDate);
    sender.date_opened = currentDate;
    receiver.date_opened = currentDate;

    struct Account accounts[] = {sender, receiver};
    struct EntityList accountsList[] = {senderAccountList, receiverAccountList};
    if (Save(db, accounts, accountsList, 2))
    {
        makeTransaction(db, sender, "transfer-send", amount);
        makeTransaction(db, receiver, "transfer-receive", amount);
    }
    Menu(db);
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
        printf("Account ID: %d\n", entityList->entities[i].account.id);
        printf("Account Number: %ld\n", entityList->entities[i].account.account_number);
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
    getchar();
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
        Menu(db);
        free(accountList.entities);
    }
}

void regularSearch(sqlite3 *db)
{
    char column[MAX_LENGTH];
    char keyword[MAX_LENGTH];
    getchar();
    do{
        printf("Please Enter a valid column (account_number, name, mobile, email_address, balance, date_opened):");

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
    }
    while(strcmp(column, "account_number") != 0 &&
        strcmp(column, "name") != 0 &&
        strcmp(column, "mobile") != 0 &&
        strcmp(column, "email_address") != 0 &&
        strcmp(column, "balance") != 0 &&
        strcmp(column, "date_opened") != 0);

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
        Menu(db);
}
