#include <map>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <cstring>
using namespace std;



class Bank {
public:
    Bank(int num_accounts) {
        const int total_balance = 100000;
        int base_amount = total_balance / num_accounts;
        int remainder = total_balance % num_accounts;

        for(int i = 1; i <= num_accounts; ++i) {
            float balance = static_cast<float>(base_amount);
            if(i == num_accounts) {  
                balance += static_cast<float>(remainder);
            }
            accounts[i] = balance;
            ids.push_back(i);
        }
    }

    vector<int> get_two_random_ids() {
        static random_device rd;
        static mt19937 gen(rd());
        uniform_int_distribution<int> distIndex(0, ids.size() - 1);
        int i = distIndex(gen);
        int j = distIndex(gen);
        while (j == i) {
            j = distIndex(gen);
        }
        return {ids[i], ids[j]};
    }

    void deposit(int src_id, int tgt_id, float amount) {
        accounts[src_id] -= amount;
        accounts[tgt_id] += amount;
    }

    float balance() {
        return totalBalance;
    }

private:
    map<int, float> accounts;
    vector<int> ids;
    float totalBalance = 100000.0f;
};

float get_random_amount() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, 1000);
    return static_cast<float>(dis(gen));
}

void single_thread_job(Bank& bank, int iterations) {
    static random_device rd;
    static mt19937 gen(rd());
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
    int numAccounts = 100; 
    int iterations = 100000;
    
    for (int i = 1; i < argc; ++i) {
        if (i + 1 >= argc) {
            std::cerr << "Missing value after " << argv[i] << std::endl;
            return 1;
        }
        if (std::strcmp(argv[i], "--accounts") == 0) {
            numAccounts = std::atoi(argv[++i]);
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
    // int total_iterations = iterations * numThreads;
    int total_iterations = 16 * iterations;
    Bank bank(numAccounts);
    for (int i = 0; i < k; ++i) {
        auto start = chrono::high_resolution_clock::now();
        single_thread_job(bank, total_iterations);
        auto end = chrono::high_resolution_clock::now();

        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        totalTime += duration.count();
    }

    double avgTime = totalTime / k;
    cout << avgTime;
    return 0;
}