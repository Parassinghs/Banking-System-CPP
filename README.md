# Banking-System-CPP
C++ Banking System
A complete console-based banking application written in C++11, built for VS Code on Windows, Linux, and macOS.

📁 Project Files
FilePurposebanking_system.cppFull source code (single file)accounts.datAuto-generated — stores all account datatransactions.logAuto-generated — persistent transaction log

accounts.dat and transactions.log are created automatically the first time you run the program. Do not delete accounts.dat or you will lose all account data.


⚙️ Requirements

g++ (MinGW on Windows, GCC on Linux/macOS)
C++11 or higher (no special flags needed)
VS Code with the C/C++ extension (optional but recommended)


🚀 How to Compile & Run
VS Code Terminal (PowerShell or bash)
powershellg++ banking_system.cpp -o banking_system
.\banking_system.exe        # Windows
./banking_system            # Linux / macOS
One-liner (compile + run)
powershellg++ banking_system.cpp -o banking_system ; .\banking_system

🔑 Default Credentials
RoleCredentialAdmin passwordadmin123Customer PINSet by admin when creating the account (minimum 4 digits)

🗂️ Menu Structure
Main Menu
├── 1. Admin Login
│   ├── 1. Create New Account
│   ├── 2. View All Accounts
│   ├── 3. Search Account
│   ├── 4. Delete Account
│   └── 5. Back
├── 2. Customer Login  (Account No + PIN)
│   ├── 1. Check Balance
│   ├── 2. Deposit
│   ├── 3. Withdraw
│   ├── 4. Transfer Funds
│   ├── 5. Mini Statement
│   ├── 6. Change PIN
│   └── 7. Logout
└── 3. Exit

✅ Features
Admin

Create accounts with a unique auto-incremented account number (starts at 100001)
View all accounts in a formatted table (account no, name, balance, status)
Search any account by account number
Soft-delete (close) an account — data is preserved in file

Customer

Secure PIN login (minimum 4 digits)
Check current balance
Deposit funds (any positive amount)
Withdraw funds (minimum balance of ₹500.00 enforced)
Transfer funds to another active account
View mini-statement with full timestamped history for the current session
Change PIN (requires current PIN verification)

System

Data persistence — accounts saved to accounts.dat on every change; data survives program restarts
Transaction log — every event (deposit, withdrawal, transfer, PIN change, open/close) appended to transactions.log with timestamps
Input validation — all invalid input (letters in number fields, negative amounts, wrong PIN, insufficient funds) handled gracefully with clear error messages
Cross-platform — cls on Windows, clear on Linux/macOS
ANSI colour output — colour-coded messages (green = success, red = error, yellow = prompt)


📐 Code Architecture
banking_system.cpp
│
├── struct Transaction         — holds one transaction record
│
├── class Account              — represents one bank account
│   ├── deposit()
│   ├── withdraw()
│   ├── receiveTransfer()
│   ├── markTransferOut()
│   ├── changePin()
│   ├── close()
│   ├── printStatement()
│   ├── toCSV() / fromCSV()    — serialisation
│   └── addTransaction()       — writes to in-memory history + log file
│
├── class Bank                 — manages all accounts + menus
│   ├── run()                  — main menu loop
│   ├── adminMenu()
│   ├── customerMenu()
│   ├── createAccount()
│   ├── listAccounts()
│   ├── searchAccount()
│   ├── deleteAccount()
│   ├── doDeposit/Withdraw/Transfer/ChangePin()
│   ├── saveToFile()           — writes accounts.dat
│   └── loadFromFile()         — reads accounts.dat on startup
│
└── main()                     — creates Bank("SwiftBank"), calls run()

📄 Data File Format
accounts.dat (pipe-delimited):
100003                          ← next account number to assign
100001|1234|Rahul Sharma|8500.00|1
100002|5678|Priya Mehta|2000.00|0
Fields: AccountNo | PIN | Name | Balance | Active(1/0)
transactions.log (comma-delimited, append-only):
100001,2025-09-01 10:32:11,CREATED,0.00,5000.00,Account opened
100001,2025-09-01 10:45:00,DEPOSIT,3500.00,8500.00,
100002,2025-09-01 11:00:00,DELETED,0.00,2000.00,Account closed

⚠️ Known Limitations

PINs are stored in plain text in accounts.dat. For a production system, use hashing (e.g. bcrypt).
The mini-statement only shows transactions from the current session (in-memory). Historical transactions are in transactions.log.
No multi-user concurrency — designed for single-user sequential access.
ANSI colours may not display on older Windows CMD. Use Windows Terminal or VS Code's integrated terminal for best results.

