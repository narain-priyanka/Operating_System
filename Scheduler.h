#pragma once
#include <deque>
#include <vector>
#include "Process.h"
#include <list> 

using namespace std;

class Scheduler {
    public:
        virtual void add_process(Process* p) =0;
        virtual Process* get_next_process() =0;
        // virtual bool test_preempt(Process* activated_process) = 0;
        virtual string getType() const = 0;
        virtual ~Scheduler() {} ;
        
};


// implementing all the algorithms
Process* objptrToProcess;
extern Process* CURRENT_RUNNING_PROCESS ;

class FCFS_Scheduler : public Scheduler {
    deque<Process*> readyQueue;
    void add_process(Process* p) override {
        readyQueue.push_back(p); 
    }

    Process* get_next_process() override {
        if (!readyQueue.empty()) {
            Process* next = readyQueue.front(); 
            readyQueue.pop_front(); 
            return next;
        }
        return nullptr; 
    }
    string getType() const override { return "FCFS"; }
};

//LCFS
class LCFS_Scheduler: public Scheduler{
    deque<Process*> readyQueue;
    void add_process(Process* p) override {
        readyQueue.push_back(p); 
    }

    Process* get_next_process() override {
        if (!readyQueue.empty()) {
            Process* next = readyQueue.back(); 
            readyQueue.pop_back(); 
            return next;
        }
        return nullptr; 
    }
    string getType() const override { return "LCFS"; }
};

// SRTF
class SRTF_Scheduler : public Scheduler{
    list<Process*> readyQueue;
    void add_process(Process* p) override {
        readyQueue.push_back(p); 
        readyQueue.sort([](const Process* a, const Process* b) {
            return a->getTotalCPUTimeRemaining() < b->getTotalCPUTimeRemaining();
        });
    }

    Process* get_next_process() override {
        if (!readyQueue.empty()) {
            Process* next = readyQueue.front(); 
            readyQueue.pop_front(); 
            return next;
        }
        return nullptr; 
    }
    string getType() const override { return "SRTF"; }
};

// RR
class RR_Scheduler : public Scheduler{
    deque<Process*> readyQueue;
    void add_process(Process* p) override {
        readyQueue.push_back(p); 
    }

    Process* get_next_process() override {
        if (!readyQueue.empty()) {
            Process* next = readyQueue.front(); 
            readyQueue.pop_front(); 
            return next;
        }
        return nullptr; 
    }
    string getType() const override { return "RR"; }
};

// PRIO : non-preemptive
class PRIO_Scheduler : public Scheduler{
    protected:
    std::deque<std::vector<Process*>> readyQueue;
    std::deque<std::vector<Process*>> expiredQueue;
    int maxprio;

    public:
        PRIO_Scheduler(int maxprio) : maxprio(maxprio) {
            readyQueue.resize(maxprio);
            expiredQueue.resize(maxprio);
        }
    void print_ready_and_expired_queue() {
        outputFile << "Ready Queue Contents:" << endl;
        for (int prio = maxprio - 1; prio >= 0; --prio) {
            outputFile << "Priority " << prio << ": ";
            if (readyQueue[prio].empty()) {
                outputFile << "empty" << endl;
            } else {
                for (auto process : readyQueue[prio]) {
                    outputFile << "Process ID: " << process->getProcessId() << " ";
                }
                outputFile << endl;
            }
        }
        outputFile << "Expired Queue Contents:" << endl;
        for (int prio = maxprio - 1; prio >= 0; --prio) {
            outputFile << "Priority " << prio << ": ";
            if (expiredQueue[prio].empty()) {
                outputFile << "empty" << endl;
            } else {
                for (auto process : expiredQueue[prio]) {
                    outputFile << "Process ID: " << process->getProcessId() << " ";
                }
                outputFile << endl;
            }
        }
    }

    void add_process(Process* p) override {
       if (p->dynamicPriority >= 0) {
            readyQueue[p->dynamicPriority].push_back(p);
        } else {
            p->dynamicPriority = p->staticPriority - 1; 
            expiredQueue[p->dynamicPriority].push_back(p);
        }
    }

