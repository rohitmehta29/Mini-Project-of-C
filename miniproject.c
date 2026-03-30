#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Simple encryption function (XOR with key)
void encrypt(char *data, int key) {
    for (int i = 0; data[i] != '\0'; i++) {
        data[i] ^= key;
    }
}

void decrypt(char *data, int key) {
    encrypt(data, key); // XOR is symmetric
}

#define MAX_ACCOUNTS 100
#define MAX_TRANSACTIONS 1000
#define ENCRYPTION_KEY 42

typedef struct {
    int id;
    char type[20]; // deposit, withdrawal, transfer, interest
    double amount;
    char date[20];
    char description[100];
} Transaction;

typedef struct {
    int accountNumber;
    char name[50];
    char password[20];
    double balance;
    char currency[4]; // USD, EUR, INR, etc.
    Transaction transactions[MAX_TRANSACTIONS];
    int transactionCount;
} Account;

Account accounts[MAX_ACCOUNTS];
int accountCount = 0;
int nextAccountNumber = 1001;

// Function to create account
void createAccount() {
    if (accountCount >= MAX_ACCOUNTS) {
        printf("Maximum accounts reached.\n");
        return;
    }

    Account newAccount;
    newAccount.accountNumber = nextAccountNumber++;
    printf("Enter name: ");
    scanf("%s", newAccount.name);
    printf("Enter password: ");
    scanf("%s", newAccount.password);
    printf("Enter initial balance: ");
    scanf("%lf", &newAccount.balance);
    printf("Enter currency (USD/EUR/INR): ");
    scanf("%s", newAccount.currency);
    newAccount.transactionCount = 0;

    accounts[accountCount++] = newAccount;
    printf("Account created successfully. Account Number: %d\n", newAccount.accountNumber);
}

// Function to login
int login(int *accountIndex) {
    int accNum;
    char pass[20];
    printf("Enter account number: ");
    scanf("%d", &accNum);
    printf("Enter password: ");
    scanf("%s", pass);

    for (int i = 0; i < accountCount; i++) {
        if (accounts[i].accountNumber == accNum && strcmp(accounts[i].password, pass) == 0) {
            *accountIndex = i;
            return 1;
        }
    }
    return 0;
}

// Function to deposit money
void deposit(int accountIndex) {
    double amount;
    printf("Enter amount to deposit: ");
    scanf("%lf", &amount);
    if (amount > 0) {
        accounts[accountIndex].balance += amount;
        // Add transaction
        Transaction t;
        t.id = accounts[accountIndex].transactionCount + 1;
        strcpy(t.type, "deposit");
        t.amount = amount;
        time_t now = time(NULL);
        strftime(t.date, sizeof(t.date), "%Y-%m-%d %H:%M:%S", localtime(&now));
        sprintf(t.description, "Deposit of %.2f %s", amount, accounts[accountIndex].currency);
        accounts[accountIndex].transactions[accounts[accountIndex].transactionCount++] = t;
        printf("Deposit successful. New balance: %.2f %s\n", accounts[accountIndex].balance, accounts[accountIndex].currency);
    } else {
        printf("Invalid amount.\n");
    }
}

// Function to withdraw money
void withdraw(int accountIndex) {
    double amount;
    printf("Enter amount to withdraw: ");
    scanf("%lf", &amount);
    if (amount > 0 && amount <= accounts[accountIndex].balance) {
        accounts[accountIndex].balance -= amount;
        // Add transaction
        Transaction t;
        t.id = accounts[accountIndex].transactionCount + 1;
        strcpy(t.type, "withdrawal");
        t.amount = -amount;
        time_t now = time(NULL);
        strftime(t.date, sizeof(t.date), "%Y-%m-%d %H:%M:%S", localtime(&now));
        sprintf(t.description, "Withdrawal of %.2f %s", amount, accounts[accountIndex].currency);
        accounts[accountIndex].transactions[accounts[accountIndex].transactionCount++] = t;
        printf("Withdrawal successful. New balance: %.2f %s\n", accounts[accountIndex].balance, accounts[accountIndex].currency);
    } else {
        printf("Invalid amount or insufficient balance.\n");
    }
}

