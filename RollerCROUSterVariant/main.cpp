#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <semaphore>
#include <random>
#include <chrono>

using namespace std;

// Mutex to protect trail and student
mutex mut1, mut2, mut3, mut4;
counting_semaphore boardQueue{0}, unboardQueue{0}, crousFull{0}, crousEmpty{0};

//number of trays taken
int TRAYS_TAKEN = 0;
//number of trays left
int TRAYS_LEFT = 0;
//number of student in the CROUS
int NB_STUDENTS=0;
//max number of trays
const int NB_TRAYS= 20;


void lunch(){
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

void student(int id){
    // Wait for an available tray
    {
        lock_guard<mutex> lock(mut3);
        boardQueue.acquire();
    }

    // Take a tray
    {
        lock_guard<mutex> lock(mut1);
        cout << "Student " << id << " take a tray." << endl;
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
        cout << "Student " << id << " leave his tray." << endl;
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

/*
 * Simulates a new arrival of students every 5 seconds, with between 1 and 10 students each time.
 * The service will start as soon as there are 20 students in the queue
 * */
void simulateStudentArrival() {
    random_device rd;
    default_random_engine gen(rd());
    uniform_int_distribution<int> studentCount(1, 10); // Generates between 1 and 10 students at a time
    vector<thread> passenger_threads;

    while (true) {
        int newStudents = studentCount(gen); // Generates a new arrival of 1 to 10 students at a time
        cout << "Incoming " << newStudents << " students." << endl;


        for (int i = 0; i < newStudents; ++i) {
            // Create a thread for each new student
            passenger_threads.emplace_back(student, NB_STUDENTS);
            NB_STUDENTS++; // Increase the total number of students
        }

        for (auto &thread : passenger_threads) {
            //Checks if a thread is stopped
            if(!thread.joinable()){
                thread.join(); // Free the thread to avoid zombie threads
            }
        }
        // Wait 5 seconds before the next arrival
        this_thread::sleep_for(chrono::seconds(5));
    }
}

int main()
{
    thread crous_thread(lunch);
    thread student_arrival_thread(simulateStudentArrival);

    crous_thread.join();
    student_arrival_thread.join();

    return 0;
}
