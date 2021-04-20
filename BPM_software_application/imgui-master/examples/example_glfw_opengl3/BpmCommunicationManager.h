#pragma once
#include <string>
#include <string_view>
class BpmCommunicationManager
{
public:
    virtual void setupCommunication() = 0;
    virtual void requestData()=0;
    virtual void cleanUpCommunication() = 0;
    std::string_view getEcchoMessage() { return ecchoMessage; };
    uint16_t* getBeamLocation(){return beamLocation};
    float* getDeviation();
    uint32_t getIntensity() { return intensity };
    uint16_t* getFwhm() {return fwhm};
    bool newPlotDataAvailable() { return plotDataAvailable };
    void setPlotDataAvailable(bool status) {plotDataAvailable= status};
    uint16_t* getPlot() { return plot };
    
protected:
    std::string ecchoMessage;
    uint16_t beamLocation[6] = { 0 };
    float deviation[2] = { 0 };
    uint32_t intensity = 0;
    uint16_t fwhm[2] = { 0 };
    float skewness[2] = { 0 };
    uint16_t plot[8334] = { 0 };
    bool plotDataAvailable=false;
};

