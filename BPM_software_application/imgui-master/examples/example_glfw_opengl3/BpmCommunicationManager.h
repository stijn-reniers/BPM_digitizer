#pragma once
#include <string>
#include <string_view>
#include <iostream>
#include <stdio.h>
#include <thread>
class BpmCommunicationManager
{
public:
    virtual bool setupCommunication() = 0;
    virtual void requestData()=0;
    virtual void cleanUpCommunication() = 0;
    std::string_view getEcchoMessage() { return ecchoMessage; };
    uint16_t* getBeamLocation() { return beamLocation; };
    float* getDeviation() { return deviation; };
    uint32_t* getIntensity() { return &intensity; };
    uint16_t* getFwhm() { return fwhm; };
    float* getSkewness() { return skewness; };

    bool newPlotDataAvailable() { return plotDataAvailable; };
    void setPlotDataAvailable(bool status) { plotDataAvailable = status; };
    uint16_t* getPlot() { return plot; };

    void setRunning(bool status) { running = status; };
    void setPlotUpdateFrequency(int freq) { plotUpdateFrequency = freq; };
    void updateTriggerDelay(int delay) { newTriggerDelay = true; triggerDelay = delay; };
protected:
    std::string ecchoMessage="";
    uint16_t beamLocation[6] = { 0 };
    float deviation[2] = { 0 };
    uint32_t intensity = 0;
    uint16_t fwhm[2] = { 0 };
    float skewness[2] = { 0 };
    uint16_t plot[8334] = { 0 };
    bool plotDataAvailable=false;
    bool running = true;
    int plotUpdateFrequency = 0;
    bool newTriggerDelay = false;
    int triggerDelay = 0;

};

