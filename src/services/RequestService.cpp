#include "RequestService.h"

#include <QHostInfo>
#include <QSysInfo>

# include "NetworkManager.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>

RequestService::RequestService()
{
//    m_headers["Api-Key"] =  DEFINE_API_KEY;

    auto finished = [=](Response* r) {
        this->requestFinished(r);
    };

    auto uploadProgress = [=](QString requestId, double progress) {
        this->uploadProgress(requestId, progress);
    };

    m_manager = new NetworkManager(finished, uploadProgress, nullptr);

    checkupTimer.setInterval(10 * 1000);
    connect(&checkupTimer, &QTimer::timeout, this , &RequestService::checkActiveRequests);
    checkupTimer.start();

    setUserAgentHeader();
}

Request *RequestService::GET(QString url)
{
    Request* req = new Request(url, HttpMethods::GET, this);
    return req;
}

Request *RequestService::POST(QString url)
{
    Request* req = new Request(url, HttpMethods::POST, this);
    return req;
}

Request *RequestService::PATCH(QString url)
{
    Request* req = new Request(url, HttpMethods::PATCH, this);
    return req;
}

Request *RequestService::PUT(QString url)
{
    Request* req = new Request(url, HttpMethods::PUT, this);
    return req;
}

Request *RequestService::DELETE(QString url)
{
    Request* req = new Request(url, HttpMethods::DELETE, this);
    return req;
}

void RequestService::sendRequest(Request *request)
{
    m_pendingRequests.enqueue(request);
    evaluateQueue();
}

void RequestService::cancelRequest(QString requestId)
{
    Q_ASSERT_X(m_activeRequests.contains(requestId), Q_FUNC_INFO, QString("No active request found for given id: " + requestId).toStdString().c_str());
    Request* request = m_activeRequests.value(requestId);
    request->wasManuallyCanceled = true;
    m_manager->cancelRequest(requestId);
}

void RequestService::evaluateQueue()
{
    while (!m_pendingRequests.isEmpty() && m_activeRequests.size() <= m_maxActiveRequests) {
        auto requestToSend = m_pendingRequests.dequeue();
        m_activeRequests.insert(requestToSend->id, requestToSend);

        sendRequestToManager(requestToSend);
    }

    if (m_activeRequests.isEmpty()) {
        checkupTimer.stop();
    } else if (!checkupTimer.isActive()) {
        checkupTimer.start();
    }
}

void RequestService::checkActiveRequests()
{
    qDebug() << Q_FUNC_INFO;
    for (auto reqKey : m_activeRequests.keys()) {

        auto request = m_activeRequests.value(reqKey);

        auto now = QDateTime::currentMSecsSinceEpoch();
        if (request->cancelAfterMs > 0 && (now - request->sendTimestampMs) > request->cancelAfterMs) {
            this->m_manager->cancelRequest(request->id);
        }
    }
}

void RequestService::sendRequestToManager(Request *request)
{
    QTimer::singleShot(request->delayMs, this, [=](){
        request->sendTimestampMs = QDateTime::currentMSecsSinceEpoch();
        m_manager->sendRequest(request);
    });
}

void RequestService::retryRequest(Request *request)
{
    request->delayMs += m_delayedRetryStepMs;

    sendRequestToManager(request);
}

void RequestService::requestFinished(Response* response)
{
    Q_ASSERT_X(m_activeRequests.contains(response->id), Q_FUNC_INFO, "No active request for response, something is very wrong");

    auto request = m_activeRequests.value(response->id);
    response->duration = QDateTime::currentMSecsSinceEpoch() - request->sendTimestampMs;

    // add request headers in response
    response->requestHeaders.clear();
    for (QString key : request->headers.keys()) {
        response->requestHeaders.insert(key, request->headers.value(key));
    }

    log(request, response);
    emitInformativeSignals(request, response);

    bool done = false;
    if (response->statusCode / 100 == 2) {
        done = true;
        if (request->success) request->success(response);
    }
    else if (response->statusCode / 100 == 4) {
        done = true;

        if (request->fail) request->fail(response);
    }
    else if (response->statusCode / 100 == 5) {
        if (request->retryOnServerError && request->delayMs < m_maxDelayMs) {
            retryRequest(request);
        }
        else
            done = true;
    } else if (response->statusCode == 0) {
        if (request->wasManuallyCanceled) {
            done = true;
            if (request->fail) request->fail(response);
        } else if (request->retryOnTimeout) { // if request should be retried on connection error
            if (request->delayMs < m_maxDelayMs) {
                retryRequest(request);
            } else { // if request retry delay has reached maximum delay, then move request back to pending
                m_activeRequests.remove(response->id);
                m_pendingRequests.enqueue(request);
            }
        } else {
            done = true;
            // TODO: make this logic clearer/more straightforward
            if (request->connectionError) request->connectionError(response);
        }
    } else {
        qWarning() << "UNHANDLED CASE, status code: " << response->statusCode;
    }

    if (done) {
        QVariantMap logData{
            { "request_id", request->getId() },
            { "method", request->methodAsString() },
            { "url", request->getUrl() },
            { "status code", response->statusCode },
            { "duration", response->duration },
        };

        QJsonDocument jsonDoc = QJsonDocument::fromJson(response->rawResponse);
        QVariantMap requestBody;
        if (!request->payload.isNull()) {
            auto doc = QJsonDocument::fromJson(request->payload.toStyledString().c_str());
            requestBody = doc.object().toVariantMap();
        }
        if (!request->getUploadPath().isEmpty()) {
            requestBody.insert("upload_file_path", request->getUploadPath());
        }
        QVariantMap reqHeaders;
        for (auto it = request->headers.begin(); it != request->headers.end(); ++it)
            reqHeaders.insert(it.key(), it.value());
        QVariantMap resHeaders;
        for (auto it = response->headers.begin(); it != response->headers.end(); ++it)
            resHeaders.insert(it.key(), it.value());

        QVariantMap privateData{
            { "request_headers", reqHeaders },
            { "request_body", requestBody },
            { "response_headers", resHeaders },
        };
        // special handling required because QVariantMap's initializer list doesn't accept QVariantList as value :(
        if (jsonDoc.isArray())
            privateData.insert("response_body", jsonDoc.array().toVariantList());
        else
            privateData.insert("response_body", jsonDoc.object().toVariantMap());

        m_activeRequests.remove(response->id);
        delete request;
        delete response;
        evaluateQueue();
    }
}

