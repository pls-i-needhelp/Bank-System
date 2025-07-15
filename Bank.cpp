#include <iostream>
#include <fstream>
#include <cstring>
#include <termios.h>
#include <unistd.h>
#include <ctime>
using namespace std;

char getch_custom() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);              // Save old settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);            // Disable buffering & echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);     // Apply new settings
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);     // Restore old settings
    return ch;
}

void delay(unsigned int mseconds) {
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

void readPassword(char* buffer, int len = 4) {

    // clearing '\n' from buffer
    cin.ignore(1000, '\n');

    cout<<"Enter your 4 digit Pin: ";
    for (int i = 0; i < len; i++) {
        buffer[i] = getch_custom();
        cout << '*';
        delay(100);
    }
    buffer[len] = '\0';
    cout<<endl;
}

class Bank {
public:
    int acno;
    char name[100];
    char pas[10];
    char actype;
    int amt;
    char mob[11];
    char status;
 
    static int getTotalAccounts() {
        Bank b;
        ifstream fin("bankData", ios::binary);
        if (!fin) return 0;
        fin.seekg(0, ios::end);
        return fin.tellg() / sizeof(Bank);
    }

    static void createAccount() {
        Bank b1;
        char pass[10];
        int c = getTotalAccounts();

        ofstream fout("bankData", ios::app | ios::binary);
        if (!fout) {
            cout << "Error opening file."<<endl;
            return;
        }

        cout << "Enter Account Type (c/s): ";
        cin >> b1.actype;
        cout << "Enter Your Name: ";
        cin.ignore();
        cin.getline(b1.name, 50);
        cout << "Enter Amount you want to Deposit: ";
        cin >> b1.amt;
        cout << "Enter Your Mobile No: ";
        cin >> b1.mob;

        b1.acno = c + 1;
        b1.status = 'a';

        if ((b1.actype == 'c' && b1.amt < 10000) || (b1.actype == 's' && b1.amt < 1000)) {
            cout << "Insufficient balance for this account type."<<endl;
            return;
        }
        
        readPassword(pass);
        
        strcpy(b1.pas, pass);

        fout.write(reinterpret_cast<char*>(&b1), sizeof(Bank));
        fout.close();
        cout << "Account Created Successfully. Account No: " << b1.acno << endl;
    }

    static bool authenticate(int acno, Bank &b1) {
        fstream file("bankData", ios::in | ios::out | ios::binary);
        if (!file) return false;

        file.seekg((acno - 1) * sizeof(Bank), ios::beg);
        file.read(reinterpret_cast<char*>(&b1), sizeof(Bank));

        if (b1.status != 'a') {
            cout << "Account is Deactivated."<<endl;
            return false;
        }
        
        fflush(stdin);

        char pass[10];
        readPassword(pass);
        
        if (strcmp(b1.pas, pass) != 0) {
            cout << "Incorrect PIN."<<endl;
            return false;
        }

        return true;
    }

    static void showAccount() {
        Bank b1;
        int n;
        int total = getTotalAccounts();
        cout << "There are " << total << " bank accounts."<<endl;
        cout << "Enter Account Number: ";
        cin >> n;

        if (n <= 0 || n > total) {
            cout << "Invalid Account Number"<<endl;
            return;
        }
        cout<<"=============================================================="<<endl;
        if (authenticate(n, b1)) {
            cout << "Account Number: " << b1.acno << endl << "Name: " << b1.name << endl << "Amount: " << b1.amt << endl << "Mobile: " << b1.mob << endl << "Status: " << b1.status << endl << "Type: " << b1.actype << endl;
        }
    }

    static void depositAccount() {
        Bank b1;
        int n, damt;
        fstream file("bankData", ios::in | ios::out | ios::binary);
        int total = getTotalAccounts();
        cout << "Enter Account Number: ";
        cin >> n;

        if (n <= 0 || n > total) {
            cout << "Invalid Account Number"<<endl;
            return;
        }

        file.seekg((n - 1) * sizeof(Bank), ios::beg);
        file.read(reinterpret_cast<char*>(&b1), sizeof(Bank));

        if (b1.status != 'a') {
            cout << "Account is Deactivated."<<endl;
            return;
        }

        cout << "Enter amount to deposit: ";
        cin >> damt;
        b1.amt += damt;

        file.seekp((n - 1) * sizeof(Bank), ios::beg);
        file.write(reinterpret_cast<char*>(&b1), sizeof(Bank));
        cout << "Amount Deposited Successfully."<<endl;
        file.close();
    }

    static void withdrawAccount() {
        Bank b1;
        int n, wamt;
        fstream file("bankData", ios::in | ios::out | ios::binary);
        int total = getTotalAccounts();
        cout << "Enter Account Number: ";
        cin >> n;

        if (n <= 0 || n > total) {
            cout << "Invalid Account Number"<<endl;
            return;
        }

        file.seekg((n - 1) * sizeof(Bank), ios::beg);
        file.read(reinterpret_cast<char*>(&b1), sizeof(Bank));

        if (b1.status != 'a') {
            cout << "Account is Deactivated."<<endl;
            return;
        }

        cout << "Enter amount to withdraw: "<<endl;
        cin >> wamt;

        if (b1.amt - wamt < (b1.actype == 'c' ? 10000 : 1000)) {
            cout << "Insufficient Balance."<<endl;
            return;
        }

        b1.amt -= wamt;
        file.seekp((n - 1) * sizeof(Bank), ios::beg);
        file.write(reinterpret_cast<char*>(&b1), sizeof(Bank));
        cout << "Amount Withdrawn Successfully."<<endl;
        file.close();
    }

    static void deactivateAccount() {
        changeStatus('d');
    }

    static void activateAccount() {
        changeStatus('a');
    }

    static void changeStatus(char newStatus) {
        Bank b1;
        int n;
        fstream file("bankData", ios::in | ios::out | ios::binary);
        int total = getTotalAccounts();
        cout << "Enter Account Number: ";
        cin >> n;

        if (n <= 0 || n > total) {
            cout << "Invalid Account Number"<<endl;
            return;
        }

        file.seekg((n - 1) * sizeof(Bank), ios::beg);
        file.read(reinterpret_cast<char*>(&b1), sizeof(Bank));

        if (b1.status == newStatus) {
            cout << "Account is already in desired state."<<endl;
            return;
        }

        b1.status = newStatus;
        file.seekp((n - 1) * sizeof(Bank), ios::beg);
        file.write(reinterpret_cast<char*>(&b1), sizeof(Bank));
        cout << "Account status updated."<<endl;
        file.close();
    }

    static void changePassword() {
        Bank b1;
        int n;
        fstream file("bankData", ios::in | ios::out | ios::binary);
        int total = getTotalAccounts();
        cout << "Enter Account Number: ";
        cin >> n;

        if (n <= 0 || n > total) {
            cout << "Invalid Account Number"<<endl;
            return;
        }

        if (!authenticate(n, b1)) return;

        char newpass[10];
        readPassword(newpass);
        strcpy(b1.pas, newpass);

        file.seekp((n - 1) * sizeof(Bank), ios::beg);
        file.write(reinterpret_cast<char*>(&b1), sizeof(Bank));
        cout << "Password Updated Successfully."<<endl;
        file.close();
    }

    static void transferAmount() {
        Bank sender, receiver;
        int n1, n2, amt;
        fstream file("bankData", ios::in | ios::out | ios::binary);
        int total = getTotalAccounts();
        cout << "Enter Sender Account Number: ";
        cin >> n1;
        cout << "Enter Receiver Account Number: ";
        cin >> n2;

        if (n1 <= 0 || n2 <= 0 || n1 > total || n2 > total || n1 == n2) {
            cout << "Invalid account numbers."<<endl;
            return;
        }

        if (!authenticate(n1, sender)) return;

        file.seekg((n2 - 1) * sizeof(Bank), ios::beg);
        file.read(reinterpret_cast<char*>(&receiver), sizeof(Bank));
        if (receiver.status != 'a') {
            cout << "Receiver account is deactivated."<<endl;
            return;
        }

        cout << "Enter amount to transfer: ";
        cin >> amt;

        if (sender.amt - amt < (sender.actype == 'c' ? 10000 : 1000)) {
            cout << "Insufficient Balance."<<endl;
            return;
        }

        sender.amt -= amt;
        receiver.amt += amt;

        file.seekp((n1 - 1) * sizeof(Bank), ios::beg);
        file.write(reinterpret_cast<char*>(&sender), sizeof(Bank));
        file.seekp((n2 - 1) * sizeof(Bank), ios::beg);
        file.write(reinterpret_cast<char*>(&receiver), sizeof(Bank));
        cout << "Amount Transferred Successfully."<<endl;
        file.close();
    }
};

int main() {
    int choice;
    char cont;

    do {
        system("cls");
        cout << "1. Create Account"<<endl<<"2. Show Account"<<endl<<"3. Deposit"<<endl<<"4. Withdraw"<<endl<<"5. Deactivate"<<endl<<"6. Activate /n"<<endl;
        cout << "7. Change Password"<<endl<<"8. Transfer Amount"<<endl<<"Enter any other key to exit"<<endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: Bank::createAccount(); break;
            case 2: Bank::showAccount(); break;
            case 3: Bank::depositAccount(); break;
            case 4: Bank::withdrawAccount(); break;
            case 5: Bank::deactivateAccount(); break;
            case 6: Bank::activateAccount(); break;
            case 7: Bank::changePassword(); break;
            case 8: Bank::transferAmount(); break;
            default: return 0;
        }

        cout << "Do you want to continue? (y/n): ";
        cin >> cont;
    } while (cont == 'y' || cont == 'Y');

    return 0;
}

