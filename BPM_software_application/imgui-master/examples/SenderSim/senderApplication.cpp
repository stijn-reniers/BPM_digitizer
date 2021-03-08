// senderApplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECRURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "rs232.h"
#include <time.h>
#include <chrono>
using namespace std;
int main()
{
    int i = 0, x=0,
        cport_nr = 5,        /* /dev/ttyS0 (COM1 on windows) */
        bdrate = 115200;       /* 9600 baud */

    char mode[] = { '8','N','1',0 },
        str[2][512];

    const double parameter[12] = {1014.0,5296.0,1062.0,5344.0,1099.0,5380.0,41719.0,41467.0,34.0,34.0,-0.13187,-0.035948};
    const uint16_t sync[5] = { 10111,9999,8888,7777,6666 };

    if (RS232_OpenComport(cport_nr, bdrate, mode, 0))
    {
        printf("Can not open comport\n");

        return(0);
    }

    while (1)
    {
        RS232_SendBuf(cport_nr, (unsigned char*)(&sync[0]),2);
        for (int i = 0; i < 3;i++) {
            RS232_SendBuf(cport_nr, (unsigned char*)(parameter+i), 8);
        }
        RS232_SendBuf(cport_nr, (unsigned char*)(&sync[1]), 2);
        for (int i = 3; i < 6;i++) {
            RS232_SendBuf(cport_nr, (unsigned char*)(parameter + i), 8);
        }
        RS232_SendBuf(cport_nr, (unsigned char*)(&sync[2]), 2);
        RS232_SendBuf(cport_nr, (unsigned char*)(&parameter[6]), 8);
        RS232_SendBuf(cport_nr, (unsigned char*)(&parameter[7]), 8);
        RS232_SendBuf(cport_nr, (unsigned char*)(&sync[3]), 2);
        RS232_SendBuf(cport_nr, (unsigned char*)(&parameter[8]), 8);
        RS232_SendBuf(cport_nr, (unsigned char*)(&parameter[9]), 8);
        RS232_SendBuf(cport_nr, (unsigned char*)(&sync[4]), 2);
        RS232_SendBuf(cport_nr, (unsigned char*)(&parameter[10]), 8);
        RS232_SendBuf(cport_nr, (unsigned char*)(&parameter[11]), 8);
        //RS232_flushTX(5);
        //RS232_cputs(cport_nr, (char*)parameter);

        

#ifdef _WIN32
        Sleep(1000);
#else
        usleep(1000000);  /* sleep for 1 Second */
#endif
        x++;

        i= x%2;
        if (x == 100) {
            break;
        }
    }

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file




//n = RS232_PollComport(cport_nr, &data, 1);
//if (n > 0 && data == 128) {
//    n = RS232_PollComport(cport_nr, &data, 1);
//    if (n > 0 && data == 255) {
//        reading = 0;
//        while (reading < 6) {
//            n = RS232_PollComport(cport_nr, buf, 8);
//            if (n > 0) {
//                parameter[reading] = ((double*)buf)[0];
//                reading++;
//            }
//        }
//        n = RS232_PollComport(cport_nr, &data, 1);
//        if (n > 0 && data == 67) {
//            while (reading < 12) {
//                n = RS232_PollComport(cport_nr, buf, 8);
//                if (n > 0) {
//                    parameter[reading] = ((double*)buf)[0];
//                    reading++;
//                }
//            }
//        }
//    }
//}
