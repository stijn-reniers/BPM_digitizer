#include "BPM_Reader.h"

bool running = true, newPlotData = false;
double parameter[12] = { 0 };
int n=0;
uint16_t plot[plotSize];
double syncData;
unsigned char buf[255];
bool newTriggerLevel = false, newTriggerDelay = false;
char tLevel = 255, tDelay = 0;
//update a certain amount of parameters from a given start index
void updateParameters(uint8_t amount, uint8_t startingIndex) {
    uint8_t reading = 0;
    while (reading < amount) {
        n = RS232_PollComport(cport_nr, buf, 8);
        if (n > 0) {
            parameter[startingIndex + reading] = ((double*)buf)[0];
            reading++;
        }
    }
}

// main function for receiving parameter, checks for delimiters and updates parameters when delimeters is read
int recieveData() {
    n = RS232_PollComport(cport_nr, (unsigned char*)&syncData, 8);
    if (n > 0) {
        if (syncData == 6666) {
            updateParameters(6, 0);
            n = RS232_PollComport(cport_nr, (unsigned char*)&syncData, 8);
            if (syncData == 7777) {
                updateParameters(6, 6);

            }
            return 1;
        }
        RS232_flushRXTX(3);

    }
    else {
        return 0;
    }
}

// send a plot request to the atmel microcontroller and reads the incoming data
void requestPlot() {
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, 3);
    RS232_SendByte(cport_nr, 255);
    //wait for the plot data to become available
    Sleep(2000);
    int i = 0;
    n = RS232_PollComport(cport_nr, ((unsigned char*)plot) + 1, 16668);
    std::cout << n << std::endl;
    newPlotData = true;
    
}

// send a parameter request to the atmel microcontroller and reads the incoming data
void requestParameters() {
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, 2);
    RS232_SendByte(cport_nr, 255);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    recieveData();
}
//updates trigger level
void updateTriggerLevel(int level) {
    newTriggerLevel = true;
    tLevel = (char)level;
}
//updates trigger delay
void updateTriggerDelay(int delay) {
    newTriggerDelay = true;
    tDelay = (char)delay;
}

// reader thread
void requestData() {
    int i = 0;
    while (running) {
        //request parameter
        auto start = std::chrono::high_resolution_clock::now();
        requestParameters();
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << " microseconds" << std::endl;

        //update parameters if necessary
        if (newTriggerDelay) {
            RS232_SendByte(cport_nr, 255);
            RS232_SendByte(cport_nr, 0);
            RS232_SendByte(cport_nr, tDelay);
            newTriggerDelay = false;
        }
        if (newTriggerLevel) {
            RS232_SendByte(cport_nr, 255);
            RS232_SendByte(cport_nr, 1);
            RS232_SendByte(cport_nr, tLevel);
            newTriggerLevel = false;
        }
        //request plot every couple of seconds
        if (i >= 100) {
            std::cout << "plot requested" << std::endl;
            start = std::chrono::high_resolution_clock::now();
            requestPlot();
            stop = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << duration.count() << " microseconds" << std::endl;

            i = 0;
        }
        Sleep(50);
        i++;
    }
}

double* getParameters()
{
    return parameter;
}

void setRunning(bool status)
{
    running = status;
}

bool getNewPlotData()
{
    return newPlotData;
}

void setNewPlotData(bool status)
{
    newPlotData = status;
}

uint16_t* getPlot()
{
    return plot;
}
