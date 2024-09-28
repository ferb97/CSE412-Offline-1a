#include<bits/stdc++.h>
#include "lcgrand.c"
#include "lcgrand.h"
using namespace std;

// Define maximum queue length and busy and idle status
#define MAX_Q 100
#define BUSY 1
#define IDLE 0

// Declare global variables
int next_event, current_total_delayed, total_delayed_required, total_events, current_queue_length, server_status;

double queue_length_area, server_status_area, current_simulation_time, last_event_time, next_event_time[3], arrival_time[MAX_Q + 1], total_delay;

// Declare input parameters
double mean_interarrival, mean_service;

// Variables for total customers arrived and current customer in service
int total_customers_arrived, current_customer_in_service;

// Declare input and output file streams
ifstream input_file;
ofstream output_file1, output_file2;

// Declare function prototypes
void initialize();
void timing_function();
double exponential_random_variable(double mean);
void arrival();
void departure();
void report();
void update_statistics();


// Main function
int main()
{
    // Open input and output files
    input_file.open("input.txt");
    output_file1.open("output1.txt");
    output_file2.open("output2.txt");

    // Check if files are opened successfully
    if(!input_file.is_open()){
        cerr << "Error opening input file" << endl;
        return 1;
    }

    if(!output_file1.is_open()){
        cerr << "Error opening output file 1" << endl;
        return 1;
    }

    if(!output_file2.is_open()){
        cerr << "Error opening output file 2" << endl;
        return 1;
    }

    // Define the number of total events
    total_events = 2;

    // Read input parameters
    input_file >> mean_interarrival >> mean_service >> total_delayed_required;

    // Write input parameters to output file 1
    output_file1 << "Single-server queueing system\n\n";
    output_file1 << "Mean interarrival time" << setw(11) << fixed << setprecision(3) << mean_interarrival << " minutes\n\n";
    output_file1 << "Mean service time" << setw(16) << fixed << setprecision(3) << mean_service << " minutes\n\n";
    output_file1 << "Number of customers" << setw(14) << total_delayed_required << "\n\n";

    // Initialize the simulation
    initialize();

    // Loop until the required number of delayed customers is reached
    for(int i = 1; current_total_delayed < total_delayed_required; i++){

        // Call the timing function
        timing_function();

        // Call the update statistics function
        update_statistics();

        // Arrival or departure routine based on the next event
        if(next_event == 1){
            // Total customers increased, Arrival event called
            total_customers_arrived++;
            output_file2 << i << ". Next event: Customer " << total_customers_arrived << " Arrival" << endl; 
            arrival();
        } else if(next_event == 2){
            // Departure event called
            output_file2 << i << ". Next event: Customer " << current_customer_in_service << " Departure" << endl;
            departure();
        }
    }

    // Simulation ended, report the results
    report();

    // Close the input and output files
    input_file.close();
    output_file1.close();
    output_file2.close();

    return 0;
}

// Function to generate exponential random variable
double exponential_random_variable(double mean){
    return -mean * log(lcgrand(1));
}

// Initializing the simulation
void initialize(){
    // Set the simulation time to zero
    current_simulation_time = 0.0;

    // Initialize the state variables
    current_queue_length = 0;
    server_status = IDLE;
    last_event_time = 0.0;

    // Initialize the statistical counters
    queue_length_area = 0.0;
    server_status_area = 0.0;
    total_delay = 0.0;
    current_total_delayed = 0;

    // Initialize the total customers arrived and current customer in service
    total_customers_arrived = 0;
    current_customer_in_service = 0;

    // Initialize the event list
    next_event_time[1] = current_simulation_time + exponential_random_variable(mean_interarrival);
    next_event_time[2] = 1.0e+30;
}


// Timing function to determine the next event
void timing_function(){
    double min_time_next_event = 1.0e+29;
    next_event = 0;

    // Determine the next event
    for(int i = 1; i <= total_events; i++){
        if(next_event_time[i] < min_time_next_event){
            min_time_next_event = next_event_time[i];
            next_event = i;
        }
    }

    // Event list empty if next event is 0   
    if(next_event == 0){
        output_file2 << "\nEvent list empty at time " << current_simulation_time << endl;
        exit(1);
    }

    // Update the simulation time
    current_simulation_time = min_time_next_event;
}


// Update the statistics
void update_statistics(){
    // Calculate time passed after last event
    double time_passed_after_last_event = current_simulation_time - last_event_time;
    last_event_time = current_simulation_time;

    // Update the queue length area
    queue_length_area += current_queue_length * time_passed_after_last_event;

    // Update the server status area
    server_status_area += server_status * time_passed_after_last_event;
}


// Arrival function
void arrival(){
    // Calculate the next arrival time
    next_event_time[1] = current_simulation_time + exponential_random_variable(mean_interarrival);

    // Check the server status
    if(server_status == BUSY){
        // If the server is busy, increase the queue length
        current_queue_length++;

        // Check if the queue length exceeds the maximum queue length
        if(current_queue_length > MAX_Q){
            output_file2 << "Overflow of the array time_arrival at time " << current_simulation_time << endl;
            exit(2);
        }

        // Save the arrival time
        arrival_time[current_queue_length] = current_simulation_time;
    }

    else{
        // If the server is idle, the customer goes directly to the server
        server_status = BUSY;

        // Increase the total delayed customers, as delay is zero, it is not added to total delay
        current_total_delayed++;

        // Update the current customer in service
        current_customer_in_service = total_customers_arrived;

        output_file2 << "\n---------No. of customers delayed: " << current_total_delayed << "--------\n\n";

        // Calculate the departure time for the customer in service
        next_event_time[2] = current_simulation_time + exponential_random_variable(mean_service);
    }
}


// Departure function
void departure(){
    // Check if the queue is empty
    if(current_queue_length == 0){
        // If the queue is empty, the server goes to idle state
        server_status = IDLE;
        next_event_time[2] = 1.0e+30;

        // Currently no customer in service
        current_customer_in_service = 0;
    }

    else{
        // If the queue is not empty, the first customer in the queue goes to the server, so queue length decreased
        current_queue_length--;

        // Calculate the delay for the customer and add it to the total delay
        total_delay += (current_simulation_time - arrival_time[1]);
        current_total_delayed++;

        // Update the current customer in service
        current_customer_in_service = total_customers_arrived - current_queue_length;

        // Calculate the departure time for the customer in service
        next_event_time[2] = current_simulation_time + exponential_random_variable(mean_service);
        output_file2 << "\n---------No. of customers delayed: " << current_total_delayed << "--------\n\n";

        // Shift the arrival times in the queue
        for(int i = 1; i <= current_queue_length; i++){
            arrival_time[i] = arrival_time[i + 1];
        }
    }
}


// Report function to write the results to the output file
void report(){
    output_file1 << "\n\nAverage delay in queue" << setw(11) << fixed << setprecision(3) << total_delay / total_delayed_required << " minutes\n\n";
    output_file1 << "Average number in queue" << setw(10) << fixed << setprecision(3) << queue_length_area / current_simulation_time << "\n\n";
    output_file1 << "Server utilization" << setw(15) << fixed << setprecision(3) << server_status_area / current_simulation_time << "\n\n";
    output_file1 << "Time simulation ended" << setw(12) << fixed << setprecision(3) << current_simulation_time << " minutes\n\n";
}



