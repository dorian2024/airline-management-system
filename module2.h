#ifndef MYHEADER_H  // Header guard (prevents double inclusion)
#define MYHEADER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <queue>  
#include <vector> 
using namespace std;
std::mutex aircraftMutex;



///////////////////////////////////////////////////CLASSES///////////////////////////////////////////////////////////////////

//------------------------------------------aircraft class--------------------
class Aircraft {
  public:
    bool AVNStatus; // initially false
    int id;
    bool isAvailable;
    Aircraft(int anID) : id(anID), AVNStatus(false), isAvailable(true) {}
    virtual string getType() = 0;  //to make it abstract class
    
};

//derived aircraft classes:
class CargoAircraft : public Aircraft {
    string Type;
    public:
    CargoAircraft(int id) : Aircraft(id)  {
        Type = "Cargo";
    }
    string getType() override {
        return Type;
    }
};

class CommercialAircraft : public Aircraft {
    string Type;
    public:
    CommercialAircraft(int id) : Aircraft(id) {
        Type = "Commercial";
    }
    string getType() override {
        return Type;
    }
};

class EmergencyAircraft : public Aircraft {
    string Type;
    public:
    EmergencyAircraft(int id) : Aircraft(id) {
        Type = "Emergency";
    }
    string getType() override {
        return Type;
    }
};


//------------------------airline class ------------------------------
class Airline{ //can also be a struct
    string AirlineName;
    string Type;
    int aircraftCount;
    int Flights; //number of active flights
    vector <Aircraft*> ac;
    public:
    Airline(string n, int t, int c, int f): AirlineName(n), aircraftCount(c), Flights(f){
        switch (t){
                  case 1 : Type = "Commercial";
                  break;
                  case 2: Type = "Military"; 
                  break;
                  case 3: Type = "Cargo";
                  break;
                  case 4: Type = "Medical";
                  break;
                  
                  for(int i = 0; i < aircraftCount; i++){
                        if(Type == "Medical" || "Type" == "Military"){
                        ac.push_back(new EmergencyAircraft(i));
                        }
                        else if(Type == "Cargo"){
                        ac.push_back(new CargoAircraft(i));
                        }
                        else if(Type == "Commercial"){
                        ac.push_back(new CommercialAircraft(i));
                        }
                  }
            }
      }
      Aircraft* assignAircraftToFlight(){
          std::lock_guard<std::mutex> lock(aircraftMutex); // locks the mutex until function exits

        if (!ac.empty()) { //if its not empty
              for(int i = 0; i < aircraftCount; i++){
                    if(ac[i]->isAvailable){
                          ac[i]->isAvailable = false;  // mark as in use
                          return ac[i];
                      }
              }
        
              return ac[0];  // Assign the first one for simplicity
        }
          return nullptr; 
                          
       
      }
    virtual void display() {
        cout << "Airline info: " << endl << "Name: " << AirlineName << endl << "Type: " << Type << endl;
        cout << "Flights: " << Flights << endl << "Aircrafts: " << aircraftCount << endl;
    }
};

extern Airline airlines[6] = {
            Airline("PIA", 1 , 6, 4), 
            Airline("AirBlue", 1 , 4, 4), 
            Airline("FedEx", 3, 3, 2), 
            Airline("Pakistan Airforce", 2, 2, 1), 
            Airline("Blue Dart", 3, 2, 2), 
            Airline("AghaKhan Air Ambulance", 4, 2, 1)
};
//------------------------flight class ----------------------
class Flight{
////////////////////////////////////////////////////////////
/*
phase: //Holding, Approach, Landing, AtGate, Taxi, TakeoffRoll, Climb, Cruise
REMOVED flighttype: //InternationalArrivals, DomesticArrivals, InternationalDepartures, DomesticDepartures
direction: //north, south, east, west
priorities: (1 for Emergency, 2 for Cargo, 3 for Commercial): 4 for exceptional cases
*/
protected: //they all have to be inherited but unchangeable by other classes 
        string phase; 
        int speed;
        string direction; 
        bool emergency; //yes or no
        
       
        int airlineID;
        Airline* airline;
        //aircraft type and airline are attributes of aircraft 
        