// Function to transfer money
void transfer(int accountIndex) {
    int toAccNum;
    double amount;
    printf("Enter recipient account number: ");
    scanf("%d", &toAccNum);
    printf("Enter amount to transfer: ");
    scanf("%lf", &amount);

    int toIndex = -1;
    for (int i = 0; i < accountCount; i++) {
        if (accounts[i].accountNumber == toAccNum) {
            toIndex = i;
            break;
        }
    }

    if (toIndex == -1) {
        printf("Recipient account not found.\n");
        return;
    }

    if (amount > 0 && amount <= accounts[accountIndex].balance) {
        accounts[accountIndex].balance -= amount;
        accounts[toIndex].balance += amount;

        // Add transaction for sender
        Transaction t1;
        t1.id = accounts[accountIndex].transactionCount + 1;
        strcpy(t1.type, "transfer");
        t1.amount = -amount;
        time_t now = time(NULL);
        strftime(t1.date, sizeof(t1.date), "%Y-%m-%d %H:%M:%S", localtime(&now));
        sprintf(t1.description, "Transfer to account %d: %.2f %s", toAccNum, amount, accounts[accountIndex].currency);
        accounts[accountIndex].transactions[accounts[accountIndex].transactionCount++] = t1;

        // Add transaction for receiver
        Transaction t2;
        t2.id = accounts[toIndex].transactionCount + 1;
        strcpy(t2.type, "transfer");
        t2.amount = amount;
        strftime(t2.date, sizeof(t2.date), "%Y-%m-%d %H:%M:%S", localtime(&now));
        sprintf(t2.description, "Transfer from account %d: %.2f %s", accounts[accountIndex].accountNumber, amount, accounts[toIndex].currency);
        accounts[toIndex].transactions[accounts[toIndex].transactionCount++] = t2;

        printf("Transfer successful.\n");
    } else {
        printf("Invalid amount or insufficient balance.\n");
    }
}

// Function for international transfer (simplified with fixed rates)
void internationalTransfer(int accountIndex) {
    int toAccNum;
    double amount;
    char toCurrency[4];
    printf("Enter recipient account number: ");
    scanf("%d", &toAccNum);
    printf("Enter amount to transfer: ");
    scanf("%lf", &amount);
    printf("Enter recipient currency (USD/EUR/INR): ");
    scanf("%s", toCurrency);

    int toIndex = -1;
    for (int i = 0; i < accountCount; i++) {
        if (accounts[i].accountNumber == toAccNum) {
            toIndex = i;
            break;
        }
    }

    if (toIndex == -1) {
        printf("Recipient account not found.\n");
        return;
    }

    // Simple conversion rates (example)
    double rate = 1.0;
    if (strcmp(accounts[accountIndex].currency, "USD") == 0 && strcmp(toCurrency, "EUR") == 0) rate = 0.85;
    else if (strcmp(accounts[accountIndex].currency, "USD") == 0 && strcmp(toCurrency, "INR") == 0) rate = 83.0;
    else if (strcmp(accounts[accountIndex].currency, "EUR") == 0 && strcmp(toCurrency, "USD") == 0) rate = 1.18;
    else if (strcmp(accounts[accountIndex].currency, "EUR") == 0 && strcmp(toCurrency, "INR") == 0) rate = 88.0;
    else if (strcmp(accounts[accountIndex].currency, "INR") == 0 && strcmp(toCurrency, "USD") == 0) rate = 0.012;
    else if (strcmp(accounts[accountIndex].currency, "INR") == 0 && strcmp(toCurrency, "EUR") == 0) rate = 0.011;

    double convertedAmount = amount * rate;

    if (amount > 0 && amount <= accounts[accountIndex].balance) {
        accounts[accountIndex].balance -= amount;
        accounts[toIndex].balance += convertedAmount;

        // Add transaction for sender
        Transaction t1;
        t1.id = accounts[accountIndex].transactionCount + 1;
        strcpy(t1.type, "international_transfer");
        t1.amount = -amount;
        time_t now = time(NULL);
        strftime(t1.date, sizeof(t1.date), "%Y-%m-%d %H:%M:%S", localtime(&now));
        sprintf(t1.description, "International transfer to account %d: %.2f %s -> %.2f %s", toAccNum, amount, accounts[accountIndex].currency, convertedAmount, toCurrency);
        accounts[accountIndex].transactions[accounts[accountIndex].transactionCount++] = t1;

        // Add transaction for receiver
        Transaction t2;
        t2.id = accounts[toIndex].transactionCount + 1;
        strcpy(t2.type, "international_transfer");
        t2.amount = convertedAmount;
        strftime(t2.date, sizeof(t2.date), "%Y-%m-%d %H:%M:%S", localtime(&now));
        sprintf(t2.description, "International transfer from account %d: %.2f %s", accounts[accountIndex].accountNumber, convertedAmount, toCurrency);
        accounts[toIndex].transactions[accounts[toIndex].transactionCount++] = t2;

        printf("International transfer successful. Converted amount: %.2f %s\n", convertedAmount, toCurrency);
    } else {
        printf("Invalid amount or insufficient balance.\n");
    }
}

