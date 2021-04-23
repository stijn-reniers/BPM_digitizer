#include "BpmHttpManager.h"



bool BpmHttpManager::setupCommunication()
{
    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
    parameterPost(true);
    return curl;
}

void BpmHttpManager::requestData()
{
    int counter = 0;
    while (running) {

        if (counter > plotUpdateFrequency) {
            requestPlot();
            counter = 0;
        }
        else {
            parameterGet();
            ecchoMessage = "Plot updated";
        }
        if (newTriggerDelay) {
            ecchoMessage = "trigger delay update send";
            triggerDelayPost();
            newTriggerDelay = false;
        }
        counter++;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void BpmHttpManager::cleanUpCommunication()
{
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}


void BpmHttpManager::requestPlot()
{
    parameterPost(false);
    plotPost();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    plotGet();
    parameterPost(true);
}

void BpmHttpManager::plotGet()
{
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:80/api/latest");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = NULL;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    if (result.length() > 17000) {
        jsonParsePlot();
    }
    result = "";
}

void BpmHttpManager::plotPost()
{
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:80/api/latest");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string body = "{  \"request_id\" : \"2\",  \"get_plot_data\" : true}";
    
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

}

void BpmHttpManager::jsonParseParameters()
{
    j = json::parse(result.c_str());
    beamLocation[0] = *j.find("peak_x_max");
    beamLocation[1] = *j.find("peak_x_start");
    beamLocation[2] = *j.find("peak_x_end");
    beamLocation[3] = *j.find("peak_y_max");
    beamLocation[4] = *j.find("peak_y_start");
    beamLocation[5] = *j.find("peak_y_end");
    std::string str = *j.find("peak_x_variance");
    deviation[0] = std::stof(str);
    str = *j.find("peak_y_variance");
    deviation[1] = std::stof(str);
    str = *j.find("skewness_x");
    skewness[0] = std::stof(str);
    str = *j.find("skewness_y");
    skewness[1] = std::stof(str);
    intensity = *j.find("beam_intensity");
    fwhm[0] = *j.find("fwhm_x");
    fwhm[1] = *j.find("fwhm_y");

}

void BpmHttpManager::jsonParsePlot()
{
    j = json::parse(result.c_str());
    std::string data = (*j.find("data"));
    std::cout << "data length: " << data.length() << std::endl;
    std::replace(data.begin(), data.end(), ';', ' ');
    std::stringstream ss(data);
    int i = 0;
    while (i<8334 && ss >> plot[i]) {
        i++;
    }
    plotDataAvailable = true;

}

void BpmHttpManager::triggerDelayPost()
{
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:80/api/latest");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string body = "{  \"request_id\" : \"3\",  \"set_trigger_delay\" : ";
    body.append(std::to_string(triggerDelay));
    body.append("}");
    std::cout << body << std::endl;

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
}

void BpmHttpManager::parameterPost(bool activate)
{
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:80/api/latest");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    //headers = curl_slist_append(headers, "content-length: 55");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    std::string body = "{  \"request_id\" : \"1\",  \"get_beam_params\" : ";
    if (activate) {
        body.append("true}");

    }
    else {
        body.append("false}");
    }
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }


}

void BpmHttpManager::parameterGet()
{
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:80/api/latest");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = NULL;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    if (result.length() > 17000 || (result.length() < 500 && result.length()>400)) {
        jsonParseParameters();
    }
    result = "";

}

std::size_t getCallback(const char* in, std::size_t size, std::size_t num, std::string* out)
{
    const std::size_t totalBytes(size * num);
    // *out = json::parse(in);
    (*out).append(in);
    std::cout << (*out).length() << std::endl;
    return totalBytes;
}



