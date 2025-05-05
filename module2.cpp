#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <queue>  
#include <vector>
#include "module2.h"
#include <sys/wait.h>
#include <SFML/System/Clock.hpp>
#include <chrono>
using namespace std;
mutex pushFlightToQ;
///////////////////////////////////////////////////////CONSTANTS/////////////////////////////////////////////////////////
const int MAX_FLIGHTS = 14; //could be changed later
//global
/////declare airlines according to project statement
sf::Clock simTime;
mutex timeLock;
int getTime(){
std::lock_guard<std::mutex> lock(timeLock); 
return int(simTime.getElapsedTime().asSeconds());
}
struct ComparePriority {
	bool operator()(const Flight* f1, const Flight* f2) const {
		if (f1->priority == f2->priority) {
		        if(f1->aircraft && f2->aircraft){
		          if (f1->aircraft->fuelLevel < f2->aircraft->fuelLevel) {
				return f1->aircraft->fuelLevel > f2->aircraft->fuelLevel;
			    }
		        }
			
			return f1->flightDuration > f2->flightDuration;  // FCFS within same priority
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
const int TOTAL_SECONDS = 300; // 5 minutes
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void defaultFlights();
void flightInitialiser();
void* flightSimulator(void* args);
void* pushFlightsToQueue(void* arg);
void* flightThread(void* arg);
void allocateRunway(Flight& flight);
void* manageFlightQueues(void * arg);
void progressFlightArrival(Flight* f);
void progressFlightDeparture(Flight* f);
int main() {

	cout << "Do you want to use our default flight information for this simulation?  (y/n) ";
	char ch;
	cin >> ch;
	while (!(ch == 'y' || ch == 'Y' || ch == 'n' || ch == 'N')) {
		cout << "Invalid value entered. Try again. " << endl;
		cin >> ch;
	}
	int flightCount = 6; //default value
	(ch == 'n' || ch == 'N') ? flightInitialiser() : defaultFlights();
        simTime.restart(); //start sim
       pthread_t flightSimulatorThread;
       pthread_create(&flightSimulatorThread, NULL, flightSimulator, NULL);

     for(int seconds = 0; seconds <= TOTAL_SECONDS; seconds++) {
        // Clear line
        std::cout << "\r";
        
        // Calculate minutes and remaining seconds
        int minutes = seconds / 60;
        int remainingSeconds = seconds % 60;
        
        // Display time in MM:SS format
     //   std::cout << minutes << ":" <<  remainingSeconds;
        
       // std::cout << std::flush;
        
        // Wait for 1 second
        if (seconds < TOTAL_SECONDS) {
            sleep(1);
        }
    }
    
  
   std::cout << "\n\nTimer completed!\n";
 
	return 0;
}

void* flightSimulator(void* args) {

	pthread_t thr_flight[flightCount]; //an array of flight threads
	pthread_t manageFlightsThread;
	
	pthread_create(&manageFlightsThread, NULL, manageFlightQueues, NULL);
	
	for (int i = 0; i < flightCount; i++) {
		int result = pthread_create(&thr_flight[i], NULL, flightThread, (void*)flights[i]);
		if (result != 0) {
			cerr << "Error creating thread " << i << endl;
			exit(EXIT_FAILURE);
		}
	}
	//check conflict function
	//reschedule Runways function
	//reroute function
	
	for (int i = 0; i < flightCount; i++) {
		pthread_join(thr_flight[i], NULL);
	}
    return nullptr;
}
///////////////////////////////////////////////////////FUNCTION DEFINITIONS////////////////////////////////////////////////////////////////////

void defaultFlights() {
	flights = new Flight * [flightCount]; //6
	flights[0] = new Flight("NWEIW-09", "Holding", 0, "North", false, 1, false, true, 2, 45, "Cargo", 2);

	flights[1] = new Flight("WRCXQ-02", "Holding", 0, "South", false, 2, false, false, 2, 54, "Cargo", 2);
	flights[2] = new Flight("BOINQ-87", "AtGate", 0, "East", false, 3, true, false, 3, 63, "Commercial", 0); //arrival

	flights[3] = new Flight("SPO1O-98", "AtGate", 0, "East", false, 4, true, true, 1, 75, "Emergency", 5); //an emergency
	flights[4] = new Flight("AMNXA-9Q", "AtGate", 100, "West", true, 5, true, true, 2, 84, "Cargo", 4); //arrival

	flights[5] = new Flight("QJHSX-12", "AtGate", 0, "East", false, 6, true, true, 1, 25, "Emergency", 5);
        
       
	//pushFlightsToQueue();
	pthread_t thr_pushToQueue;
	pthread_create(&thr_pushToQueue, NULL, pushFlightsToQueue, NULL);
	

	return;
}


void flightInitialiser() {
	//input flights
	cout << "How many flights do you want in this simulation? " << endl;
	cin >> flightCount;
	while (flightCount < 0 || flightCount > MAX_FLIGHTS) {
		cout << "Invalid flight count. Please choose a positive number less than " << MAX_FLIGHTS << endl;
		cin >> flightCount;
	}

	flights = new Flight * [flightCount];
	for (int i = 0; i < flightCount; i++) {
		flights[i] = new Flight();
	}
	pthread_t thr_pushToQueue;
	pthread_create(&thr_pushToQueue, NULL, pushFlightsToQueue, NULL);
	//pushFlightsToQueue();
	return;
}
void* pushFlightsToQueue(void* arg) {

	//sort flights array according to schedule time
	for (int i = 0; i < flightCount; i++) {
		for (int j = 0; j < flightCount - i - 1; j++) {
			if (flights[j + 1]->scheduleTime < flights[j]->scheduleTime) {
				Flight* temp = flights[j + 1];
				flights[j + 1] = flights[j];
				flights[j] = temp;
			}
		}
	}
	 for(int i = 0; i < flightCount; i ++){
        cout << YELLOW << "Flight number: " << flights[i]->flightNumber << " schedule time: " << flights[i]->scheduleTime<< RESET << endl;
      //  flights[i]->display();
        }
	//cout << "Flights array sorted" << endl;
        int pushedFlights = 0;
        bool* pushed = new bool[flightCount];
for (int i = 0; i < flightCount; i++) pushed[i] = false;
          while (pushedFlights < flightCount) {
              for (int i = 0; i < flightCount; i++) {
                  if (pushed[i]) continue;
                  
                  
                      if (flights[i]->isDeparture) {
                      sleep(flights[i]->scheduleTime);
                      departureQueue.push(flights[i]);
          //                cout << "Pushed a departure flight" << endl;
                      } else {
                      sleep(flights[i]->scheduleTime);
                          arrivalQueue.push(flights[i]);
            //               cout << "Pushed an arrival flight" << endl;
                      }
                      pushed[i] = true;
                      pushedFlights++;
                  
              }
          }//while loop bracket

            
	/*
  //push without waiting
  for(int i = 0; i < flightCoun}t; i ++){
	  if(flights[i]  && flights[i]->isDeparture){
	  int index = 0;
	  for (int j = 0; j < flightCount; j++) {
		  if (flights[j]->scheduleTime < flights[index]->scheduleTime) {
			  index = j;
		  }
	  }
		  departureQueue.push(flights[index]);
	  }
	  else{
	  int index = 0;
	  for (int j = 0; j < flightCount; j++) {
		  if (flights[j]->scheduleTime < flights[index]->scheduleTime) {
			  index = j;
		  }
	  }
		  arrivalQueue.push(flights[i]);
	  }
  }
  */
	return nullptr;
}

///////////////thread functions /////////////////////////////////
void* flightThread(void* arg) {
	Flight* flight = (Flight*)arg;



	if (flight->isDeparture) {
		while (flight->phase != "Climb");
	}
	else {
		while (flight->phase != "AtGate");
	}

	pthread_exit(NULL);

}

void* handleFlight(void* arg) {
	Flight* flight = ((Runways*)arg)->flight;
	//cout << "reached handle flight function";
	if (!flight) {
		cout << "No flights on the runway\n";
		return nullptr;
	}

	//	 coutMutex.lock();
		// Lock before accessing shared resources
	{
		//cout << "Flight " << flight->flightNumber << " ("	<< (flight->isDeparture ? "Departure" : "Arrival") << ") processing at time " << flight->scheduleTime << endl;
	}
	//    coutMutex.unlock();
	// Simulate flight phases
	int flightTimeDivision;
	bool departureFlag = 0;
	if (flight->isDeparture == 1) {
		flightTimeDivision = flight->flightDuration / 4;
		departureFlag = 1;
	}
	else {
		flightTimeDivision = flight->flightDuration / 5;
		departureFlag = 0;
	}

	if (departureFlag == 1) {
		for (int i = 0; i < 4; i++) {

			//coutMutex.lock();
			
			sleep(flightTimeDivision);
			cout << "Flight " << flight->flightNumber << " progressing through phase " << flight->phase << " at time " << getTime() << endl; 
			//<< flight->scheduleTime + (i + 1) * flightTimeDivision<< endl;
			//	coutMutex.unlock();

			//while (simTime.getElapsedTime().asSeconds() > flight->scheduleTime + (i + 1) * flightTimeDivision);
			progressFlightDeparture(flight);

		}

	}
	else if (departureFlag == 0) {
		for (int i = 0; i < 5; i++) {
			//coutMutex.lock();
			sleep(flightTimeDivision);
			cout << "Flight " << flight->flightNumber	<< " progressing through phase " << flight->phase << " at time " << getTime() << endl; 
			//flight->scheduleTime + (i + 1) * flightTimeDivision << endl;
			//	coutMutex.unlock();
			
			//while (simTime.getElapsedTime().asSeconds() > flight->scheduleTime + (i + 1) * flightTimeDivision);
			progressFlightArrival(flight);
		}
	}
	
	//coutMutex.lock();
	cout << "Flight " << flight->flightNumber << "  Terminating .. " << endl;
	((Runways*)arg)->freeRunway();
	///coutMutex.unlock();
	if(RWYA.flight == nullptr){
	//cout << "Runway A is now free " << endl;
	RWYA.runwayAbeingUsed.unlock();
	}
	if(RWYB.flight == nullptr){
	//cout << "Runway B is now free " << endl;
	RWYB.runwayBbeingUsed.unlock();
	}
	if (RWYC.flight == nullptr){
	//cout << "Runway C is now free " << endl;
	RWYC.runwayCbeingUsed.unlock();
	}
	return nullptr;
}

pthread_t runwayAThread, runwayBThread, runwayCThread;

void allocateRunway(Flight* flight) {
	 
	if (flight->priority == 1 || flight->isFacingEmergency()) {
	//preemption
	/*if(RWYC.flight != nullptr){
    
		Flight* preemptedFlight = RWYC.flight;
		RWYC.freeRunway();
		preemptedFlight->flightDuration = preemptedFlight->scheduleTime - simTime.getElapsedTime().asSeconds();

		if (preemptedFlight->isDeparture) {
			departureQueue.push(preemptedFlight);
		}
		else {
			arrivalQueue.push(preemptedFlight);
		}

		RWYC.runwayCbeingUsed.unlock();
        }*/
		RWYC.runwayCbeingUsed.lock();
		
		std::cout << GREEN <<"Allocating " << RWYC.getID() << " for emergency or overflow flight " << flight->flightNumber << " at time: " << getTime() << RESET << "\n";
		/*if (simTime.getElapsedTime().asSeconds() + 1 > flight->scheduleTime) {
			flight->waitingTime = simTime.getElapsedTime().asSeconds() - flight->scheduleTime;
			cout << "Flight number " << flight->flightNumber << " waited for " << flight->waitingTime << endl;
			cout << "Flight number " << flight->flightNumber << " rescheduled for " << int(simTime.getElapsedTime().asSeconds()) << endl;
			flight->scheduleTime = int(simTime.getElapsedTime().asSeconds());
		}*/
	      

		RWYC.useRunway(flight);
		pthread_create(&runwayCThread, NULL, handleFlight, (void*)&RWYC);
		
                //RWYC.runwayCbeingUsed.unlock();
		return;
	}
	else if (flight->getDirection() == "North" || flight->getDirection() == "South") {
		RWYA.runwayAbeingUsed.lock();
		
		std::cout << GREEN <<"Allocating " << RWYA.getID() << " for arrival flight " << flight->flightNumber << " at time: " << getTime()<< RESET <<"\n";
		/*if (simTime.getElapsedTime().asSeconds() > flight->scheduleTime) {
			flight->waitingTime = simTime.getElapsedTime().asSeconds() - flight->scheduleTime;
			cout << "Flight number " << flight->flightNumber << " waited for " << flight->waitingTime << endl;
			cout << "Flight number " << flight->flightNumber << " rescheduled for " << int(simTime.getElapsedTime().asSeconds()) << endl;
			flight->scheduleTime = int(simTime.getElapsedTime().asSeconds());
		}*/

		RWYA.useRunway(flight);
		pthread_create(&runwayCThread, NULL, handleFlight, (void*)&RWYA);
		//RWYA.runwayAbeingUsed.unlock();
	}
	else if (flight->getDirection() == "East" || flight->getDirection() == "West") {
	
		RWYB.runwayBbeingUsed.lock();
	      
		std::cout<< GREEN << "Allocating " << RWYB.getID() << " for departure flight " << flight->flightNumber << " at time: " << getTime()<< RESET <<"\n";
		if (simTime.getElapsedTime().asSeconds() > flight->scheduleTime) {
			flight->waitingTime = simTime.getElapsedTime().asSeconds() - flight->scheduleTime;
			cout << "Flight number " << flight->flightNumber << " waited for " << flight->waitingTime << endl;
			cout << "Flight number " << flight->flightNumber << " rescheduled for " << int(simTime.getElapsedTime().asSeconds()) << endl;
			flight->scheduleTime = int(simTime.getElapsedTime().asSeconds());
		}

		RWYB.useRunway(flight);
		pthread_create(&runwayCThread, NULL, handleFlight, (void*)&RWYB);
		
		//RWYB.runwayBbeingUsed.unlock();
	}
	else {
		RWYC.runwayCbeingUsed.lock();
	      
		std::cout << GREEN << "Allocating " << RWYC.getID() << " for emergency or overflow flight " << flight->flightNumber << " at time: " << getTime() << RESET <<"\n";
		RWYC.useRunway(flight);
		pthread_create(&runwayCThread, NULL, handleFlight, (void*)&RWYC);
		
		//RWYC.runwayCbeingUsed.unlock();
		return;

	}
}

void* manageFlightQueues(void* arg) {
//use time here 
	while (true) {
		while (!arrivalQueue.empty() || !departureQueue.empty()) {
			if (!arrivalQueue.empty()) { 
				Flight* flight = arrivalQueue.top();
				arrivalQueue.pop();
				allocateRunway(flight);
				cout << flight->flightNumber << " SCHEDULED! " << endl;
			}

			if (!departureQueue.empty()) {
				Flight* flight = departureQueue.top();
				
				departureQueue.pop();
				allocateRunway(flight);
				cout << flight->flightNumber << " SCHEDULED! " << endl;
			}
		}
	}

	pthread_exit(NULL);
	return nullptr;
}
