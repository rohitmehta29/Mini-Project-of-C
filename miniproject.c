/* final_enterprise_bank.c
   5-star Enterprise Banking System with Advanced Investment Advisor
   Compile: gcc -o bank final_enterprise_bank.c -lm
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX 300
#define FILE_NAME "bank.dat"
#define KEY 54321.0f
#define MAX_TRANS 300
#define MAX_PIN_TRIES 3
#define SAVINGS_INTEREST 4.0f   /* yearly auto credit */
#define MIN_BALANCE 1000.0f
#define ADMIN_PASS 1234

/* --- Account structure --- */
struct Account {
    int accNo;
    char name[50];
    char accountType[20]; /* "Savings" (can extend) */
    int pin;
    float balance; /* encrypted */
    char debitCard[32];
    int cvv;
    int expiryYear;
    int cardBlocked;
    char history[MAX_TRANS][200];
    int transCount;
    int isLocked;
};

struct Account bank[MAX];
int totalAccounts = 0;

/* ---------------- File handling (safe) ---------------- */

void saveToFile() {
    FILE *fp = fopen(FILE_NAME, "wb");
    if (!fp) {
        printf("Warning: could not open data file for saving.\n");
        return;
    }
    fwrite(&totalAccounts, sizeof(int), 1, fp);
    fwrite(bank, sizeof(struct Account), totalAccounts, fp);
    fclose(fp);
}

void loadFromFile() {
    FILE *fp = fopen(FILE_NAME, "rb");
    if (!fp) {
        totalAccounts = 0;
        return;
    }
    if (fread(&totalAccounts, sizeof(int), 1, fp) != 1) {
        totalAccounts = 0;
        fclose(fp);
        return;
    }
    if (totalAccounts < 0 || totalAccounts > MAX) {
        /* file corrupt or tampered */
        totalAccounts = 0;
        fclose(fp);
        return;
    }
    if (fread(bank, sizeof(struct Account), totalAccounts, fp) != (size_t)totalAccounts) {
        /* partial read -> reset */
        totalAccounts = 0;
    }
    fclose(fp);
}

/* ---------------- Simple "encryption" for demo ---------------- */

float encrypt(float val) { return val + KEY; }
float decrypt(float val) { return val - KEY; }

/* ---------------- Utilities ---------------- */

int findAccount(int accNo) {
    for (int i = 0; i < totalAccounts; ++i) {
        if (bank[i].accNo == accNo) return i;
    }
    return -1;
}

void addHistory(int idx, const char *msg) {
    if (idx < 0 || idx >= totalAccounts) return;
    if (bank[idx].transCount < MAX_TRANS) {
        strncpy(bank[idx].history[bank[idx].transCount], msg, 199);
        bank[idx].history[bank[idx].transCount][199] = '\0';
        bank[idx].transCount++;
    } else {
        /* rotate oldest out - optional; here we do simple rotation */
        for (int j = 1; j < MAX_TRANS; ++j)
            strncpy(bank[idx].history[j - 1], bank[idx].history[j], 200);
        strncpy(bank[idx].history[MAX_TRANS - 1], msg, 199);
        bank[idx].history[MAX_TRANS - 1][199] = '\0';
    }
}

/* ---------------- OTP Simulation ---------------- */

int generateOTP() {
    return rand() % 900000 + 100000; /* 6-digit */
}

/* Verifies OTP by printing it (simulation) then reading user entry */
int verifyOTP_simulation() {
    int otp = generateOTP();
    int entered;
    printf("\n--- OTP (simulation) ---\n");
    printf("OTP Sent (on-screen for simulation): %06d\n", otp);
    printf("Enter OTP: ");
    if (scanf("%d", &entered) != 1) { while (getchar() != '\n'); return 0; }
    if (entered != otp) {
        printf("Incorrect OTP!\n");
        return 0;
    }
    return 1;
}

/* ---------------- Debit card generation ---------------- */

void generateDebitCard(struct Account *a) {
    /* Generate a 16-digit card number starting with 5 (Mastercard-style) */
    int p1 = rand() % 1000;
    int p2 = rand() % 10000;
    int p3 = rand() % 10000;
    int p4 = rand() % 10000;
    snprintf(a->debitCard, sizeof(a->debitCard), "5%03d%04d%04d%04d", p1, p2, p3, p4);
    a->cvv = rand() % 900 + 100;
    a->expiryYear = 2028 + rand() % 8; /* 2028 - 2035 */
    a->cardBlocked = 0;
}

