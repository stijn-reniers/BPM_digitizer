#pragma once
#include "BpmCommunicationManager.h"
#include "rs232.h"
#include <thread>
class BpmRs232Manager :public BpmCommunicationManager
{
public:
    // Inherited via BpmCommunicationManager
    virtual bool setupCommunication() override;
    virtual void requestData() override;
    virtual void cleanUpCommunication() override;

    // Communication dependent functions
    void setBaudRate(int rate) { baudRate = rate; };
    void setcport_nr(int nr) { cport_nr = nr; };
    void requestParameters();
    void recieveParameters();
    void requestPlot();
    void changeTriggerDelay();

private:
    int baudRate = 115200;
    int cport_nr = 3;
    char mode[4] = { '8','N','1',0 };
    bool connected = false;
};