    public:
    int flightNumber;
    string id;
     bool isDeparture; //simple yes or no attributes
        bool isDomestic; //
        int priority;
        int scheduledTime;  // in minutes since start
        Aircraft* aircraft;
    Flight(string i, string ph, int sp, string dir, bool emegc, int flightNum, bool isDepart, bool isDom,  int pty, int time, string aircraftType, int airlineID){
          emergency = false;
          phase = ph;
          id = i;
          speed = sp;
          direction = dir;
          emergency = emegc;
          flightNumber = flightNum;
          isDeparture = isDepart;
          isDomestic = isDom;
          priority = pty;
          scheduledTime = time;
          this->airlineID = airlineID;
          if(airlineID < 0 || airlineID > 5){
          cout << "ERROR OCCURED IN AIRLINE ID";
          }
          else{
          airline = &airlines[airlineID];
          }
          
          aircraft = airline->assignAircraftToFlight();
          if (aircraftType == "Cargo") {
              //this->aircraft = new CargoAircraft(id);
              this->priority = 2;
          }
          else if (aircraftType == "Commercial") {
            //  this->aircraft = new CommercialAircraft(id);
              this->priority = 3;
          }
          else if (aircraftType == "Emergency") {
          //    this->aircraft = new EmergencyAircraft(id);
              this->priority = 1;
          }
          cout << "\n\n\n";
          
        
    }
      Flight(){
      emergency = false; 
      int temp;
      cout << "Enter flight ID: ";
      cin >> id;
      string aircraftType;
      cout << "Enter Airline id: (0, 1, 2, 3, 4, 5) ";
      cin >> airlineID;
      while(airlineID < 0 || airlineID > 5){
          cout << "ERROR OCCURED IN AIRLINE ID";
          cin >> airlineID;
          }
        airline = &airlines[airlineID];
      
      cout << "Enter aircraft type (1. Cargo, 2. Commercial 3. Emergency): ";
      cin >> temp;
      while(!(temp > 0 && temp < 4)){
      cout << "Invalid value entered. Please enter a value between 0 and 8 \n";
      cin  >> temp;
      }
      switch(temp){
      case 1: 
      aircraftType = "Cargo";
      break;
      case 2: 
      aircraftType = "Commercial";
      break;
      case 3: 
      aircraftType = "Emergency";
      break;
      }
      cout << "Enter current phase of the flight: (1. Holding, 2. Approach, 3. Landing, 4. AtGate, 5. Taxi, 6. TakeoffRoll, 7. Climb, 8. Cruise)";
      cin >> temp;
      while(!(temp > 0 && temp < 9)){
      cout << "Invalid value entered. Please enter a value between 0 and 8 \n";
      cin  >> temp;
      }
       switch(temp) {
        case 1: phase = "Holding"; break;
        case 2: phase = "Approach"; break;
        case 3: phase = "Landing"; break;
        case 4: phase = "AtGate"; break;
        case 5: phase = "Taxi"; break;
        case 6: phase = "TakeoffRoll"; break;
        case 7: phase = "Climb"; break;
        case 8: phase = "Cruise"; break;
        default: phase = "Unknown"; // Fallback for invalid input
    }
    cout << "Enter current speed of the aircraft in km/h \n";
    cin >> speed;
    while(speed < 0 ){
    cout << "Invalid value entered. Please enter a positive value. " << endl;
    cin >> speed;
    }
    cout << "Is the flight departing? (y/n)  ";
      //all flights initialised with AtGate
      char ch;
      cin >> ch;
      while(!(ch == 'y'|| ch == 'Y'|| ch  == 'n'|| ch  == 'N')){
      cout << "Invalid value entered. Try again. " << endl;
      cin >> ch;
      }
      (ch == 'y' || ch == 'Y')? isDeparture = true: isDeparture = false;
      
      cout << "Is the flight domestic? (y/n)  ";
      cin >> ch;
      while(!(ch == 'y'|| ch == 'Y'|| ch  == 'n'|| ch  == 'N')){
      cout << "Invalid value entered. Try again. " << endl;
      cin >> ch;
      }
      (ch == 'y' || ch == 'Y')? isDomestic = true: isDomestic = false;
      
      cout << "Whats the direction the flight is intended to keep? (1. North, 2, South, 3. East, 4. West) ";
      cin >> temp;
      while(!(temp > 0 && temp < 5)){
      cout << "Invalid value entered. Please enter a value between 0 and 5 \n";
      cin  >> temp;
      }
      cout << "Whats the scheduled time for this flight? (in minutes)";
      cin >> scheduledTime;
      while(scheduledTime < 0){
      cout << "Invalid value entered. Try again. " << endl;
      cin >> scheduledTime;
      }          
          aircraft = airline->assignAircraftToFlight();
          if (aircraftType == "Cargo") {
              //this->aircraft = new CargoAircraft(id);
              this->priority = 2;
          }
          else if (aircraftType == "Commercial") {
            //  this->aircraft = new CommercialAircraft(id);
              this->priority = 3;
          }
          else if (aircraftType == "Emergency") {
          //    this->aircraft = new EmergencyAircraft(id);
              this->priority = 1;
          }
          cout << "\n\n\n";

      }

