#include <map>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <thread>
#include <future>

thread_local static std::mt19937 gen{ std::random_device{}() };
thread_local std::uniform_int_distribution<int> dis(0, 1000);

float get_random_amount() {
    return static_cast<float>(dis(gen));
}

class Bank_single_thread {
public:
    Bank_single_thread() = default;
    
    Bank_single_thread(int n, float initBalance) {
        for (int i = 1; i <= n; ++i) {
            accounts[i] = initBalance;
            ids.push_back(i);
        }
    }

    std::vector<int> get_two_random_ids() {
        thread_local std::random_device rd;
        thread_local std::mt19937 gen(rd());

        std::vector<int> shuffled = ids;
        std::shuffle(shuffled.begin(), shuffled.end(), gen);
        return {shuffled[0], shuffled[1]};
    }

    void deposit(int src_id, int tgt_id, float amount) {
        if (src_id == tgt_id) return; // no-op if same account
        accounts[src_id] -= amount;
        accounts[tgt_id] += amount;
    }

    float balance() {
        float total = 0.0f;
        for (auto &[id, bal] : accounts) {
            total += bal;
        }
        return total;
    }

private:
    std::map<int, float> accounts; 
    std::vector<int>     ids;
};

void single_thread_work(Bank_single_thread &bank, int iterations) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < iterations; ++i) {
        double r = dist(gen);
        if (r < 0.95) {
            auto two = bank.get_two_random_ids();
            float amount = get_random_amount();
            bank.deposit(two[0], two[1], amount);
        } else {
            bank.balance();
        }
    }
}
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
    std::vector<int> get_two_random_ids() {
        std::random_device rd;   
        std::mt19937 gen(rd());  

        std::vector<int> ids_cpy = ids;
        std::shuffle(ids_cpy.begin(), ids_cpy.end(), gen);
        return {ids_cpy[0], ids_cpy[1]};
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
        std::vector<int> ids;
        ids.reserve(accounts.size());
        for (auto &kv : accounts) {
            ids.push_back(kv.first);
        }
        std::sort(ids.begin(), ids.end());

        std::vector<std::unique_lock<std::mutex>> locks;
        locks.reserve(ids.size());
        for (int id : ids) {
            locks.emplace_back(accountLocks[id]);
        }

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

double thread_work(Bank &myBank, int iterations)
{
    // measure only the time of actual deposit/balance loops
    auto start = std::chrono::high_resolution_clock::now();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < iterations; ++i) {
        double r = dist(gen);
        if (r < 0.95) {
            auto two = myBank.get_two_random_ids();
            float amount = get_random_amount();
            myBank.deposit(two[0], two[1], amount);
        } else {
            myBank.balance();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    return ms; 
}

int main(int argc, char* argv[]) {
    int iterations = 100000;
    int n = 2;  
    std::vector<double> times(n, 0.0);
    if (argc > 1) {
        int parsed = std::atoi(argv[1]);
        if (parsed > 0) {
            n = parsed;
        } else {
            std::cerr << "Invalid number of threads. Using default (" << n << ").\n";
        }
    }


    Bank myBank(10, 10000.0f);

    std::vector<std::thread> threads;
    threads.reserve(n);

    for (int i = 0; i < n; ++i) {
        threads.emplace_back([&myBank, iterations, &times, i]() {
            double ms = thread_work(myBank, iterations);
            times[i] = ms;
        });
    }

    for (auto &t : threads) {
        t.join();
    }
    double multi_thread_times = 0.0;
    for (double t : times) {
        multi_thread_times += t;
    }

    float balance_multi = myBank.balance();

    std::cout << n << "-threaded " << " ========> " << multi_thread_times << " ms\n";





    Bank_single_thread bank_single(10, 10000.0f);
    int iterations_single = 100000*n;

    auto start = std::chrono::high_resolution_clock::now();
    single_thread_work(bank_single, iterations_single);
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    float balance_single = bank_single.balance();

    std::cout << "1-threaded " << " ========> " << elapsed_ms << " ms\n";

    std::cout << "-----------------------------------------\n";
    std::cout << "Final balance multi: " << balance_multi << "\n";
    std::cout << "Final balance single: " << balance_single << "\n";
    return 0;
}
