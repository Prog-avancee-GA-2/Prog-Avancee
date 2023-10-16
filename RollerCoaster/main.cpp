#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <semaphore>

using namespace std;

//mutex to protect passenger and car
mutex mut1, mut2;
counting_semaphore boardQueue{0}, unboardQueue{0}, allAboard{0}, allAshore{0};
// number of people in the car while boarding
int boarders = 0;
// number of people left in the car while unboarding
int unboarders = 0;
// number of places in the car
const int C= 20;
// number of people waiting to get in the car
const int nb_passengers=100;

void car(){
    while (true) {
        // Send signal to passengers to start boarding
        for (int i = 0; i < C; ++i) {
            boardQueue.release();
        }
        // Wait until last passenger is on board
        for (int i = 0; i < C; ++i) {
            allAboard.acquire();
        }

        // Car is running
        cout << "Car is running with " << C << " passengers." << endl;
        // Simulating the ride
        this_thread::sleep_for(chrono::seconds(2));

        // Send signal to passengers to start unboarding
        for (int i = 0; i < C; ++i) {
            unboardQueue.release();
        }
        // Wait until last passengers has left the board
        for (int i = 0; i < C; ++i) {
            // Wait for the last passenger to signal allAshore
            allAshore.acquire();
        }
    }
}

void passenger(int id){
    // Wait for the car before boarding
    boardQueue.acquire();

    // Using mut1 to lock the ressources of the content below
    {
        lock_guard<mutex> lock(mut1);
        cout << "Passenger " << id << " boards." << endl;
        // The number of people that aree in the car
        boarders += 1;
        // When the car is full
        if (boarders == C) {
            for (int i = 0; i < C; ++i) {
                allAboard.release(); // Signal the car when all passengers have boarded
            }
            // Set the number of people in the car waiting for the ride to begin to 0
            boarders = 0;
        }
    }
    // Wait for the car to stop before leaving
    unboardQueue.acquire();

    // Using mut2 to lock the ressources of the content below
    {
        lock_guard<mutex> lock(mut2);
        cout << "Passenger " << id << " unboards." << endl;
        // The number of people that have left the car
        unboarders += 1;
        // When the car is empty
        if (unboarders == C) {
            for (int i = 0; i < C; ++i) {
                allAshore.release(); // Signal the car when all passengers have unboarded
            }
            //Set the number of people waiting to leave the car to 0
            unboarders = 0;
        }
    }
}


int main()
{
    thread car_thread(car);

    vector<thread> passenger_threads;
    // While there is still passengers waiting
    for (int i = 1; i <= nb_passengers; ++i) {
        passenger_threads.emplace_back(passenger,i);
    }
    car_thread.join();
    for (auto &thread : passenger_threads) {
        thread.join();
    }
    return 0;
}
