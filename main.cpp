#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <deque>
#include <vector>
#include <unistd.h>  
#include <iomanip>
#include "Scheduler.h"
#include "Process.h"
#include "Event.h"
#include "random_utils.h"

using namespace std;

deque<Event*> eventQue;
vector<int> randomNumbers;
vector<Process*> processes;
int ofs = 1;
int totalIO=0;
int totalCPUTime = 0;

int myrandom(int burst) {
    if (ofs == randomNumbers.size()) {
        ofs = 1;
    }
    int randomNum = randomNumbers[ofs];
    ofs++;
    int num = 1 + (randomNum % burst);
    return 1 + (randomNum % burst);
}

// This function calculates and formats the final output
void printFinalOutput(const vector<Process*>& processes, Scheduler* scheduler, int quantum) {
    if (quantum !=0){ 
    // if (scheduler->getType() == "RR" || scheduler->getType() == "PRIO" || scheduler->getType() == "PREPRIO") {
        cout << scheduler->getType() << " " << quantum << endl;
    }else{
        cout << scheduler->getType()<<endl;
    }

    int lastEventFT = 0;
    double totalCPUTime = 0.0;
    double totalIOTime = 0.0;
    double avgTurnaroundTime = 0.0;
    double avgCpuWaitTime = 0.0;
    double cpuUtilization =0.0;
    double ioUtilization=0.0;

    // Per-process information
    for (const auto& proc : processes) {
        cout << setw(4) << setfill('0') << proc->getProcessId() << ": ";
        cout <<setw(4)<< setfill(' ') << proc->arrivalTime << " "
             <<setw(4)<< setfill(' ')<< proc->totalCPUTime << " "
             <<setw(4)<< setfill(' ')<< proc->cpuBurst << " "
             <<setw(4)<< setfill(' ')<< proc->ioBurst << " "
             <<setw(4)<< setfill(' ')<< proc->staticPriority << " | "
             <<setw(4)<< setfill(' ')<< proc->FT << " "
             <<setw(4)<< setfill(' ')<< proc->TT << " "
             <<setw(4)<< setfill(' ')<< proc->IT << " "
             <<setw(4)<< setfill(' ')<< proc->CW << endl;

        // Aggregate metrics
        lastEventFT = std::max(lastEventFT, proc->FT);
        totalCPUTime += proc->getTotalCPUTime();
        avgTurnaroundTime += proc->TT;
        avgCpuWaitTime += proc->CW;
    }

    // Summary calculations
    int processCount = processes.size();
    avgTurnaroundTime /= processCount;
    avgCpuWaitTime /= processCount;
    cpuUtilization = (totalCPUTime / lastEventFT) * 100.0;
    ioUtilization = (static_cast<double>(totalIO) / lastEventFT) * 100.0;

    double throughput = processCount / (lastEventFT / 100.0);

    // Print summary
    cout << "SUM: " << lastEventFT << " "
         << fixed << setprecision(2) << cpuUtilization << " "
         << fixed << setprecision(2) << ioUtilization << " "
         << fixed << setprecision(2) << avgTurnaroundTime << " "
         << fixed << setprecision(2) << avgCpuWaitTime << " "
         << fixed << setprecision(3) << throughput << endl;
}


void readRandomFile(const string& randFile) {
    ifstream file(randFile);
    if (!file) {
        cout << "Error: Unable to open file " << randFile << endl;
        return;
    }

    int num;
    while (file >> num) {
        randomNumbers.push_back(num);
    }
    file.close();
}

// Read the input file
void readFile(const string& inputFile, int maxprio, Scheduler* scheduler, int quantum, bool verbose) {
    ifstream file(inputFile);
    if (!file) {
        cout << "Error: Unable to open file " << inputFile << endl;
        return;
    }

    string line;
    int processId = 0;    
    int arrivalTime, totalCPUTime, cpuBurst, ioBurst, staticPriority, dynamicPriority;

    while (getline(file, line)) {
        istringstream iss(line);
        if (iss >> arrivalTime >> totalCPUTime >> cpuBurst >> ioBurst) {
            staticPriority = myrandom(maxprio); 
            dynamicPriority = staticPriority - 1;  // Initialize dynamic priority
            
            Process* newProcess = new Process(processId,arrivalTime, totalCPUTime, cpuBurst, ioBurst, staticPriority, dynamicPriority, quantum);
            processes.push_back(newProcess);
            Event* arrivalEvent = new Event(arrivalTime, newProcess, TRANS_TO_READY, scheduler, verbose); // Scheduler passed here
            arrivalEvent->put_event(arrivalEvent); // Insert event in event queue
            processId++;
        } else {
            cout << "Error: Invalid line format: " << line << endl;
        }
    }
    file.close();
}

// Function to parse the command-line arguments
void parser(int argc, char* argv[], int& quantum, int& maxprio, Scheduler*& scheduler) {
    int c;
    string sched;
    char schedType;
    bool verbose = false;
    maxprio = 4;

    // getopt to read command line arguments
    while ((c = getopt(argc, argv, "hvteps:")) != -1) {
        switch (c) {
            case 'h':  // Help flag
                cout << "Help: Usage <program> [-h] [-v] [-t] [-e] [-p] [-s<schedspec>] inputfile randfile\n";
                exit(0);
            case 'v':  // Verbose flag
                verbose = true; // if it is in command line arg
                break;
            case 's':  // Scheduler specification
                sscanf(optarg, "%c%d:%d", &schedType, &quantum, &maxprio);  // Parse quantum and maxprio from optarg
                if (schedType == 'F') {
                    scheduler = new FCFS_Scheduler();
                } else if (schedType == 'L') {
                    scheduler = new LCFS_Scheduler();
                } else if (schedType == 'S') {
                    scheduler = new SRTF_Scheduler();
                } else if (schedType == 'R') {
                    scheduler = new RR_Scheduler();
                } else if (schedType == 'P') {
                    scheduler = new PRIO_Scheduler(maxprio);
                } else if (schedType == 'E'){
                    scheduler = new PREPRIO_Scheduler(maxprio);
                } else {
                    std::cerr << "Error: Unknown scheduler type" << std::endl;
                    exit(1);
                }
                break;
            default:
                exit(1);
        }
    }

    if (optind < argc) {
        string randfile = argv[optind + 1];
        readRandomFile(randfile);  // Load random numbers first

        string inputfile = argv[optind];
        
        readFile(inputfile, maxprio, scheduler, quantum, verbose);  // Then read input file
        // cout << verbose << endl;
    } else {
        cerr << "Missing inputfile and randfile\n";
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    int quantum = 0;  // Default for quantum
    int maxprio = 4;  // Default maxprio is 4
    Scheduler* scheduler = nullptr; // create a interface ptr to which schedtype will be assigned

    parser(argc, argv, quantum, maxprio, scheduler);

    if (!eventQue.empty()) {
        Event* evt = eventQue.front();
            evt->Simulation();  // Use regular Simulation for other scheduler 
    }
    printFinalOutput(processes, scheduler, quantum);
    return 0;
};



