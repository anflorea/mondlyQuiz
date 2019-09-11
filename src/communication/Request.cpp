#include "Request.h"

#include <QFileInfo>
#include "RequestService.h"

Request::Request(QString _url, HttpMethods method, RequestService *initiatingService)
{
    m_requestService = initiatingService;
    logVerbosity = initiatingService->m_logVerbosityLevel;
    url = m_requestService->resolveUrl(_url);
    httpMethod = method;

    // setup headers
    headers.insert("Connection", "close");
    headers.insert("Accept", "application/json");
    Json::Value customHeaders = m_requestService->getHeaders();
    for (auto key : customHeaders.getMemberNames()) {
        headers.insert(QString::fromStdString(key), QString::fromStdString(customHeaders[key].asString()));
    }
}

QString Request::methodAsString()
{
    switch (httpMethod)
    {
        case HttpMethods::GET:
            return QString("GET");
        case HttpMethods::POST:
            return QString("POST");
        case HttpMethods::PATCH:
            return QString("PATCH");
        case HttpMethods::PUT:
            return QString("PUT");
        case HttpMethods::DELETE:
            return QString("DELETE");
    }
    return QString();
}

Request *Request::setPayload(Json::Value data)
{
    payload = data;
    switch (httpMethod)
    {
        case HttpMethods::GET:
        {
            break;
        }
        case HttpMethods::POST:
        case HttpMethods::PATCH:
        case HttpMethods::PUT:
        case HttpMethods::DELETE:
        {
            headers.insert("Content-Type", "application/json");
            break;
        }
    }
    return this;
}

Request *Request::uploadFile(QString path)
{
    Q_ASSERT_X(QFileInfo::exists(path), Q_FUNC_INFO, QString("File does not exist for path: " + path).toStdString().c_str());

    uploadPath = path;
    return this;
}

Request *Request::setUploadFileName(QString fileName)
{
    uploadFileName = fileName;
    return this;
}

void Request::onSuccess(std::function<void (Response*)> successCallback)
{
    success = successCallback;
}

void Request::onFail(std::function<void (Response*)> failCallback)
{
    fail = failCallback;
}

void Request::onProgress(std::function<void (double)> progressCallback)
{
    progress = progressCallback;
}

void Request::onConnectionError(std::function<void (Response *)> connectionErrorCallback)
{
    connectionError = connectionErrorCallback;
}

void Request::setNoRetries()
{
    retryOnServerError = false;
    retryOnTimeout = false;
}

void Request::setNoRetriesOnTimeout()
{
    retryOnTimeout = false;
}

void Request::setNoRetriesOnServerError()
{
    retryOnServerError = false;
}

void Request::setNoTimeout()
{
    cancelAfterMs = -1;
}
