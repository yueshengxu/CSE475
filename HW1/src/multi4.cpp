#include <map>
#include <vector>
#include <mutex>
#include <thread>
#include <random>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <cstring>
using namespace std;

struct Transaction {
    int src_id;
    int tgt_id;
    float amount;
};

class Bank {
public:
    Bank(int n) : totalBalance(100000.0f) {
        num_accounts = n;
        int base_amount = 100000 / num_accounts;
        int remaining_amount = 100000 % num_accounts;
        for (int i = 1; i <= num_accounts; ++i) {
            float balance = static_cast<float>(base_amount);
            if (i == num_accounts) {
                balance += remaining_amount;
            }
            accounts[i] = balance;
            ids.push_back(i);
        }
    }

    pair<int,int> get_two_random_ids() {
        thread_local static random_device rd;
        thread_local static mt19937 gen(rd());
        
        uniform_int_distribution<size_t> dist(0, num_accounts-1);
        int i = dist(gen);
        int j = dist(gen);
        while (j == i) {
            j = dist(gen);
        }
        return {ids[i], ids[j]};
    }

    void deposit(const vector<Transaction> &transactions) {
        unique_lock<mutex> lock(bankMutex);
        for (auto &t : transactions) {
            accounts[t.src_id] -= t.amount;
            accounts[t.tgt_id] += t.amount;
            totalBalance += t.amount;
            totalBalance -= t.amount;
        }
    }

    float balance() {
        unique_lock<mutex> lock(bankMutex);
        return totalBalance;
    }

private:
    unordered_map<int, float> accounts;
    vector<int> ids;
    int num_accounts;
    float totalBalance;
    mutex bankMutex;
};

float get_random_amount() {
    thread_local static random_device rd;
    thread_local static mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, 1000);
    return static_cast<float>(dis(gen));
}

void thread_work_with_batch(Bank &bank, int iterations, int batch_size) {
    thread_local static random_device rd;
    thread_local static mt19937 gen(rd());
    uniform_real_distribution<double> dis(0.0, 1.0);
    vector<Transaction> localBuffer;
    localBuffer.reserve(batch_size);
    for (int i = 0; i < iterations; i++) {
        double r = dis(gen);
        if (r < 0.95) {
            auto [src, tgt] = bank.get_two_random_ids();
            Transaction t{src, tgt, get_random_amount()};
            localBuffer.push_back(t);
            if (static_cast<int>(localBuffer.size()) >= batch_size) {
                bank.deposit(localBuffer);
                localBuffer.clear();
            }
        } else {
            bank.balance();
        }
    }
    if (!localBuffer.empty()) {
        bank.deposit(localBuffer);
        localBuffer.clear();
    }
}

int main(int argc, char* argv[]) {
    int numAccounts = 100;
    int numThreads = 4;
    int iterations = 100000;
    int batch_size = 400;
    int total_iterations = 8 * iterations;


    for (int i = 1; i < argc; ++i) {
        // Make sure we don't go out of bounds when we do `argv[i+1]`.
        if (i + 1 >= argc) {
            std::cerr << "Missing value after " << argv[i] << std::endl;
            return 1;
        }

        if (std::strcmp(argv[i], "--batch_size") == 0) {
            batch_size = std::atoi(argv[++i]);
        }
        else if (std::strcmp(argv[i], "--accounts") == 0) {
            numAccounts = std::atoi(argv[++i]);
        }
        else if (std::strcmp(argv[i], "--threads") == 0) {
            numThreads = std::atoi(argv[++i]);
        }
        else if (std::strcmp(argv[i], "--iterations") == 0) {
            iterations = std::atoi(argv[++i]);
        }
        else {
            std::cerr << "Unknown argument: " << argv[i] << std::endl;
            return 1;
        }
    }

    int k = 10;
    double totalTime = 0.0;

    vector<int> batchSizes(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        batchSizes[i] = batch_size * (i + 1);
    }
    for (int i = 0; i < k; ++i) {
        Bank bank(numAccounts);
        vector<thread> threads;
        threads.reserve(numThreads);
        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([&]() {
                thread_work_with_batch(bank, total_iterations/numThreads, batchSizes[i]);
            });
        }
        for (auto &t : threads) {
            t.join();
        }
        auto end = chrono::high_resolution_clock::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        totalTime += ms;
    }

    double avgTime = totalTime / k;
    cout << avgTime;
}
