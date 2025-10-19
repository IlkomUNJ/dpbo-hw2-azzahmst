#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <limits>
using namespace std;

struct Transaction {
    string buyer, seller, item;
    int qty;
    double total;
    string status;
    string date;
};

struct Item {
    string name;
    double price;
    int stock;
    int sold;
};

struct BankAccount {
    string owner;
    double balance;
    vector<pair<string, double>> history;
};

class Bank {
public:
    map<string, BankAccount> accounts;

    void createAccount(const string& name) {
        if (accounts.count(name)) {
            cout << "[BANK] Akun udah ada.\n";
            return;
        }
        accounts[name] = {name, 0, {}};
        cout << "[BANK] Akun bank buat " << name << " berhasil dibuat.\n";
    }

    void topup(const string& name, double amount) {
        if (!accounts.count(name)) {
            cout << "[BANK] Akun ga ditemukan.\n";
            return;
        }
        accounts[name].balance += amount;
        accounts[name].history.push_back({"credit", amount});
        cout << "[BANK] Topup " << amount << " ke " << name << endl;
    }

    void withdraw(const string& name, double amount) {
        if (!accounts.count(name)) {
            cout << "[BANK] Akun ga ditemukan.\n";
            return;
        }
        if (accounts[name].balance < amount) {
            cout << "[BANK] Saldo ga cukup.\n";
            return;
        }
        accounts[name].balance -= amount;
        accounts[name].history.push_back({"debit", -amount});
        cout << "[BANK] " << name << " tarik saldo " << amount << endl;
    }

    void listCustomers() {
        cout << "=== Daftar Nasabah ===\n";
        for (auto &p : accounts)
            cout << "- " << p.first << endl;
    }
};

class Store {
public:
    Bank* bank;
    map<string, map<string, Item>> sellers;
    vector<Transaction> transactions;

    Store(Bank* b) { bank = b; }

    static string currentDate() {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        char buf[20];
        sprintf(buf, "%04d-%02d-%02d", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
        return string(buf);
    }

    void registerSeller(const string& name) {
        if (!bank->accounts.count(name)) {
            cout << "[STORE] Seller harus punya akun bank dulu.\n";
            return;
        }
        if (sellers.count(name)) {
            cout << "[STORE] Seller udah terdaftar.\n";
            return;
        }
        sellers[name] = {};
        cout << "[STORE] Seller " << name << " berhasil didaftarkan.\n";
    }

    void addItem(const string& seller, const string& item, double price, int stock) {
        if (!sellers.count(seller)) {
            cout << "[STORE] Seller belum terdaftar.\n";
            return;
        }
        sellers[seller][item] = {item, price, stock, 0};
        cout << "[STORE] Item " << item << " berhasil ditambah.\n";
    }

    void purchase(const string& buyer, const string& seller, const string& item, int qty) {
        if (!bank->accounts.count(buyer) || !bank->accounts.count(seller)) {
            cout << "[STORE] Akun bank buyer/seller tidak ditemukan.\n";
            return;
        }
        if (!sellers.count(seller) || !sellers[seller].count(item)) {
            cout << "[STORE] Item tidak tersedia.\n";
            return;
        }

        Item& it = sellers[seller][item];
        double total = it.price * qty;
        if (it.stock < qty) { cout << "[STORE] Stok kurang.\n"; return; }
        if (bank->accounts[buyer].balance < total) { cout << "[STORE] Saldo buyer kurang.\n"; return; }

        bank->withdraw(buyer, total);
        bank->topup(seller, total);
        it.stock -= qty; it.sold += qty;
        transactions.push_back({buyer, seller, item, qty, total, "paid", currentDate()});
        cout << "[STORE] Transaksi berhasil: " << buyer << " beli " << qty << " " << item << " dari " << seller << endl;
    }

    void listTransactions() {
        cout << "=== Semua Transaksi ===\n";
        for (auto &t : transactions)
            cout << t.date << " | " << t.buyer << " -> " << t.seller
                 << " | " << t.item << " | " << t.total << " | " << t.status << endl;
    }

    void topItems() {
        map<string,int> count;
        for (auto &t : transactions) count[t.item] += t.qty;
        vector<pair<string,int>> v(count.begin(), count.end());
        sort(v.begin(), v.end(), [](auto&a,auto&b){return a.second>b.second;});
        cout << "=== Top Item ===\n";
        for (auto &p:v) cout << p.first << " : " << p.second << " terjual\n";
    }
    
    void saveData() {
        ofstream f("data.txt");
        if (!f) {
            cout << "[ERROR] Gagal buat file data.txt\n";
            return;
        }
        for (auto &t : transactions) {
            f << t.date << "|" << t.buyer << "|" << t.seller << "|" 
              << t.item << "|" << t.qty << "|" << t.total << "|" << t.status << "\n";
        }
        f.close();
        cout << "[STORE] Data transaksi disimpan ke data.txt\n";
    }
};

void menu() {
    Bank bank;
    Store store(&bank);
    int choice;

    while (true) {
        cout << "\n=== MENU ===\n";
        cout << "1. Buat akun bank\n2. Topup\n3. Withdraw\n4. Daftar seller\n5. Tambah item\n";
        cout << "6. Beli item\n7. Lihat transaksi\n8. Top item\n9. Daftar nasabah\n10. Keluar\n";
        cout << "Pilih: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            string n; cout << "Nama: "; cin >> n; bank.createAccount(n);
        } else if (choice == 2) {
            string n; double a; cout << "Nama: "; cin >> n; cout << "Jumlah: "; cin >> a; bank.topup(n,a);
        } else if (choice == 3) {
            string n; double a; cout << "Nama: "; cin >> n; cout << "Jumlah: "; cin >> a; bank.withdraw(n,a);
        } else if (choice == 4) {
            string n; cout << "Nama seller: "; cin >> n; store.registerSeller(n);
        } else if (choice == 5) {
            string s,i; double p; int st; cout << "Seller: "; cin >> s; cout << "Item: "; cin >> i;
            cout << "Harga: "; cin >> p; cout << "Stok: "; cin >> st; store.addItem(s,i,p,st);
        } else if (choice == 6) {
            string b,s,i; int q; cout << "Buyer: "; cin >> b; cout << "Seller: "; cin >> s;
            cout << "Item: "; cin >> i; cout << "Qty: "; cin >> q; store.purchase(b,s,i,q);
        } else if (choice == 7) {
            store.listTransactions();
        } else if (choice == 8) {
            store.topItems();
        } else if (choice == 9) {
            bank.listCustomers();
        } else if (choice == 10) {
            store.saveData();
            cout << "Keluar...\n"; 
            break;
        } else {
            cout << "Pilihan ga valid.\n";
        }
    }
}

int main() {
    menu();
    return 0;
}