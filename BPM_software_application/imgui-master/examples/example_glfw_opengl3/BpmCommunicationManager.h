#pragma once
#include <string>
#include <string_view>
#include <iostream>
#include <stdio.h>
#include <thread>
class BpmCommunicationManager
{
public:
    //interface functions
    /* Function that is called before the start op data acquisition
    * This function should initiate the communication medium*/
    virtual bool setupCommunication() = 0;

    /* Function that is called by a second thread, this function should keep repeating
    * until bool running = false. It is supposed to fetch the beam parameters, collector plot and should also check
    * wether or not the board settings need to be updated*/
    virtual void requestData()=0;
    /* Final function that is called after running has been set to false, this function should
    * should clean up the communication medium*/
    virtual void cleanUpCommunication() = 0;

    //beam parameter getters
    uint16_t* getBeamLocation() { return beamLocation; };
    float* getDeviation() { return deviation; };
    uint32_t* getIntensity() { return &intensity; };
    uint16_t* getFwhm() { return fwhm; };
    float* getSkewness() { return skewness; };
    uint16_t* getPlot() { return plot; };

    //functions that link the GUI with the second thread
    std::string_view getEcchoMessage() { return ecchoMessage; };
    bool newPlotDataAvailable() { return plotDataAvailable; };
    void setPlotDataAvailable(bool status) { plotDataAvailable = status; };
    void setRunning(bool status) { running = status; };
    void setPlotUpdateFrequency(int freq) { plotUpdateFrequency = freq; };
    void updateTriggerDelay(int delay) { newTriggerDelay = true; triggerDelay = delay; };
    void updatePeakThreshold(int threshold) { newPeakThreshold = true; peakThreshold = threshold; };
    void updateDcCorrection(bool status) { dcCorrection = status; newDcCorrection = true; }

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
    bool newPeakThreshold = false;
    int triggerDelay = 0;
    int peakThreshold = 0;
    bool dcCorrection = false;
    bool newDcCorrection = false;
};