// Function to calculate interest (simple interest)
void calculateInterest(int accountIndex) {
    double rate = 0.05; // 5% annual interest
    double interest = accounts[accountIndex].balance * rate / 12; // monthly
    accounts[accountIndex].balance += interest;

    // Add transaction
    Transaction t;
    t.id = accounts[accountIndex].transactionCount + 1;
    strcpy(t.type, "interest");
    t.amount = interest;
    time_t now = time(NULL);
    strftime(t.date, sizeof(t.date), "%Y-%m-%d %H:%M:%S", localtime(&now));
    sprintf(t.description, "Interest credited: %.2f %s", interest, accounts[accountIndex].currency);
    accounts[accountIndex].transactions[accounts[accountIndex].transactionCount++] = t;

    printf("Interest calculated and added. New balance: %.2f %s\n", accounts[accountIndex].balance, accounts[accountIndex].currency);
}

// Function to view transaction history
void viewTransactionHistory(int accountIndex) {
    printf("Transaction History for Account %d:\n", accounts[accountIndex].accountNumber);
    for (int i = 0; i < accounts[accountIndex].transactionCount; i++) {
        Transaction t = accounts[accountIndex].transactions[i];
        printf("%d. %s - %.2f - %s - %s\n", t.id, t.type, t.amount, t.date, t.description);
    }
}

// Function to save data with encryption
void saveData() {
    FILE *file = fopen("bank_data.enc", "wb");
    if (file == NULL) {
        printf("Error saving data.\n");
        return;
    }

    // Encrypt and save
    char buffer[sizeof(accounts)];
    memcpy(buffer, accounts, sizeof(accounts));
    encrypt(buffer, ENCRYPTION_KEY);
    fwrite(buffer, sizeof(buffer), 1, file);

    char countBuffer[sizeof(accountCount)];
    memcpy(countBuffer, &accountCount, sizeof(accountCount));
    encrypt(countBuffer, ENCRYPTION_KEY);
    fwrite(countBuffer, sizeof(countBuffer), 1, file);

    fclose(file);
    printf("Data saved securely.\n");
}

// Function to load data with decryption
void loadData() {
    FILE *file = fopen("bank_data.enc", "rb");
    if (file == NULL) {
        printf("No saved data found.\n");
        return;
    }

    char buffer[sizeof(accounts)];
    fread(buffer, sizeof(buffer), 1, file);
    decrypt(buffer, ENCRYPTION_KEY);
    memcpy(accounts, buffer, sizeof(accounts));

    char countBuffer[sizeof(accountCount)];
    fread(countBuffer, sizeof(countBuffer), 1, file);
    decrypt(countBuffer, ENCRYPTION_KEY);
    memcpy(&accountCount, countBuffer, sizeof(accountCount));

    fclose(file);
    printf("Data loaded securely.\n");
}

int main() {
    loadData();

    int choice;
    int loggedIn = 0;
    int accountIndex = -1;

    while (1) {
        if (!loggedIn) {
            printf("\nBank Simulation Menu:\n");
            printf("1. Create Account\n");
            printf("2. Login\n");
            printf("3. Exit\n");
            printf("Enter choice: ");
            scanf("%d", &choice);

            switch (choice) {
                case 1:
                    createAccount();
                    break;
                case 2:
                    if (login(&accountIndex)) {
                        loggedIn = 1;
                        printf("Login successful.\n");
                    } else {
                        printf("Invalid credentials.\n");
                    }
                    break;
                case 3:
                    saveData();
                    return 0;
                default:
                    printf("Invalid choice.\n");
            }
        } else {
            printf("\nAccount Menu:\n");
            printf("1. Deposit Money\n");
            printf("2. Withdraw Money\n");
            printf("3. Transfer Money\n");
            printf("4. International Transfer\n");
            printf("5. Calculate Interest\n");
            printf("6. View Transaction History\n");
            printf("7. Logout\n");
            printf("Enter choice: ");
            scanf("%d", &choice);

            switch (choice) {
                case 1:
                    deposit(accountIndex);
                    break;
                case 2:
                    withdraw(accountIndex);
                    break;
                case 3:
                    transfer(accountIndex);
                    break;
                case 4:
                    internationalTransfer(accountIndex);
                    break;
                case 5:
                    calculateInterest(accountIndex);
                    break;
                case 6:
                    viewTransactionHistory(accountIndex);
                    break;
                case 7:
                    loggedIn = 0;
                    accountIndex = -1;
                    printf("Logged out.\n");
                    break;
                default:
                    printf("Invalid choice.\n");
            }
        }
    }

    return 0;
}
