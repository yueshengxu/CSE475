#include <map>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <thread>
#include <future>


class Bank {
public:
    Bank() = default;
    Bank(int n, float balance) {
        for (int i = 1; i <= n; ++i) {
            accounts[i] = balance;
            accountLocks[i]; 
            ids.push_back(i);
        }
    }
    void deposit(int src_id, int tgt_id, float amount) {
        if (src_id < tgt_id) {

            std::unique_lock<std::mutex> lockA(accountLocks[src_id], std::defer_lock);
            std::unique_lock<std::mutex> lockB(accountLocks[tgt_id], std::defer_lock);
            lockA.lock(); 
            lockB.lock();
            accounts[src_id] -= amount;
            accounts[tgt_id] += amount;
            lockA.unlock(); 
            lockB.unlock(); 

        }else {
            std::unique_lock<std::mutex> lockA(accountLocks[tgt_id], std::defer_lock);
            std::unique_lock<std::mutex> lockB(accountLocks[src_id], std::defer_lock);
            lockA.lock(); 
            lockB.lock();
            accounts[src_id] -= amount;
            accounts[tgt_id] += amount;
            lockA.unlock(); 
            lockB.unlock(); 
        }   
    }
    float balance() {
        // Lock all accounts in ascending order
        std::vector<int> ids;
        ids.reserve(accounts.size());
        for (auto &kv : accounts) {
            ids.push_back(kv.first);
        }
        std::sort(ids.begin(), ids.end());

        // Lock each
        std::vector<std::unique_lock<std::mutex>> locks;
        locks.reserve(ids.size());
        for (int id : ids) {
            locks.emplace_back(accountLocks[id]);
        }

        // Compute sum
        float sum = 0.0f;
        for (int id : ids) {
            sum += accounts[id];
        }

        return sum;
    }

private:
    std::map<int, float>        accounts;
    std::map<int, std::mutex>   accountLocks;
    std::vector<int> ids;
};
void print_balances(std::map<int, float>& bank) {
    std::cout << "\nBalances:\n";
    for (auto &[id, balance] : bank) {
        std::cout << "  Account " << id << ": " << balance << '\n';
    }
}
std::vector<int> get_two_random_ids(std::vector<int> ids) {
    static std::random_device rd;   
    static std::mt19937 gen(rd());  

    std::shuffle(ids.begin(), ids.end(), gen);
    return {ids[0], ids[1]};
}
float get_random_amount() {
    static std::random_device rd;   
    static std::mt19937 gen(rd());  

    static std::uniform_int_distribution<int> dis(0, 1000);
    return static_cast<float>(dis(gen));

}
void deposit(std::map<int, float>& bank, int src_id, int tgt_id, float amount, std::map<int, std::mutex> accountLocks) {
    std::unique_lock<std::mutex> lock_src(accountLocks[src_id], std::defer_lock); 
    std::unique_lock<std::mutex> lock_tgt(accountLocks[tgt_id], std::defer_lock); 
    lock_src.lock(); 
    lock_tgt.lock();
    bank[src_id] -= amount;
    bank[tgt_id] += amount;
    lock_src.unlock(); 
    lock_tgt.unlock(); 
}

float balance(std::map<int, float> &bank, std::mutex &bankMutex) {
    std::lock_guard<std::mutex> lock(bankMutex);
    float sum = 0.0f;
    for (auto &[id, balance] : bank) {
        sum += balance;
    }
    return sum;
}

double do_work(std::map<int, float> &bank, std::mutex &bankMutex, int iterations)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::vector<int> ids = {1, 2, 3, 4};

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        double r = dist(gen);
        if (r < 0.95) {
            std::vector<int> two_ids = get_two_random_ids(ids);
            float amount = get_random_amount();
            deposit(bank, two_ids[0], two_ids[1], amount, bankMutex);

        } else {
            balance(bank, bankMutex);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto exec_time_i = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    return exec_time_i;
}
int main(int argc, char* argv[]) {
    int iterations = 100000;
    int n = 2;  

    if (argc > 1) {
        int parsed = std::atoi(argv[1]);
        if (parsed > 0) {
            n = parsed;
        } else {
            std::cerr << "Invalid number of threads. Using default (" << n << ").\n";
        }
    }


    Bank myBank(10, 10000.0f);


    
    return 0;
}
