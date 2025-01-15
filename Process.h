#pragma once
#include <algorithm> // For std::min
#include <fstream>
#include "random_utils.h"  // For myrandom
using namespace std;
extern ofstream outputFile;

// Define the states using an enumeration
enum ProcessState {
    CREATED,
    READY,
    RUNNING,
    BLOCK,
    PREEMPTED,
    TERMINATED
};
extern int randomgeneratedcpuburst;
// Modify the Process class to include the state
class Process {
public:
    int processId;
    int totalCPUTime;
    int arrivalTime;
    int cpuBurst;
    int ioBurst;
    int staticPriority;
    int dynamicPriority;
    int quantum;
    int FT; // Finish Time
    int TT; // Turnaround Time
    int IT; // Total time the process spent in the BLOCKED state (I/O time)
    int CW; // Total time the process spent in the READY state (waiting time)
    int cpuTime; // Total CPU time consumed by the process
    int totalCPUTimeRemaining; // Total CPU time remaining
    int cbRemaining; // CPU burst remaining
    int newIOBurst;
    ProcessState currentState;  // Current state of the process
    int state_ts; // Timestamp of the state change
    

    // Constructor
    Process(int pid, int at, int tcp, int cb, int io, int sp, int dp, int quantum)
        : processId(pid), arrivalTime(at), totalCPUTime(tcp), cpuBurst(cb), ioBurst(io),
          staticPriority(sp), dynamicPriority(dp), quantum(quantum), currentState(ProcessState::CREATED),
          FT(0), TT(0), IT(0), CW(0), cpuTime(0), totalCPUTimeRemaining(tcp), cbRemaining(0), newIOBurst(0){}

    // Getters for process attributes
    int getProcessId() const { return processId; }
    int getTotalCPUTime() const { return totalCPUTime; }
    int getArrivalTime() const { return arrivalTime; }
    int getStaticPriority() const { return staticPriority; }
    int getDynamicPriority() { return dynamicPriority; }
    int getIOBurst() const { return ioBurst; }
    int getQuantum() const { return quantum; }

    // Update the total time the process has spent in CPU (used after RUNNING state)
    void updateCpuTime(int timeConsumed) {
            cpuTime += timeConsumed; // Add the time consumed in RUNNING state to total CPU time
    }

    int getCPUTime() const {
        return cpuTime;
    }

    // Update the total waiting time when the process is in READY state
    void updateCPUWaitTime(int timeInPrevState) {   
            CW += timeInPrevState; // Add the waiting time in READY state
    }

    // Update the total I/O time when the process is in BLOCKED state
    void updateIOTime(int newIOBurst) {
            IT += newIOBurst; // Add the time spent in BLOCKED state
    }
    int getIOTime(){
        return IT;
    }
    // Update the dynamic priority based on the current state
    void updateDynamicPriority() {
            dynamicPriority--;
             outputFile << " it sometimes enter here: " << dynamicPriority << endl;
            if (dynamicPriority < 0) {
                outputFile << " it sometimes enter here: " << dynamicPriority << endl;
                dynamicPriority = staticPriority - 1;
            }
            // outputFile << "Dynamic priority updated to: " << dynamicPriority << endl;
    }

    // Get total CPU time remaining
    int getTotalCPUTimeRemaining() const {
        return totalCPUTime - cpuTime;
    }

    // Set the process state and update the timestamp of the state change
    void setState(ProcessState newState, int currentTime) {
        currentState = newState;
        state_ts = currentTime;
    }

    // Get current process state
    ProcessState getState() const {
        return currentState;
    }

    // Update the total CPU time remaining (based on consumed CPU time)
    // void updateTotalCPUTimeRemaining() {
    //     totalCPUTimeRemaining = totalCPUTime - cpuTime;
    // }

    // NEW CODE ADDED 
    void updatenewIOburst(int newIOburst){
        newIOBurst = newIOburst;
    }
    int getnewIOBurst(){
        return newIOBurst;
    }
    // NEW CODE ENDS

    void updateCPUBurst(int randomgeneratedcpuburst){
        cpuBurst = randomgeneratedcpuburst;
    }

    int getCPUBurst() const {
        // outputFile << "cpuburust is " << cpuBurst<< endl;
        return cbRemaining;
    }

    int getCBRemaining(){
        return cbRemaining;
    }
};
