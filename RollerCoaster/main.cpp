#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <semaphore>

using namespace std;

mutex mut1, mut2;
counting_semaphore boardQueue{0}, unboardQueue{0}, allAboard{0}, allAshore{0};
int boarders = 0;
int unboarders = 0;

const int C= 20;
const int nb_passengers=100;

void car(){
    while (true) {
            for (int i = 0; i < C; ++i) {
                boardQueue.release(); // Signal C passengers to board
            }

            for (int i = 0; i < C; ++i) {
                allAboard.acquire(); // Wait for the last passenger to signal allAboard
            }

            // Car is running
            cout << "Car is running with " << C << " passengers." << endl;
            this_thread::sleep_for(chrono::seconds(2)); // Simulating the ride

            for (int i = 0; i < C; ++i) {
                unboardQueue.release(); // Signal C passengers to unboard
            }

            for (int i = 0; i < C; ++i) {
                allAshore.acquire(); // Wait for the last passenger to signal allAshore
            }
        }

}

void passenger(int id){
    boardQueue.acquire(); // Wait for the car before boarding

        {
            lock_guard<mutex> lock(mut1);
            cout << "Passenger " << id << " boards." << endl;
            boarders += 1;
            if (boarders == C) {
                for (int i = 0; i < C; ++i) {
                    allAboard.release(); // Signal the car when all passengers have boarded
                }
                boarders = 0;
            }
        }

        unboardQueue.acquire(); // Wait for the car to stop before leaving


        {
            lock_guard<mutex> lock(mut2);
            cout << "Passenger " << id << " unboards." << endl;
            unboarders += 1;
            if (unboarders == C) {
                for (int i = 0; i < C; ++i) {
                    allAshore.release(); // Signal the car when all passengers have unboarded
                }
                unboarders = 0;
            }
        }
}


int main()
{
    thread car_thread(car);

    vector<thread> passenger_threads;
    for (int i = 1; i <= nb_passengers; ++i) {
        passenger_threads.emplace_back(passenger,i);
    }

    car_thread.join();
    for (auto &thread : passenger_threads) {
        thread.join();
    }
    return 0;
}
