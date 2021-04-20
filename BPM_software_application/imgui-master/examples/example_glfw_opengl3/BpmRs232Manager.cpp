#include "BpmRs232Manager.h"

bool BpmRs232Manager::setupCommunication()
{
    if (RS232_OpenComport(cport_nr, baudRate, mode, 0))
    {
        printf("Can not open comport\n");

        return false;
    }
    RS232_flushRXTX(cport_nr);
    return true;
}

void BpmRs232Manager::requestData()
{
    int counter = 0;
    while (running) {
        requestParameters();
        if (newTriggerDelay) {
            changeTriggerDelay();
            newTriggerDelay = false;
        }
        if (counter >= plotUpdateFrequency && connected) {
            requestPlot();
            ecchoMessage = "Plot updated";
            counter = 0;
        }
        counter++;
    }
}
    

void BpmRs232Manager::cleanUpCommunication()
{
    RS232_CloseComport(cport_nr);
}

void BpmRs232Manager::requestParameters()
{
    unsigned char buf[3];
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, 2);
    RS232_SendByte(cport_nr, 255);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    RS232_PollComport(cport_nr, buf, 3);
    recieveParameters();
}

void BpmRs232Manager::recieveParameters()
{
    unsigned char buf[38];
    int n = RS232_PollComport(cport_nr, buf, 38);
    if (n == 38 && buf[0] == 111 && buf[37] == 222) {
        for (int i = 0; i < 6; i++) {
            beamLocation[i] = ((uint16_t*)(buf + 1))[i];
        }
        intensity = *((uint32_t*)(buf + 13));
        fwhm[0] = *((uint16_t*)(buf + 17));
        fwhm[1] = *((uint16_t*)(buf + 19));
        skewness[0] = *((float*)(buf + 21));
        skewness[1] = *((float*)(buf + 25));
        deviation[0] = *((float*)(buf + 29));
        deviation[1] = *((float*)(buf + 33));
        std::cout << "parameter fetching success" << std::endl;
        if (!connected) {
            connected = true;
        }
        
    }

}

void BpmRs232Manager::requestPlot()
{
    unsigned char buf[255];
    RS232_flushRXTX(cport_nr);
    Sleep(100);
    RS232_PollComport(cport_nr, buf, 255);
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, 3);
    RS232_SendByte(cport_nr, 255);
    Sleep(2000);                                                                    //wait for the plot data to become available
    RS232_PollComport(cport_nr, buf, 3);
    int n = RS232_PollComport(cport_nr, ((unsigned char*)plot), 16668);
    std::cout << n << std::endl;
    plotDataAvailable = true;
}

void BpmRs232Manager::changeTriggerDelay()
{
    //clear buffer
    unsigned char buf[255];
    RS232_flushRXTX(cport_nr);
    Sleep(100);
    RS232_PollComport(cport_nr, buf, 255);

    //send new parameters
    std::cout << "parameters changed" << std::endl;
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, 0);
    RS232_SendByte(cport_nr, triggerDelay);

    std::this_thread::sleep_for(std::chrono::microseconds(10));
    //recieve eccho
    RS232_PollComport(cport_nr, buf, 3);
    std::cout << "eccho received" << std::endl;
    for (int i = 0;i < 3;i++) {
        std::cout << +buf[i] << std::endl;
    }
    if (buf[0] == 255) {
        if (buf[1] == 0) {
            std::cout << "command recognized" << std::endl;
            ecchoMessage = "eccho message received trigger delay has now been set to = ";
            ecchoMessage.append(std::to_string(buf[2]));
            ecchoMessage.append("ms");
        }
    }
    else {
        ecchoMessage = "failed to recognize eccho message";
    }
}

