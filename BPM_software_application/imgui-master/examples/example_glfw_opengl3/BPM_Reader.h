#pragma once
#include <stdio.h>
#include <stdint.h>
#include "rs232.h"
#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <thread>
#define plotSize 8334
#define cport_nr 3
void updateParameters(uint8_t amount, uint8_t startingIndex);
int recieveData();
void requestPlot();
void requestParameters();
void updateTriggerLevel(int level);
void updateTriggerDelay(int delay);
void requestData();
double* getParameters();
void setRunning(bool status);
bool getNewPlotData();
void setNewPlotData(bool status);
uint16_t* getPlot();
