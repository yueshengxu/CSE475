#include <map>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <thread>
#include <future>

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
void deposit(std::map<int, float>& bank, int src_id, int tgt_id, float amount, std::mutex &bankMutex) {
    std::lock_guard<std::mutex> lock(bankMutex);

    bank[src_id] -= amount;
    bank[tgt_id] += amount;

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
    std::map<int, float> bank;
    std::mutex bankMutex;

    bank.insert(std::pair(1, 10000.0f));
    bank.insert(std::pair(2, 10000.0f));
    bank.insert(std::pair(3, 10000.0f));
    bank.insert(std::pair(4, 10000.0f));
    bank.insert(std::pair(5, 10000.0f));
    bank.insert(std::pair(6, 10000.0f));
    bank.insert(std::pair(7, 10000.0f));
    bank.insert(std::pair(8, 10000.0f));
    bank.insert(std::pair(9, 10000.0f));
    bank.insert(std::pair(10, 10000.0f));

    std::vector<int> ids = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> two_ids = get_two_random_ids(ids);

                             
    std::vector<std::thread> threads(n);          
    std::vector<double> results(n, 0);             

    for (int i = 0; i < n; ++i) {
        threads[i] = std::thread([&bank, &bankMutex, iterations, &results, i]() {
            double exec_time_i = do_work(bank, bankMutex, iterations);
            results[i] = exec_time_i;
        });
    }

    for (auto &t : threads) {
        t.join();
    }
    float b = balance(bank, bankMutex);
    std::cout << "Total balance across all accounts: " << b << '\n';
    std::cout << "Collected results:\n";
    for (int i = 0; i < n; ++i) {
        std::cout << "Thread " << i << " result = " << results[i] << "\n";
    }
    
    return 0;
}
