#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <queue>  
#include <vector> 
#include "module2.h"
#include <sys/wait.h>
using namespace std;
///////////////////////////////////////////////////////CONSTANTS/////////////////////////////////////////////////////////
const int MAX_FLIGHTS = 14; //could be changed later
//global 
/////declare airlines according to project statement

struct ComparePriority {
    bool operator()(const Flight* f1, const Flight* f2) const {
        if (f1->priority == f2->priority) {
            return f1->scheduledTime > f2->scheduledTime;  // FCFS within same priority
        }
        return f1->priority > f2->priority;  // Lower number = higher priority
    }
};


std::mutex queueMutex; // Add this near your queue declarations

// queues for arrivals and departures
std::priority_queue<Flight*, std::vector<Flight*>, ComparePriority> arrivalQueue;
std::priority_queue<Flight*, std::vector<Flight*>, ComparePriority> departureQueue;
Flight** flights;
int flightCount = 6; //default value
RunwayA RWYA;
RunwayB RWYB;
RunwayC RWYC;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void defaultFlights();
void flightInitialiser();
void flightSimulator();
void pushFlightsToQueue();
void* flightThread(void* arg);
void allocateRunway(Flight& flight);
void manageFlightQueues();
int main(){
    cout << "This code is meant to be turned into a simulation later" << endl;

    cout << "Do you want to use our default flight information for this simulation?  (y/n) ";
    char ch;
    cin >> ch;
    while(!(ch == 'y'|| ch == 'Y'|| ch  == 'n'|| ch  == 'N')){
      cout << "Invalid value entered. Try again. " << endl;
      cin >> ch;
    }
    int flightCount = 6; //default value
    (ch == 'n' || ch == 'N')? flightInitialiser(): defaultFlights();
    flightSimulator();
return 0;
}

void flightSimulator() {
    pthread_t thr_flight[flightCount];
    
    for(int i = 0; i < flightCount; i++) {
        int result = pthread_create(&thr_flight[i], NULL, flightThread, (void*)flights[i]);
        if (result != 0) {
            cerr << "Error creating thread " << i << endl;
            exit(EXIT_FAILURE);
        }
    }
    //check conflict function
    //reschedule Runways function
    //reroute function
    manageFlightQueues();


  
    for(int i = 0; i < flightCount; i++) {
        pthread_join(thr_flight[i], NULL);
    }
}
///////////////////////////////////////////////////////FUNCTION DEFINITIONS////////////////////////////////////////////////////////////////////

void defaultFlights(){
    flights = new Flight*[flightCount]; //6
    flights[0] = new Flight("NWEIW-09", "AtGate", 0 ,"North", false, 1, true, true, 2, 46, "Cargo", 2);
    flights[1] = new Flight("WRCXQ-02", "AtGate", 0 ,"South",  false, 2, true, false, 2, 34, "Cargo", 2);
    flights[2] = new Flight("BOINQ-87", "Holding", 500 , "North", false, 3, false , false, 3, 20, "Commercial", 0); //arrival
    flights[3] = new Flight("SPO1O-98", "AtGate", 0 , "East", false, 4, true, true, 1, 60, "Emergency", 5); //an emergency
    flights[4] = new Flight("AMNXA-9Q", "Landing", 100 ,"West", true , 5, false, true, 2, 34, "Cargo", 4); //arrival
    flights[5] = new Flight("QJHSX-12", "AtGate", 0 ,"North",  false, 6, true, true, 3, 76, "Commercial", 1);
    pushFlightsToQueue();
return;
}


void flightInitialiser(){
//input flights
    cout << "How many flights do you want in this simulation? " << endl;
    cin >> flightCount;
    while(flightCount < 0 || flightCount > MAX_FLIGHTS){
    cout << "Invalid flight count. Please choose a positive number less than " << MAX_FLIGHTS << endl;
    cin >> flightCount;
    }
  
    flights = new Flight*[flightCount];
    for(int i = 0; i < flightCount ; i++){
    flights[i] = new Flight();
    }
    pushFlightsToQueue();
return;
}
void pushFlightsToQueue(){
    for(int i = 0; i < flightCount; i ++){
        if(flights[i]  && flights[i]->isDeparture){
            departureQueue.push(flights[i]);
        }
        else{
            arrivalQueue.push(flights[i]);
        }
    }
}

///////////////thread functions /////////////////////////////////
void* flightThread(void* arg) {
    Flight* flight = (Flight*)arg;
    
    // Lock before accessing shared resources
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        cout << "Flight " << flight->flightNumber << " (" 
             << (flight->isDeparture ? "Departure" : "Arrival")
             << ") processing..." << endl;
    }

    // Simulate flight phases
    for (int i = 0; i < 10; ++i) {
        usleep(500000); // 0.5s delay between phases
        
        std::lock_guard<std::mutex> lock(queueMutex);
        cout << "Flight " << flight->flightNumber
             << " progressing through phase " << i << endl;
    }
    cout << "FLight " << flight->flightNumber << "Terminating .. " << endl;
    pthread_exit(NULL);
}
mutex RunwayA_RunwayLock;
void allocateRunway(Flight* flight) {
    std::lock_guard<std::mutex> lock(RunwayA_RunwayLock);  // Locking the Runway
    if (flight->getDirection() == "North" || flight->getDirection() == "South") {
        std::cout << "Allocating " << RWYA.getID() << " for arrival flight " << flight->flightNumber << "\n";
    } else if (flight->getDirection() == "East" || flight->getDirection() == "West") {
        std::cout << "Allocating " << RWYB.getID() << " for departure flight " << flight->flightNumber << "\n";
    } else {
        std::cout << "Allocating " << RWYC.getID() << " for emergency or overflow flight " << flight->flightNumber << "\n";
    }
}

void manageFlightQueues() {
    while (!arrivalQueue.empty() || !departureQueue.empty()) {
        if (!arrivalQueue.empty()) {
            Flight* flight = arrivalQueue.top();
            arrivalQueue.pop();
            allocateRunway(flight);
        }

        if (!departureQueue.empty()) {
            Flight* flight = departureQueue.top();
            departureQueue.pop();
            allocateRunway(flight);
        }
    }
}
