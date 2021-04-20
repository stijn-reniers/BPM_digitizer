#pragma once
#include "BpmCommunicationManager.h"
#include <curl/curl.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class BpmHttpManager :public BpmCommunicationManager
{
public:

    // Inherited via BpmCommunicationManager
    virtual bool setupCommunication() override;
    virtual void requestData() override;
    virtual void cleanUpCommunication() override;

    // Communication dependent functions
    void requestPlot();
    void parameterPost(bool activate);
    void parameterGet();
    void plotGet();
    void plotPost(bool activate);
    void jsonParseParameters();
    void jsonParsePlot();
private:
    json j;
    CURL* curl;
    CURLcode res;

    
};


std::size_t getParameterCallback(const char* in, std::size_t size, std::size_t num, json* out);
std::size_t getPlotCallback(const char* in, std::size_t size, std::size_t num, json* out);
