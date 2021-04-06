#pragma once
#include <stdio.h>
#include <stdint.h>
#include "rs232.h"
#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <string>
#define plotSize 8334
#define cport_nr 4


//functions
void updateParameters(uint8_t amount, uint8_t startingIndex);
int recieveData(); // main function for receiving parameters, checks for delimiters and updates parameters
void requestPlot(); //request new plot data from the atmel controller
void requestParameters(); //send request command for parameters to atmel controller and reads the response
void updateTriggerLevel(int level); // set new trigger value, settings will be updated the next thread cyclus
void updateTriggerDelay(int delay); // set new delay value, settings will be updated the next thread cyclus
void requestData(std::string & message); //main thread function
void setRunning(bool status); //start & stop the thread
void setNewPlotData(bool status); //indicates that new plotdata is available
bool getNewPlotData(); 
uint16_t* getPlot(); // plot data getter
double* getParameters(); //parameter getter
void changeSettings(char commando, char value,std::string & message); //change a setting in the atmel controller


