#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <semaphore>

using namespace std;

mutex mut1, mut2;
counting_semaphore boardQueue{0}, unboardQueue{0}, crousFull{0}, crousEmpty{0};
int TRAYS_TAKEN = 0;
int TRAYS_LEFT = 0;

const int NB_TRAYS= 20;
const int NB_STUDENTS=100;

void lunch(){
    while (true) {
            for (int i = 0; i < NB_TRAYS; ++i) {
                boardQueue.release();
            }

            for (int i = 0; i < NB_TRAYS; ++i) {
                crousFull.acquire();
            }


            cout << "Lunch with " << NB_TRAYS << " student's trays." << endl;
            this_thread::sleep_for(chrono::seconds(2));

            for (int i = 0; i < NB_TRAYS; ++i) {
                unboardQueue.release();
            }

            for (int i = 0; i < NB_TRAYS; ++i) {
                crousEmpty.acquire();
            }
        }

}

void student(int id){
    boardQueue.acquire();

        {
            lock_guard<mutex> lock(mut1);
            cout << "Student " << id << " take a tray." << endl;
            TRAYS_TAKEN += 1;
            if (TRAYS_TAKEN == NB_TRAYS) {
                for (int i = 0; i < NB_TRAYS; ++i) {
                    crousFull.release();
                }
                TRAYS_TAKEN = 0;
            }
        }

        unboardQueue.acquire();

        {
            lock_guard<mutex> lock(mut2);
            cout << "Student " << id << " leave his tray." << endl;
            TRAYS_LEFT += 1;
            if (TRAYS_LEFT == NB_TRAYS) {
                for (int i = 0; i < NB_TRAYS; ++i) {
                    crousEmpty.release();
                }
                TRAYS_LEFT = 0;
            }
        }
}


int main()
{
    thread car_thread(lunch);

    vector<thread> passenger_threads;
    for (int i = 1; i <= NB_STUDENTS; ++i) {
        passenger_threads.emplace_back(student,i);
    }

    car_thread.join();
    for (auto &thread : passenger_threads) {
        thread.join();
    }

    return 0;
}
