#pragma once
#include "BpmCommunicationManager.h"
#include <curl/curl.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class BpmHttpManager :public BpmCommunicationManager
{
public:
    // Inherited via BpmCommunicationManager
    virtual void setupCommunication() override;
    virtual void requestData() override;
    virtual void cleanUpCommunication() override;

    // Communication dependent functions
    void requestParameters();
    void requstPlot();
    void parameterPost(bool activate);
    void parameterGet();
private:
    json j;
    CURL* curl;
    CURLcode res;

    
};

