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
        if (newDcCorrection) {
            setDcCorrection();
            newDcCorrection = false;
        }
        if (counter >= plotUpdateFrequency && connected) {
            requestPlot();
            ecchoMessage = "Plot updated";
            counter = 0;
        }
        counter++;
        RS232_flushRXTX(cport_nr);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
    

void BpmRs232Manager::cleanUpCommunication()
{
    RS232_CloseComport(cport_nr);
}

void BpmRs232Manager::setDcCorrection()
{
    int enable = 0;
    if (dcCorrection) {
        enable = 255;
    }
    editSettings(enable, 4);
}

void BpmRs232Manager::requestParameters()
{
    unsigned char buf[3];
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, 2);
    RS232_SendByte(cport_nr, 255);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    RS232_PollComport(cport_nr, buf, 2);
    if (buf[0] == 38) {
        recieveParameters();
    }
}

void BpmRs232Manager::recieveParameters()
{
    unsigned char buf[38];
    int n = RS232_PollComport(cport_nr, buf, 36);
        for (int i = 0; i < 6; i++) {
            beamLocation[i] = ((uint16_t*)(buf))[i];
        }
        intensity = *((uint32_t*)(buf + 12));
        fwhm[0] = *((uint16_t*)(buf + 16));
        fwhm[1] = *((uint16_t*)(buf + 18));
        uint32_t helper = *((uint32_t*)(buf + 20));
        skewness[0] = static_cast<float>(*((int32_t*)(buf + 20))) / 10000.0;
        skewness[1] = static_cast<float>(*((int32_t*)(buf + 24))) / 10000.0;
        deviation[0] = static_cast<float>(*((int32_t*)(buf + 28))) / 10000.0;
        deviation[1] = static_cast<float>(*((int32_t*)(buf + 32))) / 10000.0;
        std::cout << "parameter fetching success" << std::endl;
        if (!connected) {
            connected = true;
        }

}

void BpmRs232Manager::requestPlot()
{
    unsigned char buf[255];
    Sleep(100);
    RS232_PollComport(cport_nr, buf, 255);
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, 3);
    RS232_SendByte(cport_nr, 255);
    Sleep(3000);                                                                    //wait for the plot data to become available
    RS232_PollComport(cport_nr, buf, 2);
    int n = RS232_PollComport(cport_nr, ((unsigned char*)plot), 16668);
    std::cout << n << std::endl;
    plotDataAvailable = true;
}

void BpmRs232Manager::changeTriggerDelay()
{
    editSettings(triggerDelay, 0);
}

void BpmRs232Manager::editSettings(char sendValue, char commandIndex)
{
    //clear rs232 buffer
    unsigned char buf[255];
    RS232_PollComport(cport_nr, buf, 255);

    //send new parameters
    std::cout << "parameters changed" << std::endl;
    RS232_SendByte(cport_nr, 255);
    RS232_SendByte(cport_nr, commandIndex);
    RS232_SendByte(cport_nr, sendValue);
    std::this_thread::sleep_for(std::chrono::microseconds(100));

    //recieve eccho
    RS232_PollComport(cport_nr, buf, 2);
    RS232_PollComport(cport_nr, buf, 3);
    std::cout << "eccho received" << std::endl;
    for (int i = 0;i < 3;i++) {
        std::cout << +buf[i] << std::endl;
    }
    if (buf[0] == 255) {
        if (commandIndex == 0 && buf[1] == 0 ) {
            std::cout << "command recognized" << std::endl;
            ecchoMessage = "eccho message received trigger delay has now been set to = ";
            ecchoMessage.append(std::to_string(buf[2]));
            ecchoMessage.append("ms");
        }
        if (commandIndex == 4 && buf[1] == 4) {
            if (buf[2] == 255) {
                ecchoMessage = "DC compensation enabled.";
            }
            else
            {
                ecchoMessage = "DC compensation disabled.";
            }
        }
    }
    else {
        ecchoMessage = "failed to recognize eccho message";
    }
}