      void display(){
          cout << "Displaying flight info: " << endl;
          cout << "aircraft ID: " <<  this->aircraft->id << endl << "Phase: " << phase << endl<< "AVN Status: " << aircraft->AVNStatus << endl << "Speed: " << speed << " knots "  << endl << "Direction: " << direction << endl; 
          cout << "Flight Type: ";
      isDomestic? cout <<  "Domestic " << endl : cout << "International " << endl;
     isDeparture? cout << "Departure " << endl : cout << "Arrival" << endl;     
      }
      
      bool isFacingEmergency(){
          return emergency;
      }

      void setAircraft(Aircraft* A) {
          this->aircraft = A;
      }

      string getAircraftType() {
          return this->aircraft->getType();
      }

      void setEmergency(){
          int randomValue = std::rand() % 100 + 1;  // 1-100
          if (!isDomestic && !isDeparture) {
              emergency = (randomValue <= 10); 
          }
          else if (isDomestic && !isDeparture) {
              emergency = (randomValue <= 5); 
          }
          else if (!isDomestic && isDeparture) {
              emergency = (randomValue <= 15); 
          }
          else if (isDomestic && isDeparture) {
              emergency = (randomValue <= 20); 
          }
      }
      virtual void checkViolation(){
          unordered_map<string, int> mapping = {{"Holding", 1}, {"Approach", 2}, {"Landing", 3}, {"Taxi", 4}, {"AtGate", 5}, {"TakeoffRoll", 6}, {"Climb", 7}, {"Cruise", 8}};

          int check = mapping[phase];
            switch (check) {
                case 1: //holding
                aircraft->AVNStatus = (speed < 400 || speed > 600);
            break;
                case 2: //approach
                aircraft->AVNStatus = (speed < 240 || speed > 290);
            break;
                case 3: //landing
                aircraft->AVNStatus = (speed > 240 || speed < 30);
            break;
                case 4: //taxi
                aircraft->AVNStatus = (speed > 30);
            break;
                case 5: //atgate
                aircraft->AVNStatus = (speed > 10);
            break;
                case 6: //takeoff roll
                aircraft->AVNStatus = (speed > 290);
            break;
                case 7: //climb
                aircraft->AVNStatus = (speed > 463);
            break;
                case 8: //cruise
                aircraft->AVNStatus = (speed < 800 || speed > 900);
            break;
                default:
                aircraft->AVNStatus = false;
            break;
            }
      }
      void setDirection(string dir){
          direction = dir;
      }
      void setPhase(string ph, int s){
        //    error checking omitted for simplicity
          phase = ph;
          speed = s; 
          checkViolation();
      }
      bool hasAVN() const {
          return aircraft->AVNStatus; 
      }
      string getPhase() const { 
          return phase; 
      }
      int getSpeed() const { 
          return speed; 
      }
      string getDirection()const {
          return direction;
      }
      
};

class Runways {
    bool beingUsed; //runway could either be used currently or not
    //not needed anymore since a mutex has been created
    string runwayID; //RWY-A, B, C
    mutex runwayBeingUsed;
    
  public:
    Runways(string id){
        beingUsed = false;
        runwayID = id;
        runwayBeingUsed.unlock(); //not being used yet
    }
    virtual bool isAllowed(Flight *flight) = 0; //checks if a certain flight is allowed in the runway
    bool available(){
        return !beingUsed;
    }
    void useRunway(){
        beingUsed = true;
    }
    void finishedUsingRunway(){
        beingUsed = false;
    }
    string getID() const {
        return runwayID;
    }
};

class RunwayA : public Runways {
  public:
      RunwayA() : Runways("RWY-A"){}
      virtual bool isAllowed(Flight* flight) override {
          if((flight->getDirection() == "North" || flight->getDirection() == "South") && available()){
            return true;
          }
      return false;
      }
};

class RunwayB : public Runways{
  public:
    RunwayB() : Runways("RWY-B"){}
    virtual bool isAllowed(Flight* flight) override {
        if((flight->getDirection() == "East" || flight->getDirection() == "West") && available()){
          return true;
        }
    return false;
    }
};
class RunwayC : public Runways{
  public:
    RunwayC() : Runways("RWY-C"){}
    virtual bool isAllowed(Flight* flight) override {
        if((flight->aircraft->getType() == "Cargo" || flight->aircraft->getType() == "Emergency") && available()){
          return true;
        }
    return false;
    }
};


#endif // MYHEADER_H