void RequestService::uploadProgress(QString requestId, double progress)
{
    Q_ASSERT_X(m_activeRequests.contains(requestId), Q_FUNC_INFO, "No active request for request, something is very wrong");

    auto request = m_activeRequests.value(requestId);
    if (request->progress) {
        request->progress(progress);
    }
}

void RequestService::emitInformativeSignals(Request *request, Response *response)
{
//    // status code 401 means that the access token has expired and the session is now invalid
//    if (response->statusCode == 401) {
//        cancellAllRequests();
//        m_sessionInvalidated();
//    }

//    // status code 419 means the user should update the application
//    if (response->statusCode == 419)
//        m_updateAppCallback();

//    // we do not emit server/connection error on first fail
//    bool isNotFirstFail = request->delayMs > 0; // NOTE: this is an unsafe assumption, will change when delayed requests will be actually needed

//    // server error case
//    if (response->statusCode / 100 == 5) {
//       // TODO: error handling where needed for each request
//    }

}

void RequestService::log(Request* request, Response* response)
{
    int oldVerbosity = m_logVerbosityLevel;

    // let request verbosity override default verbosity
    if (request->logVerbosity != m_logVerbosityLevel)
        m_logVerbosityLevel = request->logVerbosity;

    // failing requests get maximum verbosity
    if (response->statusCode / 100 == 4 || response->statusCode / 100 == 5)
        m_logVerbosityLevel = 5;

    if (m_logVerbosityLevel > 0) {

        // print request
        qDebug().noquote() << QString("=").repeated(90);
        qDebug().noquote() << "|" << request->url << request->methodAsString();

        if (m_logVerbosityLevel >= 3) {
            for (auto key : request->headers.keys())
                qDebug().noquote() << "|" << key << ": " << request->headers.value(key);
        }

        if (m_logVerbosityLevel >= 4) {
            if (!request->payload.isNull()) {
                qDebug() << "|" << "";
                qDebug("%s", request->payload.toStyledString().c_str());
            }
        }

        // print response
        qDebug().noquote() << "|" << QString("-").repeated(89);
        qDebug().noquote() << "|" << "Status:" << response->statusCode << response->reasonPhrase << "       Time:" << response->duration  << "ms" << "        Size:" << response->size << "B";

        if (m_logVerbosityLevel >= 2) {
            for (auto key : response->headers.keys())
                qDebug().noquote() << "|" << key << ": " << response->headers.value(key);
        }

        if (m_logVerbosityLevel >= 4) {
            if (!response->responseJson.isNull()) {
                qDebug() << "|" << "";
                if (m_logVerbosityLevel >= 5)
                    qDebug() << "|" << response->responseJson.toStyledString().c_str();
                else
                    qDebug() << "|" << response->responseJson.toStyledString().substr(0, 256).c_str();
            } else if (!response->rawResponse.isEmpty()) {
                qDebug() << "|" << "";
                qDebug() << "|" << response->rawResponse;
            }
        }
        qDebug().noquote() << QString("=").repeated(90);
    }

    // restore default service verbosity in case it was overriden by request
    m_logVerbosityLevel = oldVerbosity;
}

void RequestService::setAccessToken(QString accessToken)
{
    if (m_accessToken != accessToken) {
        m_accessToken = accessToken;

        m_headers["Authorization"] = QString("Bearer " + accessToken).toStdString();
    }
}

void RequestService::setUserAgentHeader()
{
    QString userAgent = QSysInfo::productType();
    userAgent += " | " + QHostInfo::localHostName();
    m_headers["User-Agent"] = userAgent.toStdString();
}

void RequestService::cancellAllRequests()
{
    for (QString key : m_activeRequests.keys()) {
        m_manager->cancelRequest(m_activeRequests.value(key)->getId());
    }
    m_pendingRequests.clear();
}
