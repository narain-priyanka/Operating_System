#pragma once
#include <deque>
#include "Process.h"
#include <fstream>
#include "Scheduler.h"
#include "random_utils.h"


enum TransitionState { 
    TRANS_TO_READY, 
    TRANS_TO_RUN, 
    TRANS_TO_BLOCK, 
    TRANS_TO_PREEMPT, 
    TRANS_TO_TERMINATE
};
string stateToString(ProcessState state) {
    switch(state) {
        case CREATED: return "CREATED";
        case READY: return "READY";
        case RUNNING: return "RUNNING";
        case BLOCK: return "BLOCK";
        case PREEMPTED: return "PREEMPTED";
        case TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}
class Event;
extern deque<Event*> eventQue; 
Process* CURRENT_RUNNING_PROCESS = nullptr;
Process* CURRENT_BLOCKED_PROCESS = nullptr;
Process* Check_for_preemption = nullptr;
ofstream outputFile("verbose_output.txt");
bool hasBeenPremptedByAnotherProcess = false;
extern int totalIO;

class Event {
public:
    int evtTimeStamp;     
    Process* evtProcess;  
    TransitionState transition;  
    
    Scheduler* scheduler; 
    bool verbose;
    int lastEventFT=0;

    Event(int ts, Process* p, TransitionState trans, Scheduler* sched, bool verbose)
        : evtTimeStamp(ts), evtProcess(p), transition(trans), scheduler(sched), verbose(verbose) {}

    void put_event(Event* evt) {
        auto it = eventQue.begin();
        while (it != eventQue.end() && (*it)->evtTimeStamp <= evt->evtTimeStamp) {
            ++it;
        }
        eventQue.insert(it, evt); 
    }

    Event* get_event() {
        if (!eventQue.empty()) {
            Event* nextEvent = eventQue.front();
            eventQue.pop_front();
            return nextEvent;
        }
        return nullptr;
    }
    string getScheduler() const {
        return scheduler->getType();  // Return the scheduler type
    }
    int get_next_event_time(){
        if (eventQue.empty()) {
            return -1;
        }
        Event* nextEvent = eventQue.front();
        return nextEvent->evtTimeStamp;
    }

    

    int getEvtTimeStamp() const { return evtTimeStamp; }
    Process* getEvtProcess() const { return evtProcess; }
    TransitionState getTransition() const { return transition; }

    bool getCurrentEventAtTimeStamp(int timestamp){
        for(auto itr = eventQue.begin(); itr != eventQue.end(); ++itr){
            if((*itr)->evtTimeStamp != timestamp){
                break;
            }
            if((*itr)->evtProcess == CURRENT_RUNNING_PROCESS){
                return true;
            }
        }
        return false;
    }

    void Simulation() {   
        Event* evt;
        // int current_time;
        int arrival_time;
        bool Call_Scheduler = false;
        int randomgeneratedcpuburst;
        int lastIO =0;
        int timeSpent;
        
        while( (evt = get_event()) ) {
            // outputFile <<"===========================================================================================================" << endl;
            // outputFile << "State : " << evt->transition << endl;
            Process *proc = evt->evtProcess;  
            arrival_time = evt->evtTimeStamp;
            TransitionState transition = evt->transition;
            int timeInPrevState = arrival_time - proc->state_ts;

            switch (transition) {
                case TRANS_TO_READY:{
                    if(CURRENT_BLOCKED_PROCESS == proc){
                        CURRENT_BLOCKED_PROCESS = nullptr;
                    }
                    if(proc->getState() == CREATED){
                        proc->setState(READY, arrival_time);
                        if (verbose) {
                        outputFile << arrival_time << " " << proc->getProcessId() << " "
                            << 0 << ": " << "CREATED" 
                            << " -> READY" << endl;
                        }
                    } else if(proc->getState() == BLOCK){
                    
                        // outputFile << "io burst total is " << proc->getIOTime() << endl;
                        proc->setState(READY,arrival_time);

                        if (verbose) {
                        outputFile << arrival_time << " " << proc->getProcessId() << " "
                            << timeInPrevState << ": " <<  "BLOCK"
                            << " -> READY" <<  " and priority = " << proc->dynamicPriority << endl;
                        }
                    } 
                    evt->scheduler->add_process(proc);
                    
                    if (evt->scheduler->getType() == "PREPRIO" && CURRENT_RUNNING_PROCESS != nullptr) {
                        bool cond1 = proc->dynamicPriority > CURRENT_RUNNING_PROCESS->dynamicPriority;
                        bool cond2 = !getCurrentEventAtTimeStamp(evt->evtTimeStamp);

                        outputFile << "---> PRIOpreempt Cond1=" << cond1 << " Cond2=" << cond2 << " (" << cond2 << ") --> ";
                        if (cond1 && cond2) {
                            outputFile << "YES" << endl;
                            
                            timeSpent = arrival_time - CURRENT_RUNNING_PROCESS->state_ts;
                            CURRENT_RUNNING_PROCESS->updateCpuTime(timeSpent);
                            CURRENT_RUNNING_PROCESS->cbRemaining = CURRENT_RUNNING_PROCESS->cbRemaining - timeSpent;
                            CURRENT_RUNNING_PROCESS->totalCPUTimeRemaining -= timeSpent;
                            for (auto itr = eventQue.begin(); itr != eventQue.end(); itr++){
                                Event *currEvent = *itr;
                                if(currEvent->evtProcess->processId == CURRENT_RUNNING_PROCESS->processId) {
                                    eventQue.erase(itr);
                                    break;
                                }
                            }
                            CURRENT_RUNNING_PROCESS->dynamicPriority--;
                            if (verbose) {
                                outputFile << arrival_time << " " << CURRENT_RUNNING_PROCESS->getProcessId() << " " 
                                    << timeSpent << ": " << stateToString(RUNNING) 
                                    << " -> " << stateToString(READY);
                                    
                                outputFile << " cb=" << CURRENT_RUNNING_PROCESS->cpuBurst
                                    << " rem=" << CURRENT_RUNNING_PROCESS->getTotalCPUTimeRemaining() 
                                    << " prio=" << CURRENT_RUNNING_PROCESS->dynamicPriority << " and priority = " << CURRENT_RUNNING_PROCESS->dynamicPriority<< endl;
                            }
                            // outputFile << "Priority preemption. Updated CB remaining: " << CURRENT_RUNNING_PROCESS->cpuBurst << endl;
                                CURRENT_RUNNING_PROCESS->setState(READY, arrival_time);
                                evt->scheduler->add_process(CURRENT_RUNNING_PROCESS);
                                CURRENT_RUNNING_PROCESS = nullptr;
                        } else {
                            outputFile << "NO" << endl;
                        }
                    }
                    // outputFile << "Event queue after scheduling: " << endl;
                    Call_Scheduler = true;
                    break;
                }

                
                case TRANS_TO_PREEMPT:{
                    CURRENT_RUNNING_PROCESS->cbRemaining -= CURRENT_RUNNING_PROCESS->quantum;
                    CURRENT_RUNNING_PROCESS->totalCPUTimeRemaining -= CURRENT_RUNNING_PROCESS->quantum;
                    CURRENT_RUNNING_PROCESS->updateCpuTime(timeInPrevState);
                    CURRENT_RUNNING_PROCESS = nullptr;                     
                    proc->setState(READY, arrival_time);
                    proc->dynamicPriority--;
                    evt->scheduler->add_process(proc); 
                    
                    Call_Scheduler = true;
                    if (verbose) {
                        outputFile << arrival_time << " " << proc->getProcessId() << " " 
                            << timeInPrevState << ": " << stateToString(RUNNING) 
                        << " -> " << stateToString(READY);
                            
                        outputFile << " cb=" << proc->cbRemaining
                            << " rem=" << proc->getTotalCPUTimeRemaining() 
                        << " prio=" << proc->dynamicPriority << " and priority = " << proc->dynamicPriority<< endl;
                    }
                    break;
                }

                case TRANS_TO_RUN: {
                    CURRENT_RUNNING_PROCESS = proc;
                    proc->setState(RUNNING, arrival_time);
                    if(proc->cbRemaining > 0 ){
                        randomgeneratedcpuburst = proc->cbRemaining;
                    }else {
                        randomgeneratedcpuburst = myrandom(proc->cpuBurst);
                        if(proc->getTotalCPUTimeRemaining() < randomgeneratedcpuburst){
                            randomgeneratedcpuburst = proc->getTotalCPUTimeRemaining();
                        }
                        proc->cbRemaining = randomgeneratedcpuburst;
                    }
                    proc->updateCPUWaitTime(timeInPrevState);
                    if (verbose) {
                        outputFile << arrival_time << " " << proc->getProcessId() << " " 
                            << timeInPrevState << ": " << stateToString(READY) 
                            << " -> " << "RUNNING";

                        outputFile << " cb=" << randomgeneratedcpuburst
                            << " rem=" << proc->getTotalCPUTimeRemaining()
                            << " prio=" << proc->dynamicPriority << " and priority = " << proc->dynamicPriority <<endl;
                    }
                  
                    if((evt->scheduler->getType() == "RR" || evt->scheduler->getType() == "PRIO" || evt->scheduler->getType() == "PREPRIO") && proc->cbRemaining > proc->quantum){
                        // outputFile <<"quantum completed by " << CURRENT_RUNNING_PROCESS -> processId << endl;
                        put_event(new Event(arrival_time+proc->getQuantum(), CURRENT_RUNNING_PROCESS, TRANS_TO_PREEMPT, evt->scheduler, verbose));
                    } else if((proc->getCPUTime() + proc->cbRemaining) < proc->getTotalCPUTime()){
                        put_event(new Event(arrival_time +  proc->cbRemaining, CURRENT_RUNNING_PROCESS, TRANS_TO_BLOCK, evt->scheduler, verbose));
                    }else if((proc->getCPUTime() +  proc->cbRemaining) >= proc->getTotalCPUTime()){
                        put_event(new Event(arrival_time +  proc->cbRemaining, CURRENT_RUNNING_PROCESS, TRANS_TO_TERMINATE, evt->scheduler, verbose));
                    } 
                  
                    Call_Scheduler = true;
                    
                    break;
                }
                
                case TRANS_TO_BLOCK: {
                    CURRENT_RUNNING_PROCESS->updateCpuTime(timeInPrevState);
                    CURRENT_RUNNING_PROCESS->totalCPUTimeRemaining -= timeInPrevState;
                    CURRENT_RUNNING_PROCESS->cbRemaining = 0;
                    CURRENT_RUNNING_PROCESS = nullptr;
                    
                    proc->setState(BLOCK, arrival_time);
                    
                    int newIOBurst = myrandom(proc->getIOBurst());
                    proc->updatenewIOburst(newIOBurst); //new line added
                    proc->updateIOTime(newIOBurst); //new line added                
                    put_event(new Event(arrival_time + newIOBurst, proc, TRANS_TO_READY, evt->scheduler, verbose));  //new line added newIOburst instead of ioburst
                    
                    proc->dynamicPriority = proc->staticPriority - 1;
                    if (verbose) {
                        outputFile << arrival_time << " " << proc->getProcessId() << " " << timeInPrevState << ": RUNNING" << " -> " << "BLOCK"
                            << " ib=" << newIOBurst << " rem=" << proc->getTotalCPUTimeRemaining() << " and priority = " << proc->dynamicPriority<< endl; // added new IOburst
                    }

                    if(CURRENT_BLOCKED_PROCESS != nullptr){
                        if((arrival_time + newIOBurst)>lastIO){
                            CURRENT_BLOCKED_PROCESS = proc;
                            totalIO += (newIOBurst+arrival_time)-lastIO;
                            lastIO = newIOBurst+arrival_time;
                        }
                    }else{ 
                        totalIO += newIOBurst;
                        lastIO = newIOBurst+arrival_time; // endtime of last blokc operation
                        CURRENT_BLOCKED_PROCESS = proc;
                    }
                    Call_Scheduler = true;
                    break;
                }

                case TRANS_TO_TERMINATE: {
                    proc->setState(TERMINATED, arrival_time);
                    proc->FT = arrival_time;
                    proc->TT = proc->FT - proc->arrivalTime;
                    CURRENT_RUNNING_PROCESS = nullptr;
                    lastEventFT = arrival_time;
                    Call_Scheduler=true;
                    if (verbose) {
                        outputFile << arrival_time << " " << proc->processId << " " << timeInPrevState << ": " 
                            << "DONE" << " " << "FT=" << proc->FT << " " << "TT=" << proc->TT << endl;
                    }
                    break;
                }
            }
            
            if (Call_Scheduler) {
                if(get_next_event_time() == arrival_time){
                    continue;
                }
                Call_Scheduler = false;
                if (CURRENT_RUNNING_PROCESS == nullptr) {
                    CURRENT_RUNNING_PROCESS = evt->scheduler->get_next_process(); // pick the process from ready queu
                    if (CURRENT_RUNNING_PROCESS == nullptr) {
                        continue;
                    }
                    evt->put_event(new Event(arrival_time, CURRENT_RUNNING_PROCESS, TRANS_TO_RUN, evt->scheduler, verbose));
                }
            }
            
        }
    }
};