#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <random>
#include <cctype>
#include <limits>

using namespace std;

class Transaction {
public:
    int accountNumber;
    string type;
    double amount;
    time_t timestamp;

    Transaction(int accNo, string t, double amt)
        : accountNumber(accNo), type(t), amount(amt) {
        time(&timestamp);
    }

    Transaction() : accountNumber(0), type(""), amount(0), timestamp(0) {}

    void display() const {
        char buffer[80];
        tm* ltm = localtime(&timestamp);
        strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", ltm);
        cout << setw(15) << left << type
             << "Rs." << setw(12) << right << fixed << setprecision(2) << amount
             << setw(25) << right << buffer << endl;
    }
};

class Account {
private:
    int accountNumber;
    string name;
    char type;
    double balance;
    vector<Transaction> transactions;

public:
    Account(int accNo = 0, string nm = "", char tp = 'S', double bal = 0.0)
        : accountNumber(accNo), name(nm), type(toupper(tp)), balance(bal) {}

    void deposit(double amount) {
        if (amount > 0) {
            balance += amount;
            transactions.emplace_back(accountNumber, "DEPOSIT", amount);
            cout << "Deposit successful. New balance: Rs." << fixed << setprecision(2) << balance << endl;
        } else {
            cout << "Invalid amount! Amount must be positive.\n";
        }
    }

    void withdraw(double amount) {
        if (amount <= 0) {
            cout << "Invalid amount! Amount must be positive.\n";
        } else if (balance >= amount) {
            balance -= amount;
            transactions.emplace_back(accountNumber, "WITHDRAWAL", amount);
            cout << "Withdrawal successful. New balance: Rs." << fixed << setprecision(2) << balance << endl;
        } else {
            cout << "Insufficient balance!\n";
        }
    }

    void display() const {
        cout << "\nAccount Details:\n";
        cout << "Account Number: " << accountNumber << endl;
        cout << "Account Holder: " << name << endl;
        cout << "Account Type: " << (type == 'S' ? "Savings" : "Current") << endl;
        cout << "Current Balance: Rs." << fixed << setprecision(2) << balance << endl;
    }

    void displayTransactions() const {
        cout << "\nTransaction History for Account: " << accountNumber << endl;
        cout << setw(15) << left << "Type" 
             << setw(15) << right << "Amount" 
             << setw(25) << right << "Date & Time" << endl;
        cout << string(55, '-') << endl;
        if (transactions.empty()) {
            cout << "No transactions found.\n";
        } else {
            for (const auto& t : transactions) {
                t.display();
            }
        }
    }

    int getAccountNumber() const { return accountNumber; }
    string getName() const { return name; }
    char getType() const { return type; }
    double getBalance() const { return balance; }
    const vector<Transaction>& getTransactions() const { return transactions; }
    void setTransactions(const vector<Transaction>& txs) { transactions = txs; }
};

class Bank {
private:
    vector<Account> accounts;
    map<string, string> userCredentials;
    string currentUser;

    void saveAccounts() {
        ofstream outFile("accounts.dat", ios::binary);
        for (const auto& acc : accounts) {
            size_t nameSize = acc.getName().size();
            outFile.write(reinterpret_cast<const char*>(&nameSize), sizeof(size_t));
            outFile.write(acc.getName().c_str(), nameSize);
            outFile.write(reinterpret_cast<const char*>(&acc), sizeof(Account) - sizeof(vector<Transaction>));
            size_t numTransactions = acc.getTransactions().size();
            outFile.write(reinterpret_cast<const char*>(&numTransactions), sizeof(size_t));
            for (const auto& t : acc.getTransactions()) {
                outFile.write(reinterpret_cast<const char*>(&t), sizeof(Transaction));
            }
        }
    }

    void loadAccounts() {
        ifstream inFile("accounts.dat", ios::binary);
        if (!inFile) return;
        accounts.clear();

        while (inFile.peek() != EOF) {
            size_t nameSize;
            inFile.read(reinterpret_cast<char*>(&nameSize), sizeof(size_t));
            if (inFile.eof() || nameSize == 0 || nameSize > 1000) break;
            string name(nameSize, '\0');
            inFile.read(&name[0], nameSize);

            Account acc;
            inFile.read(reinterpret_cast<char*>(&acc), sizeof(Account) - sizeof(vector<Transaction>));
            acc = Account(acc.getAccountNumber(), name, acc.getType(), acc.getBalance());

            size_t numTransactions;
            inFile.read(reinterpret_cast<char*>(&numTransactions), sizeof(size_t));
            if (numTransactions > 1000) break;
            vector<Transaction> txs;
            for (size_t i = 0; i < numTransactions; ++i) {
                Transaction t;
                inFile.read(reinterpret_cast<char*>(&t), sizeof(Transaction));
                if (!inFile) break;
                txs.push_back(t);
            }
            acc.setTransactions(txs);
            accounts.push_back(acc);
        }
    }

    void saveUsers() {
        ofstream outFile("users.dat");
        for (const auto& pair : userCredentials) {
            outFile << pair.first << "," << pair.second << "\n";
        }
    }

