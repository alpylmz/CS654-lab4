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

#include "u3.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <time.h>

#define MY_SIGNAL (SIGRTMIN)

HANDLE handle;
bool digital_out = 1;
u3CalibrationInfo caliInfo;
double voltage1, voltage2;

void timer_handler(int sig, siginfo_t *si, void *uc) {
    // Handler called on each timer expiration
    printf("Timer expired: Signal %d received.\n", sig);
	eDO(handle, 1, 2, digital_out);
	
	// do the same on the analog output
	if(digital_out){
		eDAC(handle, &caliInfo, 1, 3, voltage1, 0, 0, 0);
	}
	else{
		eDAC(handle, &caliInfo, 1, 3, voltage2, 0, 0, 0);
	}
	

	digital_out = !digital_out;
}

/* This function should initialize the DAQ and return a device
 * handle. The function takes as a parameter the calibratrion info to
 * be filled up with what obtained from the device. */
HANDLE init_DAQ(u3CalibrationInfo * caliInfo)
{
	HANDLE hDevice;
	int localID;
	
	/* Invoke openUSBConnection function here */
	hDevice = openUSBConnection(320084577);
	
	/* Invoke getCalibrationInfo function here */
	getCalibrationInfo(hDevice, caliInfo);
	
	return hDevice;
}

int main(int argc, char **argv)
{
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
    double frequency;
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
	printf("Enter the frequency of the square wave: ");
	scanf("%lf", &frequency);	

	/* Program a timer to deliver a SIGRTMIN signal, and the
	 * corresponding signal handler to output a square wave on
	 * BOTH digital output pin FIO2 and analog pin DAC0. */


	 struct sigaction sa;
	 struct sigevent sev;
	 timer_t timerid;
	 struct itimerspec its;
	 // calculate the interval using the frequency
	 long interval_ns = 1/(2*frequency)*1e9;

    // Set up the signal handler
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(MY_SIGNAL, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Specify that the timer should notify via signal
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = MY_SIGNAL;
    sev.sigev_value.sival_ptr = &timerid;

	if (timer_create(CLOCK_MONOTONIC, &sev, &timerid) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    // Configure the timer to expire after 'interval_ns' and then repeatedly every 'interval_ns'
    its.it_value.tv_sec = interval_ns / 1000000000;
    its.it_value.tv_nsec = interval_ns % 1000000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }

	while (1) {
        pause();
    }


	//eDO(handle, 1, 2, 1);
	//// sleep for 5 seconds
	//sleep(5);
	//eDO(handle, 1, 2, 0);


	/* The square wave generated on the DAC0 analog pin should
	 * have the voltage range specified by the user in the step
	 * above. */

	/* Display a prompt to the user such that if the "exit"
	 * command is typed, the USB DAQ is released and the program
	 * is terminated. */
	
	return EXIT_SUCCESS;
}

