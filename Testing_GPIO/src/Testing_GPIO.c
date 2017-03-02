/*
===============================================================================
 Name        : Testing_GPIO.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include <cr_section_macros.h>

#include <stdio.h>

#if defined (__MULTICORE_MASTER_SLAVE_M0SLAVE) || \
    defined (__MULTICORE_MASTER_SLAVE_M4SLAVE)
#include "boot_multicore_slave.h"
#endif

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

int main(void) {

#if defined (__MULTICORE_MASTER_SLAVE_M0SLAVE) || \
    defined (__MULTICORE_MASTER_SLAVE_M4SLAVE)
    boot_multicore_slave();
#endif

    printf("Hello World\n");

    // Force the counter to be placed into memory
    volatile static int i = 0 ;
    // Enter an infinite loop, just incrementing a counter
    while(1) {
        i++ ;
        printf("%d",i);
    }
    return 0 ;
}