/* ---------------- Interest Auto-Credit ----------------
   Credited once at login (simulation). For real apps, you'd run scheduled jobs.
*/
void autoCreditInterest(int idx) {
    if (idx < 0 || idx >= totalAccounts) return;
    if (strcmp(bank[idx].accountType, "Savings") != 0) return;
    float bal = decrypt(bank[idx].balance);
    if (bal <= 0.0f) return;
    float interest = bal * (SAVINGS_INTEREST / 100.0f);
    bal += interest;
    bank[idx].balance = encrypt(bal);
    char msg[200];
    snprintf(msg, sizeof(msg), "Interest credited: %.2f (%.2f%%)", interest, (double)SAVINGS_INTEREST);
    addHistory(idx, msg);
    saveToFile();
}

/* ---------------- Account creation ---------------- */

void createAccount() {
    if (totalAccounts >= MAX) {
        printf("Bank storage full!\n");
        return;
    }

    struct Account a;
    a.transCount = 0;
    a.isLocked = 0;

    printf("\n--- Create Account ---\n");
    printf("Enter unique Account Number (integer): ");
    if (scanf("%d", &a.accNo) != 1) { while (getchar() != '\n'); printf("Invalid input\n"); return; }

    if (findAccount(a.accNo) != -1) {
        printf("Account number already exists!\n");
        return;
    }

    printf("Enter Full Name: ");
    while (getchar() != '\n'); /* flush newline */
    if (!fgets(a.name, sizeof(a.name), stdin)) strcpy(a.name, "Unnamed");
    /* remove newline */
    size_t L = strlen(a.name);
    if (L && a.name[L - 1] == '\n') a.name[L - 1] = '\0';

    /* For now only Savings offered; extendable */
    strcpy(a.accountType, "Savings");

    printf("Set 4-digit PIN (numbers only): ");
    if (scanf("%d", &a.pin) != 1) { while (getchar() != '\n'); printf("Invalid\n"); return; }

    a.balance = encrypt(0.0f);
    generateDebitCard(&a);

    bank[totalAccounts++] = a;
    saveToFile();

    printf("Account created successfully!\n");
    printf("Account No: %d  Name: %s\n", a.accNo, a.name);
    printf("Debit Card generated (show from account menu): %s\n", a.debitCard);
}

/* ---------------- PIN & transaction-level verification ---------------- */

int verifyPIN_transaction(int idx) {
    int pin;
    printf("Enter PIN to confirm: ");
    if (scanf("%d", &pin) != 1) { while (getchar() != '\n'); return 0; }
    if (pin != bank[idx].pin) {
        printf("Incorrect PIN.\n");
        return 0;
    }
    return 1;
}

/* ---------------- Banking operations ---------------- */

void deposit_flow(int idx) {
    if (!verifyPIN_transaction(idx)) return;
    if (!verifyOTP_simulation()) return;

    float amt;
    printf("Enter amount to deposit: ");
    if (scanf("%f", &amt) != 1) { while (getchar() != '\n'); printf("Invalid\n"); return; }
    if (amt <= 0.0f) { printf("Amount should be positive\n"); return; }

    float bal = decrypt(bank[idx].balance);
    bal += amt;
    bank[idx].balance = encrypt(bal);

    char msg[200];
    snprintf(msg, sizeof(msg), "Deposited: %.2f", amt);
    addHistory(idx, msg);
    saveToFile();

    printf("Deposit successful. New balance: \u20b9%.2f\n", (double)decrypt(bank[idx].balance));
}

void withdraw_flow(int idx) {
    if (!verifyPIN_transaction(idx)) return;
    if (!verifyOTP_simulation()) return;

    float amt;
    printf("Enter amount to withdraw: ");
    if (scanf("%f", &amt) != 1) { while (getchar() != '\n'); printf("Invalid\n"); return; }
    if (amt <= 0.0f) { printf("Amount should be positive\n"); return; }

    float bal = decrypt(bank[idx].balance);
    if (bal - amt < MIN_BALANCE) {
        printf("Transaction would violate minimum balance of \u20b9%.2f for Savings account.\n", (double)MIN_BALANCE);
        return;
    }
    if (amt > bal) {
        printf("Insufficient funds.\n");
        return;
    }

    bal -= amt;
    bank[idx].balance = encrypt(bal);
    char msg[200];
    snprintf(msg, sizeof(msg), "Withdrawn: %.2f", amt);
    addHistory(idx, msg);
    saveToFile();

    printf("Withdrawal successful. New balance: \u20b9%.2f\n", (double)decrypt(bank[idx].balance));
}

