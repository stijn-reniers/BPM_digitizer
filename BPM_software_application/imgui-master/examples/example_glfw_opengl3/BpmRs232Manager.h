#pragma once
#include "BpmCommunicationManager.h"
#include "rs232.h"
#include <thread>
class BpmRs232Manager :public BpmCommunicationManager
{
public:
    BpmRs232Manager(int com): cport_nr(com) {};
    // Inherited via BpmCommunicationManager
    virtual bool setupCommunication() override;
    virtual void requestData() override;
    virtual void cleanUpCommunication() override;
    

    // Communication dependent functions
    void setBaudRate(int rate) { baudRate = rate; };
    void setcport_nr(int nr) { cport_nr = nr; };
    // send a command to the ATMEL board that new parameters are required
    void requestParameters();
    // receive the parameters and update the beam parameter variables
    void recieveParameters();
    void requestPlot();
    void changeTriggerDelay();
    void changePeakThreshold();
    void setDcCorrection();
    // function that sends a certain command to the ATMEL board and expects an eccho back
    void editSettings(char sendValue, char commandIndex);
private:
    int baudRate = 115200;
    int cport_nr = 3;
    char mode[4] = { '8','N','1',0 };
    bool connected = false;
};

