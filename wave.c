/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                                                  */
/*   Description: template file for digital and     */
/*                analog square wave generation     */
/*                via the LabJack U3-LV USB DAQ     */
/*                                                  */
/****************************************************/

// Added some libs just in case
#include "u3.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

// After rereading the doc it says to use two timers 
#define MY_SIGNAL (SIGRTMIN)
#define MY_SIGNAL2  (SIGRTMAX)

HANDLE handle;
bool digital_out = 1;
bool analog_out = 1;
u3CalibrationInfo caliInfo;
double voltage1, voltage2;

// Digital Timer
void timer_handler(int sig, siginfo_t *si, void *uc){
    	// Handler called on each timer expiration
	printf("Digital Timer expired: Signal %d received.\n", sig);
	eDO(handle, 0, 2, digital_out);
	
	// do the same on the analog output
	if(digital_out){
		eDAC(handle, &caliInfo, 1, 0, voltage1, 0, 0, 0);
	}
	else{
		eDAC(handle, &caliInfo, 1, 0, voltage2, 0, 0, 0);
	}
	digital_out = !digital_out;
}

// Analog timer now with SIGRTMAX
void timer_handler2(int sig, siginfo_t *si, void *uc){
	printf("Analog Timer expired: Signal %d received.\n", sig);
	if(analog_out){
		eDAC(handle, &caliInfo, 1, 0, voltage1, 0, 0, 0);
	}
	else{
		eDAC(handle, &caliInfo, 1, 0, voltage2, 0, 0, 0);
	}
	analog_out = !analog_out;
}
	


/* This function should initialize the DAQ and return a device
 * handle. The function takes as a parameter the calibratrion info to
 * be filled up with what obtained from the device. */
HANDLE init_DAQ(u3CalibrationInfo * caliInfo){
	HANDLE hDevice;	
	/* Invoke openUSBConnection function here */
	hDevice = openUSBConnection(320084577);
	/* Invoke getCalibrationInfo function here */
	getCalibrationInfo(hDevice, caliInfo);
	
	return hDevice;
}

// OK instead of doing double the timer setup in main im just gonna do this
// helper function to set up the timers as long as I dont mess up any of the 
// predefined vars / functions it should be fine but I'm rawdogging it in github rn

timer_t create_timer(int signo, void (*handler)(int, siginfo_t*, void*), double freq){
	struct sigaction sa;
	struct sigevent sev; 
	timer_t tid;
	struct itimerspec its; 

	long interval_ns = (long)((1.0/(2.0 * freq)) * 1e9);

	// Setup the signal handler
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handler;
        sigemptyset(&sa.sa_mask);
        if (sigaction(signo, &sa, NULL) == -1) {
	    perror("sigaction");
	    exit(EXIT_FAILURE);
        }

	// Specify that the timer should notify via signal
        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = signo;
        sev.sigev_value.sival_ptr = &tid;

	// Check of created
	if(timer_create(CLOCK_MONOTONIC, &sev, &tid) == -1){
		perror("timer_create");
		exit(EXIT_FAILURE);
	}

	// Set intervals
	its.it_value.tv_sec  = interval_ns / 1000000000;
    	its.it_value.tv_nsec = interval_ns % 1000000000;
    	its.it_interval.tv_sec  = its.it_value.tv_sec;
    	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	if(timer_settime(tid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    	}
	
	return tid;
}


// way to listen for "exit" to terminate
void* exit_thread(void* arg){
	char command[16];
	while(1){
		printf("Type 'exit' to terminate program:\n");
		if(scanf("%15s", cmd) == 1){
			if(strcmp(cmd, "exit") == 0){
				printf("Exiting program \n");
				closeUSBConnection(handle);
				exit(EXIT_SUCCESS);
			}
		}
		usleep(2000); // or just sleep?
	}
	return NULL;
} // im honestly not too confident in this but well see


int main(int argc, char **argv){
	// read calibration info
	//printf("Calibration info: %lf\n", caliInfo.ccConstants[0]);
	//printf("prodID: %d\n", caliInfo.prodID);
	//printf("hardware version: %lf\n", caliInfo.hardwareVersion);
	//printf("high voltage: %d\n", caliInfo.highVoltage);

	
	/* Invoke init_DAQ and handle errors if needed */
	handle = init_DAQ(&caliInfo);
	

	/* Provide prompt to the user for a voltage range between 0
	 * and 5 V. Require a new set of inputs if an invalid range
	 * has been entered. */
    	printf("Two floating-point voltages:");
	scanf("%lf %lf", &voltage1, &voltage2);

	if(voltage1 < 0 || voltage1 > 5 || voltage2 < 0 || voltage2 > 5){
		printf("Invalid voltage range. Please enter a voltage between 0 and 5 V\n");
		exit(42);
	}
    
	
	/* Compute the maximum resolutiuon of the CLOCK_REALTIME
	* system clock and output the theoretical maximum frequency
	* for a square wave */
	// get the clock resolution of CLOCK_REALTIME
	struct timespec res;
	clock_getres(CLOCK_REALTIME, &res);
	// print the resolution
	printf("The resolution of CLOCK_REALTIME is %ld ns\n", res.tv_nsec);
	printf("The maximum frequency of a square wave is: %lf Hz\n", 1/(2*res.tv_nsec*1e-9));

	/* Provide prompt to the user to input a desired square wave
	* frequency in Hz. */
	double frequency;
	printf("Enter the frequency of the square wave: ");
	scanf("%lf", &frequency);	

	/* Program a timer to deliver a SIGRTMIN signal, and the
	 * corresponding signal handler to output a square wave on
	 * BOTH digital output pin FIO2 and analog pin DAC0. */
	// Plus this way we could set different frequencies for each timer
	timer_t digital_timer_id = create_timer(MY_SIGNAL,  timer_handler,  frequency);
	timer_t analog_timer_id  = create_timer(MY_SIGNAL2, timer_handler2, frequency);

	eDO(handle, 1, 2, digital_out);

	// exit thread 
	pthread_t tid;
    	pthread_create(&tid, NULL, exit_thread, NULL); // this part i looked up might work
	
	while (1) {
        	pause();
    	}
	
	return EXIT_SUCCESS;
}

