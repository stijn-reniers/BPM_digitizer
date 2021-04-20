#include "BPM_Reader.h"
// variables
bool running = true, newPlotData = false;
double parameter[12] = { 0 };
int n = 0;
uint16_t receivedPlot[plotSize];
double syncData;
unsigned char buf[255];
bool newTriggerLevel = false, newTriggerDelay = false, connected = false;
char tLevel = 255, tDelay = 0;
uint16_t beamPositions[6] = { 0 };
uint32_t intensity = 0;
uint16_t fwhm[2] = { 0 };
float skewness[2] = { 0 };
float deviation[2] = { 0 };
int plotUpdateFreq = 10;
int total = 0, success = 0;

// main function for receiving parameter, checks for delimiters and updates parameters when delimeters is read
void recieveData() {
    n = RS232_PollComport(cport_nr, buf, 38);
    if (n == 38 && buf[0] == 111 && buf[37] == 222) {
        for (int i = 0; i < 6; i++) {
            beamPositions[i] = ((uint16_t*)(buf + 1))[i];
        }
        intensity = *((uint32_t*)(buf + 13));
        fwhm[0] = *((uint16_t*)(buf + 17));
        fwhm[1] = *((uint16_t*)(buf + 19));
        skewness[0] = *((float*)(buf + 21));
        skewness[1] = *((float*)(buf + 25));
        deviation[0] = *((float*)(buf + 29));
        deviation[1] = *((float*)(buf + 33));
        std::cout << "parameter fetching success" << std::endl;
        connected = true;
        total++;
        success++;
        std::cout << "success rate= " << ((float)success / (float)total) * 100 << "%" << std::endl;
    }
    else {
        std::cout << "failed to fetch parameters" << std::endl;
        total++;
    }
}

// send a plot request to the atmel microcontroller and reads the incoming data
void requestPlot() {
    RS232_flushRXTX(3);
    Sleep(100);
    RS232_PollComport(cport_nr, buf, 255);
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, 3);
    RS232_SendByte(cport_nr, 255);
    //wait for the plot data to become available
    Sleep(2000);
    RS232_PollComport(cport_nr, buf, 3);
    n = RS232_PollComport(cport_nr, ((unsigned char*)receivedPlot), 16668);
    std::cout << n << std::endl;
    newPlotData = true;

}

// send a parameter request to the atmel microcontroller and reads the incoming data
void requestParameters() {
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, 2);
    RS232_SendByte(cport_nr, 255);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    //RS232_PollComport(cport_nr, buf, 3);
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
void changeSettings(char commando, char value, std::string* message) {
    RS232_flushRXTX(3);
    Sleep(100);
    n = RS232_PollComport(cport_nr, buf, 255);
    std::cout << "parameters changed" << std::endl;
    std::cout << n << std::endl;
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, commando);
    RS232_SendByte(cport_nr, value);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    RS232_PollComport(cport_nr, buf, 3);
    std::cout << "eccho received" << std::endl;
    for (int i = 0;i < 3;i++) {
        std::cout << +buf[i] << std::endl;
    }
    if (buf[0] == 255) {
        if (buf[1] == 0) {
            std::cout << "command recognized" << std::endl;
            *message = "eccho message received trigger delay has now been set to = ";
            (*message).append(std::to_string(buf[2]));
            (*message).append("ms");
        }
        if (buf[1] == 1) {
            *message = "eccho message received trigger level has now been set to = ";
            (*message).append(std::to_string(buf[2]));
        }

    }
    else {
        *message = "failed to recognize eccho message";
    }
}
// reader thread
void requestData(std::string* message) {
    int i = 0;
    while (running) {
        //request parameter
        auto start = std::chrono::high_resolution_clock::now();
        requestParameters();


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
        if (i >= plotUpdateFreq && connected) {
            std::cout << "plot requested" << std::endl;
            requestPlot();
            i = 0;
            *message = "Plot updated";
        }
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << duration.count() << " milliseconds" << std::endl;
        Sleep(1000);
        i++;
    }
}

double* getParameters()
{
    return parameter;
}
uint16_t* getBeamPostion() {
    return beamPositions;
}

uint32_t* getIntensity() {
    return &intensity;
}
uint16_t* getFwhm() {
    return fwhm;
}
float* getSkewness() {
    return skewness;
}
float* getDeviation() {
    return deviation;
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

int* getPlotUpdateFreqPtr() {
    return &plotUpdateFreq;
}
