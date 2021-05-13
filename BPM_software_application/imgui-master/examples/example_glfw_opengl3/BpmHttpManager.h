#pragma once
#include "BpmCommunicationManager.h"
#include <curl/curl.h>
#include "nlohmann/json.hpp"
#include <string>
#include <sstream>
#include <algorithm>
using json = nlohmann::json;

class BpmHttpManager :public BpmCommunicationManager
{
public:
    BpmHttpManager(int portNr) {
        serverSite = "http://localhost:";
        serverSite.append(std::to_string(portNr));
        serverSite.append("/api/latest");
    };
    // Inherited via BpmCommunicationManager
    virtual bool setupCommunication() override;
    virtual void requestData() override;
    virtual void cleanUpCommunication() override;

    // Communication dependent functions
    void requestPlot();
    void parameterPost(bool activate);
    void parameterGet();
    void plotGet();
    void plotPost();
    void jsonParseParameters();
    void jsonParsePlot();
    void triggerDelayPost();
private:
    json j;
    CURL* curl;
    CURLcode res;
    std::string result;
    std::string serverSite;
    bool updateEchoMessage = false;
};

//callback function that transfers received JSON string to the result string
std::size_t getCallback(const char* in, std::size_t size, std::size_t num, std::string* out);
