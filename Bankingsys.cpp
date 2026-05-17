/*
 * ============================================================
 *  C++ Banking System
 *  Compiler : g++ banking_system.cpp -o bank   (C++11 and above)
 *  Platform : Windows / Linux / macOS (VS Code terminal)
 * ============================================================
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <limits>

// ─── colour helpers (ANSI; ignored on plain Windows cmd) ──────────────────────
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"

// ─── constants ────────────────────────────────────────────────────────────────
const std::string DATA_FILE = "accounts.dat";
const std::string LOG_FILE = "transactions.log";
const double MIN_BALANCE = 500.0; // minimum required balance

// ─── utility helpers ──────────────────────────────────────────────────────────
static std::string currentTimestamp()
{
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}

static void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void pause()
{
    std::cout << "\n"
              << YELLOW << "Press ENTER to continue..." << RESET;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

static void printLine(char c = '-', int n = 55)
{
    std::cout << std::string(n, c) << "\n";
}

// ─── Transaction record ───────────────────────────────────────────────────────
struct Transaction
{
    std::string timestamp;
    std::string type; // DEPOSIT / WITHDRAW / TRANSFER / CREATED / DELETED
    double amount;
    double balanceAfter;
    std::string note;
};

// ─── Account class ────────────────────────────────────────────────────────────
class Account
{
public:
    // constructors
    Account() : accNo(0), pin(""), name(""), balance(0.0), active(true) {}

    Account(long long accNo, const std::string &pin,
            const std::string &name, double balance)
        : accNo(accNo), pin(pin), name(name), balance(balance), active(true)
    {
        addTransaction("CREATED", 0.0, balance, "Account opened");
    }

    // getters
    long long getAccNo() const { return accNo; }
    std::string getName() const { return name; }
    double getBalance() const { return balance; }
    bool isActive() const { return active; }
    const std::vector<Transaction> &getHistory() const { return history; }

    // PIN check
    bool verifyPin(const std::string &p) const { return pin == p; }

    // deposit
    bool deposit(double amount)
    {
        if (amount <= 0)
        {
            std::cout << RED << "Amount must be positive.\n"
                      << RESET;
            return false;
        }
        balance += amount;
        addTransaction("DEPOSIT", amount, balance, "");
        return true;
    }

    // withdraw
    bool withdraw(double amount)
    {
        if (amount <= 0)
        {
            std::cout << RED << "Amount must be positive.\n"
                      << RESET;
            return false;
        }
        if (balance - amount < MIN_BALANCE)
        {
            std::cout << RED << "Insufficient funds. Minimum balance ₹"
                      << std::fixed << std::setprecision(2) << MIN_BALANCE << " required.\n"
                      << RESET;
            return false;
        }
        balance -= amount;
        addTransaction("WITHDRAW", amount, balance, "");
        return true;
    }

    // receive transfer (internal helper)
    void receiveTransfer(double amount, long long fromAcc)
    {
        balance += amount;
        addTransaction("TRANSFER IN", amount, balance,
                       "From account " + std::to_string(fromAcc));
    }

    // send transfer (internal helper — caller already deducted)
    void markTransferOut(double amount, long long toAcc, double newBal)
    {
        balance = newBal;
        addTransaction("TRANSFER OUT", amount, balance,
                       "To account " + std::to_string(toAcc));
    }

    // close account
    void close()
    {
        active = false;
        addTransaction("DELETED", 0.0, balance, "Account closed");
    }

    // change PIN
    bool changePin(const std::string &oldPin, const std::string &newPin)
    {
        if (!verifyPin(oldPin))
        {
            std::cout << RED << "Incorrect current PIN.\n"
                      << RESET;
            return false;
        }
        if (newPin.size() < 4)
        {
            std::cout << RED << "PIN must be at least 4 digits.\n"
                      << RESET;
            return false;
        }
        pin = newPin;
        addTransaction("PIN CHANGE", 0.0, balance, "");
        return true;
    }

    // serialise to one CSV line (history stored separately in log)
    std::string toCSV() const
    {
        return std::to_string(accNo) + "|" + pin + "|" + name + "|" +
               std::to_string(balance) + "|" + (active ? "1" : "0");
    }

    // deserialise
    static Account fromCSV(const std::string &line)
    {
        Account a;
        std::istringstream ss(line);
        std::string tok;
        int field = 0;
        while (std::getline(ss, tok, '|'))
        {
            switch (field++)
            {
            case 0:
                a.accNo = std::stoll(tok);
                break;
            case 1:
                a.pin = tok;
                break;
            case 2:
                a.name = tok;
                break;
            case 3:
                a.balance = std::stod(tok);
                break;
            case 4:
                a.active = (tok == "1");
                break;
            }
        }
        return a;
    }

    // print mini-statement
    void printStatement() const
    {
        std::cout << CYAN << BOLD
                  << "\n  ── Statement for " << name
                  << " (Acc# " << accNo << ") ──\n"
                  << RESET;
        printLine();
        std::cout << std::left
                  << std::setw(20) << "Date/Time"
                  << std::setw(14) << "Type"
                  << std::setw(12) << "Amount(₹)"
                  << std::setw(13) << "Balance(₹)"
                  << "Note\n";
        printLine();
        for (const auto &t : history)
        {
            std::cout << std::left
                      << std::setw(20) << t.timestamp
                      << std::setw(14) << t.type
                      << std::setw(12) << std::fixed << std::setprecision(2) << t.amount
                      << std::setw(13) << t.balanceAfter
                      << t.note << "\n";
        }
        printLine();
    }

private:
    long long accNo;
    std::string pin;
    std::string name;
    double balance;
    bool active;
    std::vector<Transaction> history;

    void addTransaction(const std::string &type, double amount,
                        double balAfter, const std::string &note)
    {
        history.push_back({currentTimestamp(), type, amount, balAfter, note});
        // append to persistent log
        std::ofstream log(LOG_FILE, std::ios::app);
        if (log)
            log << accNo << "," << currentTimestamp() << "," << type << ","
                << std::fixed << std::setprecision(2) << amount << ","
                << balAfter << "," << note << "\n";
    }
};

// ─── Bank class ───────────────────────────────────────────────────────────────
class Bank
{
public:
    Bank(const std::string &bankName) : bankName(bankName), nextAccNo(100001)
    {
        loadFromFile();
    }

    ~Bank() { saveToFile(); }

    // ── Admin menu ────────────────────────────────────────────────────────────
    void adminMenu()
    {
        int choice;
        do
        {
            clearScreen();
            printBanner();
            std::cout << CYAN << BOLD << "  ADMIN PANEL\n"
                      << RESET;
            printLine();
            std::cout << "  1. Create New Account\n"
                      << "  2. View All Accounts\n"
                      << "  3. Search Account\n"
                      << "  4. Delete Account\n"
                      << "  5. Back to Main Menu\n";
            printLine();
            std::cout << "  Choice: ";
            choice = getInt();
            switch (choice)
            {
            case 1:
                createAccount();
                break;
            case 2:
                listAccounts();
                break;
            case 3:
                searchAccount();
                break;
            case 4:
                deleteAccount();
                break;
            case 5:
                break;
            default:
                std::cout << RED << "Invalid option.\n"
                          << RESET;
                pause();
            }
        } while (choice != 5);
    }

    // ── Customer menu ─────────────────────────────────────────────────────────
    void customerMenu()
    {
        clearScreen();
        printBanner();
        long long accNo = promptAccNo();
        auto it = accounts.find(accNo);
        if (it == accounts.end() || !it->second.isActive())
        {
            std::cout << RED << "Account not found.\n"
                      << RESET;
            pause();
            return;
        }
        std::string pin = promptPin();
        if (!it->second.verifyPin(pin))
        {
            std::cout << RED << "Incorrect PIN.\n"
                      << RESET;
            pause();
            return;
        }
        Account &acc = it->second;
        int choice;
        do
        {
            clearScreen();
            printBanner();
            std::cout << CYAN << BOLD
                      << "  Welcome, " << acc.getName()
                      << "  |  Balance: ₹"
                      << std::fixed << std::setprecision(2) << acc.getBalance()
                      << "\n"
                      << RESET;
            printLine();
            std::cout << "  1. Check Balance\n"
                      << "  2. Deposit\n"
                      << "  3. Withdraw\n"
                      << "  4. Transfer\n"
                      << "  5. Mini Statement\n"
                      << "  6. Change PIN\n"
                      << "  7. Logout\n";
            printLine();
            std::cout << "  Choice: ";
            choice = getInt();
            switch (choice)
            {
            case 1:
                checkBalance(acc);
                break;
            case 2:
                doDeposit(acc);
                break;
            case 3:
                doWithdraw(acc);
                break;
            case 4:
                doTransfer(acc);
                break;
            case 5:
                acc.printStatement();
                pause();
                break;
            case 6:
                doChangePin(acc);
                break;
            case 7:
                break;
            default:
                std::cout << RED << "Invalid option.\n"
                          << RESET;
                pause();
            }
        } while (choice != 7);
        saveToFile();
    }

    // ── Main menu ─────────────────────────────────────────────────────────────
    void run()
    {
        int choice;
        do
        {
            clearScreen();
            printBanner();
            std::cout << "  1. Admin Login\n"
                      << "  2. Customer Login\n"
                      << "  3. Exit\n";
            printLine();
            std::cout << "  Choice: ";
            choice = getInt();
            switch (choice)
            {
            case 1:
            {
                std::cout << "  Admin Password: ";
                std::string pw;
                std::cin >> pw;
                if (pw == "admin123")
                    adminMenu();
                else
                {
                    std::cout << RED << "Wrong password.\n"
                              << RESET;
                    pause();
                }
                break;
            }
            case 2:
                customerMenu();
                break;
            case 3:
                std::cout << GREEN << "  Thank you for using " << bankName << "!\n"
                          << RESET;
                break;
            default:
                std::cout << RED << "Invalid option.\n"
                          << RESET;
                pause();
            }
        } while (choice != 3);
    }

private:
    std::string bankName;
    long long nextAccNo;
    std::map<long long, Account> accounts;

    // ── banner ────────────────────────────────────────────────────────────────
    void printBanner() const
    {
        printLine('=');
        std::cout << BOLD << CYAN
                  << "   🏦  " << bankName << " Banking System\n"
                  << RESET;
        printLine('=');
    }

    // ── input helpers ─────────────────────────────────────────────────────────
    static int getInt()
    {
        int v;
        while (!(std::cin >> v))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << RED << "  Enter a valid number: " << RESET;
        }
        return v;
    }

    static double getDouble()
    {
        double v;
        while (!(std::cin >> v) || v < 0)
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << RED << "  Enter a valid positive amount: " << RESET;
        }
        return v;
    }

    static long long promptAccNo()
    {
        std::cout << "  Account Number: ";
        long long v;
        while (!(std::cin >> v))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << RED << "  Enter a valid account number: " << RESET;
        }
        return v;
    }

    static std::string promptPin()
    {
        std::cout << "  PIN: ";
        std::string p;
        std::cin >> p;
        return p;
    }

    // ── admin operations ──────────────────────────────────────────────────────
    void createAccount()
    {
        clearScreen();
        printBanner();
        std::cout << BOLD << "  CREATE NEW ACCOUNT\n"
                  << RESET;
        printLine();

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Full Name       : ";
        std::string name;
        std::getline(std::cin, name);

        std::cout << "  Set PIN (≥4 dig): ";
        std::string pin;
        std::cin >> pin;
        while (pin.size() < 4)
        {
            std::cout << RED << "  PIN too short. Try again: " << RESET;
            std::cin >> pin;
        }

        std::cout << "  Initial Deposit (min ₹" << MIN_BALANCE << "): ₹";
        double dep = getDouble();
        while (dep < MIN_BALANCE)
        {
            std::cout << RED << "  Must be at least ₹" << MIN_BALANCE << ": ₹" << RESET;
            dep = getDouble();
        }

        Account acc(nextAccNo, pin, name, dep);
        accounts[nextAccNo] = acc;
        std::cout << GREEN << "\n  ✓ Account created!\n"
                  << "    Account Number : " << nextAccNo << "\n"
                  << "    Name           : " << name << "\n"
                  << "    Opening Balance: ₹" << std::fixed << std::setprecision(2) << dep
                  << "\n"
                  << RESET;
        nextAccNo++;
        saveToFile();
        pause();
    }

    void listAccounts() const
    {
        clearScreen();
        printBanner();
        std::cout << BOLD << "  ALL ACCOUNTS\n"
                  << RESET;
        printLine();
        std::cout << std::left
                  << std::setw(12) << "Acc No"
                  << std::setw(25) << "Name"
                  << std::setw(14) << "Balance (₹)"
                  << "Status\n";
        printLine();
        for (std::map<long long, Account>::const_iterator it = accounts.begin(); it != accounts.end(); ++it)
            std::cout << std::left
                      << std::setw(12) << it->first
                      << std::setw(25) << it->second.getName()
                      << std::setw(14) << std::fixed << std::setprecision(2) << it->second.getBalance()
                      << (it->second.isActive() ? GREEN "Active" : RED "Closed") << RESET << "\n";
        printLine();
        pause();
    }

    void searchAccount() const
    {
        long long no = promptAccNo();
        auto it = accounts.find(no);
        if (it == accounts.end())
        {
            std::cout << RED << "  Account not found.\n"
                      << RESET;
        }
        else
        {
            const Account &a = it->second;
            std::cout << GREEN
                      << "  Account No : " << a.getAccNo() << "\n"
                      << "  Name       : " << a.getName() << "\n"
                      << "  Balance    : ₹" << std::fixed << std::setprecision(2) << a.getBalance() << "\n"
                      << "  Status     : " << (a.isActive() ? "Active" : "Closed") << "\n"
                      << RESET;
        }
        pause();
    }

    void deleteAccount()
    {
        long long no = promptAccNo();
        auto it = accounts.find(no);
        if (it == accounts.end() || !it->second.isActive())
        {
            std::cout << RED << "  Account not found or already closed.\n"
                      << RESET;
        }
        else
        {
            it->second.close();
            std::cout << GREEN << "  Account " << no << " has been closed.\n"
                      << RESET;
            saveToFile();
        }
        pause();
    }

    // ── customer operations ───────────────────────────────────────────────────
    static void checkBalance(const Account &acc)
    {
        std::cout << GREEN << "\n  Balance: ₹"
                  << std::fixed << std::setprecision(2) << acc.getBalance()
                  << "\n"
                  << RESET;
        pause();
    }

    static void doDeposit(Account &acc)
    {
        std::cout << "  Deposit amount (₹): ";
        double amt = getDouble();
        if (acc.deposit(amt))
            std::cout << GREEN << "  ✓ Deposited ₹" << std::fixed << std::setprecision(2)
                      << amt << ". New balance: ₹" << acc.getBalance() << "\n"
                      << RESET;
        pause();
    }

    static void doWithdraw(Account &acc)
    {
        std::cout << "  Withdraw amount (₹): ";
        double amt = getDouble();
        if (acc.withdraw(amt))
            std::cout << GREEN << "  ✓ Withdrawn ₹" << std::fixed << std::setprecision(2)
                      << amt << ". New balance: ₹" << acc.getBalance() << "\n"
                      << RESET;
        pause();
    }

    void doTransfer(Account &sender)
    {
        long long toNo = promptAccNo();
        auto it = accounts.find(toNo);
        if (it == accounts.end() || !it->second.isActive() || toNo == sender.getAccNo())
        {
            std::cout << RED << "  Invalid destination account.\n"
                      << RESET;
            pause();
            return;
        }
        std::cout << "  Transfer amount (₹): ";
        double amt = getDouble();
        double newBal = sender.getBalance() - amt;
        if (newBal < MIN_BALANCE)
        {
            std::cout << RED << "  Insufficient funds after transfer.\n"
                      << RESET;
            pause();
            return;
        }
        sender.markTransferOut(amt, toNo, newBal);
        it->second.receiveTransfer(amt, sender.getAccNo());
        std::cout << GREEN << "  ✓ ₹" << std::fixed << std::setprecision(2) << amt
                  << " transferred to account " << toNo << ".\n"
                  << RESET;
        saveToFile();
        pause();
    }

    static void doChangePin(Account &acc)
    {
        std::cout << "  Current PIN : ";
        std::string old;
        std::cin >> old;
        std::cout << "  New PIN     : ";
        std::string np;
        std::cin >> np;
        if (acc.changePin(old, np))
            std::cout << GREEN << "  ✓ PIN changed successfully.\n"
                      << RESET;
        pause();
    }

    // ── persistence ───────────────────────────────────────────────────────────
    void saveToFile() const
    {
        std::ofstream f(DATA_FILE);
        if (!f)
            return;
        f << nextAccNo << "\n";
        for (std::map<long long, Account>::const_iterator it = accounts.begin(); it != accounts.end(); ++it)
            f << it->second.toCSV() << "\n";
    }

    void loadFromFile()
    {
        std::ifstream f(DATA_FILE);
        if (!f)
            return;
        std::string line;
        std::getline(f, line);
        if (!line.empty())
            nextAccNo = std::stoll(line);
        while (std::getline(f, line))
            if (!line.empty())
            {
                Account a = Account::fromCSV(line);
                accounts[a.getAccNo()] = a;
            }
    }
};

// ─── main ─────────────────────────────────────────────────────────────────────
int main()
{
    Bank bank("SwiftBank");
    bank.run();
    return 0;
}