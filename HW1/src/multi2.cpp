#include <map>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <mutex>
#include <thread>
#include <chrono>

using namespace std;

class Bank {
public:
    Bank(int num_accounts) {
        const int total_balance = 100000;
        totalBalance = 100000.0f;
        int base_amount = total_balance / num_accounts;
        int remainder = total_balance % num_accounts;

        for(int i = 1; i <= num_accounts; ++i) {
            float balance = static_cast<float>(base_amount);
            if(i == num_accounts) {  
                balance += static_cast<float>(remainder);
            }
            accounts[i] = balance;
            accountLocks[i];
            ids.push_back(i);
        }
    }

    vector<int> get_two_random_ids() {
        thread_local static random_device rd;
        thread_local static mt19937 gen(rd());
        uniform_int_distribution<int> distIndex(0, ids.size() - 1);
        int i = distIndex(gen);
        int j = distIndex(gen);
        while (j == i) {
            j = distIndex(gen);
        }
        return {ids[i], ids[j]};
    }

    void deposit(int src_id, int tgt_id, float amount) {

        unique_lock<mutex> globalLock(balanceMutex);

        if (src_id < tgt_id) {
            unique_lock<mutex> lockA(accountLocks[src_id], defer_lock);
            unique_lock<mutex> lockB(accountLocks[tgt_id], defer_lock);
            lock(lockA, lockB); 
            accounts[src_id] -= amount;
            accounts[tgt_id] += amount;
        } else {
            unique_lock<mutex> lockA(accountLocks[tgt_id], defer_lock);
            unique_lock<mutex> lockB(accountLocks[src_id], defer_lock);
            lock(lockA, lockB);
            accounts[src_id] -= amount;
            accounts[tgt_id] += amount;
        }
    }

    float balance() {
        unique_lock<std::mutex> lock(balanceMutex);

        return totalBalance;
    }
    //write a getter for accounts
    void print_accounts() {
        for (auto& [id, bal] : accounts) {
            cout << "Account " << id << " balance: " << bal << endl;
        }
    }

private:
    map<int, float> accounts;
    map<int, mutex> accountLocks;
    vector<int> ids;
    mutex balanceMutex;
    float totalBalance;
};

float get_random_amount() {
    thread_local static random_device rd;
    thread_local static mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, 1000);
    return static_cast<float>(dis(gen));
}

void thread_work(Bank& bank, int iterations) {
    thread_local static random_device rd;
    thread_local static mt19937 gen(rd());
    uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < iterations; ++i) {
        double r = dist(gen);
        if (r < 0.95) {
            auto two = bank.get_two_random_ids();
            bank.deposit(two[0], two[1], get_random_amount());
        } else {
            bank.balance();
        }
    }
}

int main(int argc, char* argv[]) {
    int nAccounts = 100; 
    if (argc > 1) {
        int parsed = std::atoi(argv[1]);
        if (parsed > 0) {
            nAccounts = parsed;
        } else {
            std::cerr << "Invalid number of accounts. Using default (" 
                      << nAccounts << ").\n";
        }
    }
    int num_threads = 4;
    int iterations = 100000;

    Bank bank(nAccounts);

    vector<thread> threads;

    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            thread_work(bank, iterations);
        });
    }

    for (auto& t : threads) {
        t.join();
    }
    auto end = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Multi-threaded execution time (" << num_threads << " threads): " 
              << duration.count() << " ms\n";
    cout << "Final balance: " << bank.balance() << endl;
    return 0;
}