    void loadUsers() {
        ifstream inFile("users.dat");
        string line;
        while (getline(inFile, line)) {
            size_t pos = line.find(',');
            if (pos != string::npos) {
                string username = line.substr(0, pos);
                string password = line.substr(pos + 1);
                userCredentials[username] = password;
            }
        }
    }

    string generateOTP() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(100000, 999999);
        return to_string(dis(gen));
    }

public:
    Bank() {
        loadUsers();
        loadAccounts();
        userCredentials["admin"] = "admin123";
    }

    ~Bank() {
        saveAccounts();
        saveUsers();
    }

    bool login() {
        cout << "--- Welcome to Our Bank ---\n";
        cout << "Username: ";
        cin >> currentUser;
        cout << "Password: ";
        string password;
        cin >> password;

        auto it = userCredentials.find(currentUser);
        if (it != userCredentials.end() && it->second == password) {
            string otp = generateOTP();
            cout << "OTP: " << otp << "\nEnter OTP to proceed: ";
            string enteredOTP;
            cin >> enteredOTP;
            if (enteredOTP == otp) {
                cout << "\nLogin successful!\n";
                return true;
            } else {
                cout << "\nIncorrect OTP!\n";
                return false;
            }
        }
        cout << "\nInvalid credentials!\n";
        return false;
    }

    bool isAdmin() const { return currentUser == "admin"; }

    void createAccount() {
        int accNo;
        string name;
        char type;
        double balance;

        cout << "\nEnter Account Number: ";
        while (!(cin >> accNo) || accNo <= 0) {
            cout << "Invalid input. Please enter a positive number: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        cout << "Enter Account Holder Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Enter Account Type (S/C): ";
        while (!(cin >> type) || (toupper(type) != 'S' && toupper(type) != 'C')) {
            cout << "Invalid type. Please enter S for Savings or C for Current: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        type = toupper(type);

        cout << "Enter Initial Balance: Rs.";
        while (!(cin >> balance) || balance < 0) {
            cout << "Invalid amount. Please enter a positive number: Rs.";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        accounts.emplace_back(accNo, name, type, balance);
        string password = to_string(accNo) + name.substr(0, 3);
        userCredentials[to_string(accNo)] = password;
        cout << "\nAccount created successfully!\nYour login credentials:\n";
        cout << "Username: " << accNo << "\nPassword: " << password << "\n";
    }

    Account* findAccount(int accNo) {
        auto it = find_if(accounts.begin(), accounts.end(), [accNo](const Account& acc) {
            return acc.getAccountNumber() == accNo;
        });
        return it != accounts.end() ? &(*it) : nullptr;
    }

    void deleteAccount() {
        int accNo;
        cout << "Enter account number to delete: ";
        cin >> accNo;
        Account* acc = findAccount(accNo);
        if (!acc) {
            cout << "Account not found.\n";
            return;
        }
        char confirm;
        cout << "Are you sure you want to delete account " << accNo << "? (Y/N): ";
        cin >> confirm;
        if (toupper(confirm) == 'Y') {
            accounts.erase(remove_if(accounts.begin(), accounts.end(), [accNo](const Account& a) {
                return a.getAccountNumber() == accNo;
            }), accounts.end());
            userCredentials.erase(to_string(accNo));
            cout << "Account deleted successfully.\n";
        } else {
            cout << "Deletion cancelled.\n";
        }
    }

    void run() {
        while (true) {
            cout << "\n--- Welcome to Our Bank Menu ---\n";
            if (isAdmin()) {
                cout << "1. Create Account\n2. View All Accounts\n3. View User Credentials\n4. Delete Account\n0. Exit\n";
            } else {
                cout << "1. Deposit\n2. Withdraw\n3. View Account\n4. Transaction History\n0. Exit\n";
            }
            cout << "Enter your choice: ";
            int choice;
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input.\n";
                continue;
            }
            if (choice == 0) {
                cout << "Thank you for banking with us!\n";
                break;
            }

            if (isAdmin()) {
                switch (choice) {
                    case 1: createAccount(); break;
                    case 2: for (const auto& acc : accounts) acc.display(); break;
                    case 3: for (const auto& p : userCredentials) if (p.first != "admin") cout << "Username: " << p.first << " | Password: " << p.second << endl; break;
                    case 4: deleteAccount(); break;
                    default: cout << "Invalid choice!\n";
                }
            } else {
                int accNo;
                cout << "Enter Account Number: ";
                if (!(cin >> accNo)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid account number.\n";
                    continue;
                }
                Account* acc = findAccount(accNo);
                if (!acc) {
                    cout << "Account not found!\n";
                    continue;
                }
                switch (choice) {
                    case 1: {
                        double amt;
                        cout << "Enter deposit amount: Rs.";
                        cin >> amt;
                        acc->deposit(amt);
                        break;
                    }
                    case 2: {
                        double amt;
                        cout << "Enter withdrawal amount: Rs.";
                        cin >> amt;
                        acc->withdraw(amt);
                        break;
                    }
                    case 3: acc->display(); break;
                    case 4: acc->displayTransactions(); break;
                    default: cout << "Invalid choice!\n";
                }
            }
        }
    }
};

int main() {
    Bank bank;
    if (bank.login()) {
        bank.run();
    }
    return 0;
}
