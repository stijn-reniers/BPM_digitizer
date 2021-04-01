#include "BPM_Reader.h"
// variables
bool running = true, newPlotData = false;
double parameter[12] = { 0 };
int n = 0;
uint16_t receivedPlot[plotSize];
double syncData;
unsigned char buf[255];
bool newTriggerLevel = false, newTriggerDelay = false;
char tLevel = 255, tDelay = 0;

void resetBuffer() {
    for (int i = 0;i < 8;i++) {
        buf[i] = 0;
    }
}
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
    RS232_PollComport(cport_nr, buf, 3);
    n = RS232_PollComport(cport_nr, ((unsigned char*)receivedPlot) + 1, 16668);
    std::cout << n << std::endl;
    newPlotData = true;
    resetBuffer();
}

// send a parameter request to the atmel microcontroller and reads the incoming data
void requestParameters() {
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, 2);
    RS232_SendByte(cport_nr, 255);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    RS232_PollComport(cport_nr, buf, 3);
    recieveData();
    resetBuffer();
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
void changeSettings(char commando, char value, std::string& message) {
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, commando);
    RS232_SendByte(cport_nr, value);
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    RS232_PollComport(cport_nr, buf, 3);
    if (buf[0] == 255) {
        if (buf[1] == 0) {
            message = "eccho message received trigger delay has now been set to = " + std::string(1, buf[2]) + "ms";
        }
        if (buf[2] == 1) {
            message = "eccho message received trigger level has now been set to = " + std::string(1, buf[2]);
        }

    }
    else {
        message = "failed to recognize eccho message";
    }
}
// reader thread
void requestData(std::string& message) {
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
            changeSettings(0, tDelay, message);
            newTriggerDelay = false;
        }
        if (newTriggerLevel) {
            changeSettings(1, tLevel, message);
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
    return receivedPlot;
}