    Process* get_next_process() override {
        for (int prio = maxprio - 1; prio >= 0; --prio) {
            if (!readyQueue[prio].empty()) {
                Process* next = readyQueue[prio].front();
                readyQueue[prio].erase(readyQueue[prio].begin()); 
                return next;
            }
        }

        if (is_empty()) {
        std::swap(readyQueue, expiredQueue);
        if (is_empty()) {
            return nullptr; 
        }

        return get_next_process(); 
    }

    return nullptr; 
    }
    string getType() const override { return "PRIO"; }

    bool is_empty() const {
        for (int prio = 0; prio < maxprio; ++prio) {
            if (!readyQueue[prio].empty()) {
                return false;
            }
        }
        return true;
    }

};

// PREPRIO - preemptive
class PREPRIO_Scheduler : public Scheduler{
    protected:
        std::deque<std::vector<Process*>> readyQueue;
        std::deque<std::vector<Process*>> expiredQueue;
        int maxprio;
        

    public:
        PREPRIO_Scheduler(int maxprio) : maxprio(maxprio) {
            readyQueue.resize(maxprio);
            expiredQueue.resize(maxprio);
        }   

        void print_ready_and_expired_queue() {
            outputFile << "Ready Queue Contents:" << endl;
            for (int prio = maxprio - 1; prio >= 0; --prio) {
                outputFile << "Priority " << prio << ": ";
                if (readyQueue[prio].empty()) {
                    outputFile << "empty" << endl;
                } else {
                    for (auto process : readyQueue[prio]) {
                        outputFile << "Process ID: " << process->getProcessId() << " ";
                    }
                    outputFile << endl;
                }
            }
            outputFile << "Expired Queue Contents:" << endl;
            for (int prio = maxprio - 1; prio >= 0; --prio) {
                outputFile << "Priority " << prio << ": ";
                if (expiredQueue[prio].empty()) {
                    outputFile << "empty" << endl;
                } else {
                    for (auto process : expiredQueue[prio]) {
                        outputFile << "Process ID: " << process->getProcessId() << " ";
                    }
                    outputFile << endl;
                }
            }
        }

        bool test_preempt(Process *activated_process)  {
            if (CURRENT_RUNNING_PROCESS != nullptr) {
                return activated_process->dynamicPriority < CURRENT_RUNNING_PROCESS->dynamicPriority;
            }
            return false;
        }


        // Method to add processes and check for preemption
        void add_process(Process* p) override {
            // Adding process to readyQueue if dynamic priority is still >= 0
            if (p->dynamicPriority >= 0) {
                readyQueue[p->dynamicPriority].push_back(p);
            } else {
                // If dynamicPriority is < 0, move it to expiredQueue and reset dynamicPriority
                p->dynamicPriority = p->staticPriority - 1;
                expiredQueue[p->dynamicPriority].push_back(p);
            }
            // print_ready_and_expired_queue();
        }

        Process* get_next_process() override {
            // Check the readyQueue for processes starting from the highest priority (maxprio - 1) to the lowest (0)
            for (int prio = maxprio - 1; prio >= 0; --prio) {
                if (!readyQueue[prio].empty()) {
                    Process* next = readyQueue[prio].front();
                    readyQueue[prio].erase(readyQueue[prio].begin());  // Remove from the queue
                    return next;
                }
            }

            // If readyQueue is empty, swap expiredQueue with readyQueue
            if (is_empty()) {
                outputFile << "swap occured" << endl;
                std::swap(readyQueue, expiredQueue);

                // After swapping, if both queues are still empty, return nullptr
                if (is_empty()) {
                    return nullptr;
                }

                return get_next_process();  // Recheck after swapping
            }

            return nullptr;  // No process found
        }

        string getType() const override { return "PREPRIO"; }

        // Helper function to check if readyQueue is empty
        bool is_empty() const {
            for (int prio = 0; prio < maxprio; ++prio) {
                if (!readyQueue[prio].empty()) {
                    return false;
                }
            }
            return true;
        }

        // Preemption checker (optional, you may handle preemption inside `add_process`)
       
};