void transfer_flow(int idx) {
    if (!verifyPIN_transaction(idx)) return;
    if (!verifyOTP_simulation()) return;

    int recvAcc;
    printf("Enter receiver account number: ");
    if (scanf("%d", &recvAcc) != 1) { while (getchar() != '\n'); printf("Invalid\n"); return; }

    int r = findAccount(recvAcc);
    if (r == -1) { printf("Receiver account not found.\n"); return; }

    float amt;
    printf("Enter amount to transfer: ");
    if (scanf("%f", &amt) != 1) { while (getchar() != '\n'); printf("Invalid\n"); return; }

    if (amt <= 0.0f) { printf("Amount should be positive\n"); return; }

    float bal = decrypt(bank[idx].balance);
    if (bal - amt < MIN_BALANCE) {
        printf("Transaction would violate minimum balance of \u20b9%.2f for Savings account.\n", (double)MIN_BALANCE);
        return;
    }

    if (amt > bal) { printf("Insufficient funds.\n"); return; }

    bal -= amt;
    bank[idx].balance = encrypt(bal);

    float rbal = decrypt(bank[r].balance);
    rbal += amt;
    bank[r].balance = encrypt(rbal);

    char msg1[200], msg2[200];
    snprintf(msg1, sizeof(msg1), "Transferred %.2f to %d", amt, recvAcc);
    snprintf(msg2, sizeof(msg2), "Received %.2f from %d", amt, bank[idx].accNo);
    addHistory(idx, msg1);
    addHistory(r, msg2);

    saveToFile();
    printf("Transfer successful. New balance: \u20b9%.2f\n", (double)decrypt(bank[idx].balance));
}

/* ---------------- Transaction history & account details ---------------- */

void showHistory(int idx) {
    printf("\n--- Transaction History for %d (%s) ---\n", bank[idx].accNo, bank[idx].name);
    if (bank[idx].transCount == 0) {
        printf("No transactions yet.\n");
        return;
    }
    for (int j = 0; j < bank[idx].transCount; ++j)
        printf("%d. %s\n", j + 1, bank[idx].history[j]);
}

void showAccountSummary(int idx) {
    printf("\n--- Account Summary ---\n");
    printf("Account: %d\nName: %s\nType: %s\nBalance: \u20b9%.2f\n",
           bank[idx].accNo, bank[idx].name, bank[idx].accountType, (double)decrypt(bank[idx].balance));
    printf("Debit Card: %s  (CVV %03d) Exp: %d  %s\n",
           bank[idx].debitCard, bank[idx].cvv, bank[idx].expiryYear, bank[idx].cardBlocked ? "(Blocked)" : "");
}

/* ---------------- Debit card features ---------------- */

void showCardDetails(int idx) {
    printf("\n--- Debit Card Details ---\n");
    if (bank[idx].cardBlocked) {
        printf("Card is BLOCKED.\n");
    }
    printf("Card Number: %s\nCVV: %03d\nExpiry Year: %d\n", bank[idx].debitCard, bank[idx].cvv, bank[idx].expiryYear);
}

void blockCard(int idx) {
    bank[idx].cardBlocked = 1;
    addHistory(idx, "Debit card blocked");
    saveToFile();
    printf("Debit card blocked for account %d.\n", bank[idx].accNo);
}

void regenerateCard(int idx) {
    generateDebitCard(&bank[idx]);
    char msg[200];
    snprintf(msg, sizeof(msg), "Debit card regenerated: %s", bank[idx].debitCard);
    addHistory(idx, msg);
    saveToFile();
    printf("New debit card generated: %s\n", bank[idx].debitCard);
}

/* ---------------- Financial calculators ---------------- */

void loanEMI_calculator() {
    float P, R;
    int Y;
    printf("\n--- Loan EMI Calculator ---\n");
    printf("Loan principal (\u20b9): "); if (scanf("%f", &P) != 1) { while (getchar() != '\n'); return; }
    printf("Annual rate (%%): "); if (scanf("%f", &R) != 1) { while (getchar() != '\n'); return; }
    printf("Loan duration (years): "); if (scanf("%d", &Y) != 1) { while (getchar() != '\n'); return; }

    float monthlyRate = R / (12.0f * 100.0f);
    int months = Y * 12;
    if (monthlyRate <= 0.0f) {
        printf("Monthly EMI: %.2f\nTotal payment: %.2f\nTotal interest: %.2f\n",
               P / months, P, 0.0f);
        return;
    }
    float EMI = (P * monthlyRate * powf(1 + monthlyRate, months)) / (powf(1 + monthlyRate, months) - 1);
    float total = EMI * months;
    printf("Monthly EMI: \u20b9%.2f\nTotal payment: \u20b9%.2f\nTotal interest: \u20b9%.2f\n", (double)EMI, (double)total, (double)(total - P));
}

