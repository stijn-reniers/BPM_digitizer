#include "BpmHttpManager.h"

void BpmHttpManager::setupCommunication()
{
    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
}

void BpmHttpManager::requestData()
{

}

void BpmHttpManager::cleanUpCommunication()
{
    curl_global_cleanup();
}

void BpmHttpManager::requestParameters()
{

}

void BpmHttpManager::requstPlot()
{
}

void BpmHttpManager::parameterPost(bool activate)
{
}

void BpmHttpManager::parameterGet()
{
}
