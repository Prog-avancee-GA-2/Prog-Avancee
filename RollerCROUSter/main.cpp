#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <semaphore>

using namespace std;

// Mutex to protect trail and student
mutex mut1, mut2, mut3, mut4;

counting_semaphore boardQueue{0}, unboardQueue{0}, crousFull{0}, crousEmpty{0};

//number of trays taken
int TRAYS_TAKEN = 0;
//number of trays left
int TRAYS_LEFT = 0;

//max number of trays
const int NB_TRAYS = 20;
//max number of students
const int NB_STUDENTS = 100;

void lunch() {
    while (true) {
        // Fill trays for students
        for (int i = 0; i < NB_TRAYS; ++i) {
            boardQueue.release(); // Increase the number of available trays
        }

        // Wait for all trays to be taken by students
        for (int i = 0; i < NB_TRAYS; ++i) {
            crousFull.acquire(); // Decrease the number of available trays
        }

        cout << "Lunch with " << NB_TRAYS << " student's trays." << endl;
        this_thread::sleep_for(chrono::seconds(2));

        // Students return their trays after eating
        for (int i = 0; i < NB_TRAYS; ++i) {
            unboardQueue.release(); // Increase the number of empty trays
        }

        // Wait for all trays to be empty
        for (int i = 0; i < NB_TRAYS; ++i) {
            crousEmpty.acquire(); // Decrease the number of empty trays
        }
    }
}

void student(int id) {
    // Wait for an available tray
    {
        lock_guard<mutex> lock(mut3);
        boardQueue.acquire();
    }

    // Take a tray
    {
        lock_guard<mutex> lock(mut1);
        cout << "Student " << id << " takes a tray." << endl;
        TRAYS_TAKEN += 1;

        // When all students have taken a tray, indicate that the restaurant is full
        if (TRAYS_TAKEN == NB_TRAYS) {
            for (int i = 0; i < NB_TRAYS; ++i) {
                crousFull.release(); // Increase the number of full trays
            }
            TRAYS_TAKEN = 0;
        }
    }

    // Wait for returning the tray after eating
    {
        lock_guard<mutex> lock(mut4);
        unboardQueue.acquire();
    }

    // Return the tray
    {
        lock_guard<mutex> lock(mut2);
        cout << "Student " << id << " leaves their tray." << endl;
        TRAYS_LEFT += 1;

        // When all students have returned their trays, indicate that the restaurant is empty
        if (TRAYS_LEFT == NB_TRAYS) {
            for (int i = 0; i < NB_TRAYS; ++i) {
                crousEmpty.release(); // Increase the number of empty trays
            }
            TRAYS_LEFT = 0;
        }
    }
}

int main() {
    // Thread to simulate the university restaurant
    thread crous_thread(lunch);

    // Threads to simulate student behavior
    vector<thread> student_threads;
    for (int i = 1; i <= NB_STUDENTS; ++i) {
        student_threads.emplace_back(student, i);
    }

    // Wait for the university restaurant thread to finish
    crous_thread.join();

    // Wait for all student threads to finish
    for (auto &thread : student_threads) {
        thread.join();
    }

    return 0;
}
