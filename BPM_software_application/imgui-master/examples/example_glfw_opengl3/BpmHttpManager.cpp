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
        parameterGet();

        //add changing settings

        if (counter > plotUpdateFrequency) {
            requestPlot();
            counter = 0;
        }
        counter++;
        Sleep(1000);
    }
}

void BpmHttpManager::cleanUpCommunication()
{
    curl_global_cleanup();
}


void BpmHttpManager::requestPlot()
{
    parameterPost(false);
    plotPost(true);
    plotGet();
    plotPost(false);
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getPlotCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &j);
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    /* Clean up after yourself */
    curl_easy_cleanup(curl);
}

void BpmHttpManager::plotPost(bool activate)
{
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:80/api/latest");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "content-length: 55");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    char* body = "{  \"request_id\" : \"1\",  \"get_plot_data\" : ";
    if (activate) {
        strcat(body, "true}");
    }
    else {
        strcat(body, "false}");
    }
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    /* Clean up after yourself */
    curl_easy_cleanup(curl);
}

void BpmHttpManager::jsonParseParameters()
{
    beamLocation[0] = *j.find("peak_x_max");
    beamLocation[1] = *j.find("peak_x_start");
    beamLocation[2] = *j.find("peak_x_end");
    beamLocation[3] = *j.find("peak_y_max");
    beamLocation[4] = *j.find("peak_y_start");
    beamLocation[5] = *j.find("peak_x_end");
    deviation[0] = *j.find("peak_x_variance");
    deviation[1] = *j.find("peak_y_variance");
    intensity = *j.find("beam_intensity");
    fwhm[0] = *j.find("fwhm_x");
    fwhm[1] = *j.find("fwhm_y");
    skewness[0] = *j.find("skewness_x");
    skewness[1] = *j.find("skewness_y");
}

void BpmHttpManager::jsonParsePlot()
{
}

void BpmHttpManager::parameterPost(bool activate)
{
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:80/api/latest");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "content-length: 55");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    char* body = "{  \"request_id\" : \"1\",  \"get_beam_params\" : ";
    if (activate) {
        strcat(body, "true}");
    }
    else {
        strcat(body, "false}");
    }
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    /* Clean up after yourself */
    curl_easy_cleanup(curl);

}

void BpmHttpManager::parameterGet()
{

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:80/api/latest");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = NULL;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getParameterCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &j);
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    /* Clean up after yourself */
    curl_easy_cleanup(curl);


}

std::size_t getParameterCallback(const char* in, std::size_t size, std::size_t num, json* out)
{
    const std::size_t totalBytes(size * num);
    *out = json::parse(in);
    return totalBytes;
}

std::size_t getPlotCallback(const char* in, std::size_t size, std::size_t num, json* out)
{
    const std::size_t totalBytes(size * num);
    *out = json::parse(in);
    return totalBytes;
}