void SIP_calculator() {
    float monthly, annualReturn;
    int years;
    printf("\n--- SIP Calculator ---\n");
    printf("Monthly investment (\u20b9): "); if (scanf("%f", &monthly) != 1) { while (getchar() != '\n'); return; }
    printf("Expected annual return (%%): "); if (scanf("%f", &annualReturn) != 1) { while (getchar() != '\n'); return; }
    printf("Duration (years): "); if (scanf("%d", &years) != 1) { while (getchar() != '\n'); return; }

    int months = years * 12;
    float monthlyRate = annualReturn / (12.0f * 100.0f);
    float future;
    if (monthlyRate <= 0.0f) {
        future = monthly * months;
    } else {
        future = monthly * ((powf(1 + monthlyRate, months) - 1) / monthlyRate) * (1 + monthlyRate);
    }
    printf("Estimated future value after %d years: \u20b9%.2f\n", years, (double)future);
}

void FD_calculator() {
    float P, r;
    int years;
    printf("\n--- Fixed Deposit Calculator ---\n");
    printf("Principal (\u20b9): "); if (scanf("%f", &P) != 1) { while (getchar() != '\n'); return; }
    printf("Annual rate (%%): "); if (scanf("%f", &r) != 1) { while (getchar() != '\n'); return; }
    printf("Years: "); if (scanf("%d", &years) != 1) { while (getchar() != '\n'); return; }
    float maturity = P * powf(1 + r / 100.0f, years);
    printf("Maturity amount: \u20b9%.2f\n", (double)maturity);
}

/* ---------------- Advanced Smart Investment Advisor ----------------
   Uses age, monthly income, risk appetite, duration to suggest allocations,
   emergency fund, SIP suggestions and projected returns.
*/
void investmentAdvisor_advanced(int idx) {
    printf("\n--- Smart Investment Advisor (Advanced) ---\n");

    int age;
    float monthlyIncome;
    int riskChoice; /* 1 Low, 2 Medium, 3 High */
    int years;

    printf("Enter your age (years): "); if (scanf("%d", &age) != 1) { while (getchar() != '\n'); return; }
    printf("Enter monthly disposable income for investments (\u20b9): "); if (scanf("%f", &monthlyIncome) != 1) { while (getchar() != '\n'); return; }
    printf("Select risk appetite: 1) Low  2) Medium  3) High : "); if (scanf("%d", &riskChoice) != 1) { while (getchar() != '\n'); return; }
    printf("Investment duration (years): "); if (scanf("%d", &years) != 1) { while (getchar() != '\n'); return; }

    if (years <= 0) { printf("Invalid duration\n"); return; }
    if (monthlyIncome < 0) monthlyIncome = 0;

    /* Emergency fund recommendation */
    int emergencyMonths = 6; /* default 6 months */
    if (riskChoice == 1) emergencyMonths = 9;
    if (age > 55) emergencyMonths = 9;
    float emergencyFund = emergencyMonths * monthlyIncome;

    /* base equity allocation by age and risk:
       rule-of-thumb style: younger -> more equity
    */
    int baseEquityPct;
    if (age < 30) baseEquityPct = 70;
    else if (age < 45) baseEquityPct = 60;
    else if (age < 60) baseEquityPct = 40;
    else baseEquityPct = 30;

    /* adjust by risk appetite */
    int equityPct = baseEquityPct;
    if (riskChoice == 3) equityPct += 10;
    if (riskChoice == 1) equityPct -= 10;

    if (equityPct > 90) equityPct = 90;
    if (equityPct < 10) equityPct = 10;

    int debtPct = 100 - equityPct;
    int goldPct = 5; /* small allocation to gold */
    if (equityPct <= 40) goldPct = 10;

    /* Slight redistribution */
    int eq = equityPct - goldPct;
    int de = debtPct;
    if (eq < 0) { eq = equityPct; goldPct = 0; }

    /* expected return assumptions (conservative estimates) */
    float eqReturn = 0.12f;    /* 12% p.a equity */
    float debtReturn = 0.06f;  /* 6% p.a debt/FD */
    float goldReturn = 0.06f;  /* 6% p.a gold */

    /* Projected returns starting from current balance + monthly SIP from disposable income */
    float currentBal = decrypt(bank[idx].balance);
    float monthlySIP = monthlyIncome; /* use all disposable amount as SIP (suggestion) */

    /* Compute future value with monthly SIP for 'years' assuming blended return */
    float blendedReturn = ((eq * (eqReturn)) + (de * (debtReturn)) + (goldPct * (goldReturn))) / 100.0f;
    int months = years * 12;
    float monthlyRate = blendedReturn / 12.0f;
    float future = currentBal * powf(1 + blendedReturn, years);
    if (monthlyRate > 0.0f) {
        future += monthlySIP * ((powf(1 + monthlyRate, months) - 1) / monthlyRate) * (1 + monthlyRate);
    } else {
        future += monthlySIP * months;
    }

    /* SIP suggestion amount (practical) */
    float suggestedSIP = monthlyIncome * 0.6f; /* recommend putting 60% of provided disposable into SIP */
    if (suggestedSIP < 0) suggestedSIP = 0;

    /* Present suggestions */
    printf("\n--- Personalized Recommendation ---\n");
    printf("Emergency Fund Suggestion: \u20b9%.2f (approx. %d months of income)\n", (double)emergencyFund, emergencyMonths);
    printf("Target Asset Allocation (approx):\n");
    printf("  Equity: %d%%\n  Debt/FD: %d%%\n  Gold: %d%%\n", eq, de, goldPct);
    printf("Suggested monthly SIP: \u20b9%.2f (you provided \u20b9%.2f disposable)\n", (double)suggestedSIP, (double)monthlyIncome);
    printf("Projected portfolio value after %d years (with suggested SIP): \u20b9%.2f\n", years, (double)future);
    printf("\nNotes:\n - Rebalance yearly.\n - Keep emergency fund in liquid savings/FD.\n - Start SIPs early to benefit from compounding.\n");

    /* Save to history with key details */
    char hist[200];
    snprintf(hist, sizeof(hist), "InvAdvice: age%d risk%d yrs%d SIP%.0f", age, riskChoice, years, suggestedSIP);
    addHistory(idx, hist);
    saveToFile();
}

