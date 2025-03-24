#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(){

    double voltage1, voltage2;
    printf("Two floating-point voltages:");
    
    scanf("%lf %lf", &voltage1, &voltage2);


    // get the clock resolution of CLOCK_REALTIME
    struct timespec res;
    clock_getres(CLOCK_REALTIME, &res);
}