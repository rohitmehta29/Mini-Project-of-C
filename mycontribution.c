#include <stdio.h>
#include <string.h>

/* ----------- CONSTANTS ----------- */

// Maximum allowed wrong PIN attempts
#define MAX_PIN_TRIES 3

// Admin password (used for authentication)
#define ADMIN_PASS 1234

// Maximum number of accounts
#define MAX 300

/* ----------- ACCOUNT STRUCTURE ----------- */

// Structure to store account details
struct Account {
    int accNo;                 // Account number
    char name[50];             // Account holder name
    char accountType[20];      // Type of account (Savings)
    int pin;                   // Security PIN
    float balance;             // Encrypted balance
    int isLocked;              // Account lock status (1 = locked)
};

/* ----------- GLOBAL VARIABLES ----------- */

struct Account bank[MAX];      // Array to store accounts
int totalAccounts = 0;         // Total number of accounts

/* ----------- FUNCTION: FIND ACCOUNT ----------- */

// Searches account by account number
int findAccount(int accNo) {
    for (int i = 0; i < totalAccounts; i++) {
        if (bank[i].accNo == accNo)
            return i;  // return index if found
    }
    return -1; // return -1 if not found
}

/* ----------- FUNCTION: ADD HISTORY (SIMPLIFIED) ----------- */

// This function would normally store transaction logs
void addHistory(int idx, const char *msg) {
    // For now, just a placeholder
}

/* ----------- FUNCTION: SAVE DATA ----------- */

// Saves data to file (simulation)
void saveToFile() {
    // Placeholder for file saving logic
}

/* ----------- ADMIN PANEL FUNCTION ----------- */

void adminPanel() {
    printf("\n--- Admin Panel ---\n");

    int pass;

    // Ask for admin password
    printf("Enter admin password: ");
    if (scanf("%d", &pass) != 1) {
        while (getchar() != '\n'); // clear buffer
        return;
    }

    // Check password
    if (pass != ADMIN_PASS) {
        printf("Access denied.\n");
        return;
    }

    int choice = 0;

    // Menu loop
    do {
        printf("\n1) View all accounts\n");
        printf("2) Unlock account\n");
        printf("3) Delete account\n");
        printf("4) Bank totals & stats\n");
        printf("5) Reset customer's PIN\n");
        printf("6) Exit admin\n");
        printf("Choice: ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            return;
        }

        // OPTION 1: VIEW ALL ACCOUNTS
        if (choice == 1) {
            printf("\nAccNo | Name | Type | Balance | Locked\n");

            for (int i = 0; i < totalAccounts; i++) {
                printf("%d | %s | %s | %.2f | %s\n",
                       bank[i].accNo,
                       bank[i].name,
                       bank[i].accountType,
                       bank[i].balance,
                       bank[i].isLocked ? "YES" : "NO");
            }
        }

        // OPTION 2: UNLOCK ACCOUNT
        else if (choice == 2) {
            int acc;
            printf("Account to unlock: ");

            if (scanf("%d", &acc) != 1) {
                while (getchar() != '\n');
                continue;
            }

            int i = findAccount(acc);

            if (i == -1) {
                printf("Not found\n");
            } else {
                bank[i].isLocked = 0;
                addHistory(i, "Account unlocked by admin");
                saveToFile();
                printf("Unlocked\n");
            }
        }

        // OPTION 3: DELETE ACCOUNT
        else if (choice == 3) {
            int acc;
            printf("Account to delete: ");

            if (scanf("%d", &acc) != 1) {
                while (getchar() != '\n');
                continue;
            }

            int i = findAccount(acc);

            if (i == -1) {
                printf("Not found\n");
            } else {
                // Shift accounts to delete
                for (int j = i; j < totalAccounts - 1; j++) {
                    bank[j] = bank[j + 1];
                }
                totalAccounts--;

                saveToFile();
                printf("Deleted\n");
            }
        }

        // OPTION 4: TOTAL BANK STATS
        else if (choice == 4) {
            float total = 0.0f;

            for (int i = 0; i < totalAccounts; i++) {
                total += bank[i].balance;
            }

            printf("Total customers: %d\n", totalAccounts);
            printf("Total bank funds: %.2f\n", total);
        }

        // OPTION 5: RESET PIN
        else if (choice == 5) {
            int acc;
            printf("Account to reset PIN: ");

            if (scanf("%d", &acc) != 1) {
                while (getchar() != '\n');
                continue;
            }

            int i = findAccount(acc);

            if (i == -1) {
                printf("Not found\n");
            } else {
                int newpin;

                printf("New PIN: ");
                if (scanf("%d", &newpin) != 1) {
                    while (getchar() != '\n');
                    continue;
                }

                bank[i].pin = newpin;
                addHistory(i, "PIN reset by admin");
                saveToFile();

                printf("PIN reset\n");
            }
        }

    } while (choice != 6);
}

/* ----------- USER MENU FUNCTION ----------- */

void userMenu(int idx) {

    // Check valid index
    if (idx < 0 || idx >= totalAccounts)
        return;

    int tries = 0;
    int pin;

    // Check if account is locked
    if (bank[idx].isLocked) {
        printf("Account is locked. Contact admin.\n");
        return;
    }

    // PIN verification loop
    while (tries < MAX_PIN_TRIES) {
        printf("Enter PIN: ");

        if (scanf("%d", &pin) != 1) {
            while (getchar() != '\n');
            return;
        }

        if (pin == bank[idx].pin)
            break;

        tries++;
        printf("Wrong PIN (%d/%d)\n", tries, MAX_PIN_TRIES);
    }

    // Lock account if max attempts reached
    if (tries == MAX_PIN_TRIES) {
        bank[idx].isLocked = 1;

        addHistory(idx, "Account locked due to failed PIN attempts");
        saveToFile();

        printf("Account locked due to repeated wrong PIN.\n");
        return;
    }

    printf("Login successful! Welcome %s\n", bank[idx].name);
}