/* ---------------- Admin panel ---------------- */

void adminPanel() {
    printf("\n--- Admin Panel ---\n");
    int pass;
    printf("Enter admin password: ");
    if (scanf("%d", &pass) != 1) { while (getchar() != '\n'); return; }
    if (pass != ADMIN_PASS) { printf("Access denied.\n"); return; }

    int choice = 0;
    do {
        printf("\n1) View all accounts\n2) Unlock account\n3) Delete account\n4) Bank totals & stats\n5) Reset customer's PIN\n6) Exit admin\nChoice: ");
        if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); return; }
        if (choice == 1) {
            printf("\nAccNo | Name | Type | Balance | Locked\n");
            for (int i = 0; i < totalAccounts; ++i) {
                printf("%d | %s | %s | \u20b9%.2f | %s\n", bank[i].accNo, bank[i].name, bank[i].accountType,
                       (double)decrypt(bank[i].balance), bank[i].isLocked ? "YES" : "NO");
            }
        } else if (choice == 2) {
            int acc; printf("Account to unlock: "); if (scanf("%d", &acc) != 1) { while (getchar() != '\n'); continue; }
            int i = findAccount(acc); if (i == -1) { printf("Not found\n"); } else { bank[i].isLocked = 0; addHistory(i, "Account unlocked by admin"); saveToFile(); printf("Unlocked\n"); }
        } else if (choice == 3) {
            int acc; printf("Account to delete: "); if (scanf("%d", &acc) != 1) { while (getchar() != '\n'); continue; }
            int i = findAccount(acc); if (i == -1) { printf("Not found\n"); } else {
                for (int j = i; j < totalAccounts - 1; ++j) bank[j] = bank[j + 1];
                totalAccounts--; saveToFile(); printf("Deleted\n");
            }
        } else if (choice == 4) {
            float total = 0.0f; for (int i = 0; i < totalAccounts; ++i) total += decrypt(bank[i].balance);
            printf("Total customers: %d\nTotal bank funds: \u20b9%.2f\n", totalAccounts, (double)total);
        } else if (choice == 5) {
            int acc; printf("Account to reset PIN: "); if (scanf("%d", &acc) != 1) { while (getchar() != '\n'); continue; }
            int i = findAccount(acc); if (i == -1) { printf("Not found\n"); } else { int newpin; printf("New PIN: "); if (scanf("%d", &newpin) != 1) { while (getchar() != '\n'); continue; } bank[i].pin = newpin; addHistory(i, "PIN reset by admin"); saveToFile(); printf("PIN reset\n"); }
        }
    } while (choice != 6);
}

/* ---------------- Login & user menu ---------------- */

void userMenu(int idx) {
    if (idx < 0 || idx >= totalAccounts) return;

    int tries = 0;
    int pin;
    if (bank[idx].isLocked) { printf("Account is locked. Contact admin.\n"); return; }

    while (tries < MAX_PIN_TRIES) {
        printf("Enter PIN: ");
        if (scanf("%d", &pin) != 1) { while (getchar() != '\n'); return; }
        if (pin == bank[idx].pin) break;
        tries++;
        printf("Wrong PIN (%d/%d)\n", tries, MAX_PIN_TRIES);
    }
    if (tries == MAX_PIN_TRIES) {
        bank[idx].isLocked = 1;
        addHistory(idx, "Account locked due to failed PIN attempts");
        saveToFile();
        printf("Account locked due to repeated wrong PIN.\n");
        return;
    }

    /* Auto-credit interest once-per-login (simulation) */
    autoCreditInterest(idx);

    int choice = 0;
    do {
        printf("\n--- Welcome %s (Acc: %d) ---\n", bank[idx].name, bank[idx].accNo);
        printf("1) Deposit\n2) Withdraw\n3) Transfer\n4) Show account summary\n5) Transaction history\n6) Debit card details / block / regenerate\n7) Investment Advisor (advanced)\n8) SIP Calculator\n9) FD Calculator\n10) Loan EMI calculator\n11) Change PIN\n12) Logout\nChoice: ");
        if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); return; }

        if (choice == 1) deposit_flow(idx);
        else if (choice == 2) withdraw_flow(idx);
        else if (choice == 3) transfer_flow(idx);
        else if (choice == 4) showAccountSummary(idx);
        else if (choice == 5) showHistory(idx);
        else if (choice == 6) {
            printf("\nCard Menu: 1 Show  2 Block  3 Regenerate  4 Back\nChoice: ");
            int c; if (scanf("%d", &c) != 1) { while (getchar() != '\n'); continue; }
            if (c == 1) showCardDetails(idx);
            else if (c == 2) { blockCard(idx); }
            else if (c == 3) { regenerateCard(idx); }
        }
        else if (choice == 7) investmentAdvisor_advanced(idx);
        else if (choice == 8) SIP_calculator();
        else if (choice == 9) FD_calculator();
        else if (choice == 10) loanEMI_calculator();
        else if (choice == 11) {
            int oldpin, newpin;
            printf("Enter current PIN: "); if (scanf("%d", &oldpin) != 1) { while (getchar() != '\n'); continue; }
            if (oldpin != bank[idx].pin) { printf("Incorrect current PIN\n"); }
            else { printf("Enter new PIN: "); if (scanf("%d", &newpin) != 1) { while (getchar() != '\n'); continue; } bank[idx].pin = newpin; addHistory(idx, "PIN changed"); saveToFile(); printf("PIN changed successfully\n"); }
        }

    } while (choice != 12);
}

/* ---------------- Main entry ---------------- */

int main(void) {
    srand((unsigned int)time(NULL));
    loadFromFile();

    int choice = 0;
    while (1) {
        printf("\n=== 5-STAR ENTERPRISE BANK SYSTEM ===\n");
        printf("1) Create Account\n");
        printf("2) Login\n");
        printf("3) Admin Panel\n");
        printf("4) Exit\nChoice: ");
        if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); printf("Invalid\n"); continue; }

        if (choice == 1) createAccount();
        else if (choice == 2) {
            int acc;
            printf("Enter Account Number: ");
            if (scanf("%d", &acc) != 1) { while (getchar() != '\n'); printf("Invalid\n"); continue; }
            int idx = findAccount(acc);
            if (idx == -1) { printf("Account not found.\n"); continue; }
            userMenu(idx);
        }
        else if (choice == 3) adminPanel();
        else if (choice == 4) {
            saveToFile();
            printf("Goodbye!\n");
            exit(0);
        }
    }
    return 0;
}



