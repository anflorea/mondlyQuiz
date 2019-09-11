#ifndef REQUESTSERVICE_H
#define REQUESTSERVICE_H

#include <QObject>
#include <QQueue>
#include <QTimer>

#include "Request.h"
#include "Response.h"
#include "INetworkManager.h"

class RequestService : public QObject
{
    friend class Request;
public:
    RequestService();

    Request* GET(QString url);
    Request* POST(QString url);
    Request* PATCH(QString url);
    Request* PUT(QString url);
    Request* DELETE(QString url);

    // maybe replace this with send function on Request
    void sendRequest(Request* request);
    void cancelRequest(QString requestId);

    void addUpgradeVersionCallback(std::function<void()> updateAppCallback) {
        m_updateAppCallback = updateAppCallback;
    }

    void setSessionInvalidatedCallback(std::function<void()> sessionInvalidated) {
        m_sessionInvalidated = sessionInvalidated;
    }

    void setAccessToken(QString accessToken);
    QString getAccessToken() { return m_accessToken; }
    Json::Value getHeaders() { return m_headers; }

    bool isLoggedIn() { return !m_accessToken.isEmpty(); }

private:
    INetworkManager*                m_manager;
    QQueue<Request*>                m_pendingRequests;
    QHash<QString, Request*>        m_activeRequests;
    const int                       m_maxActiveRequests = 10;
    const int                       m_maxDelayMs = 7 * 1000;
    const int                       m_delayedRetryStepMs = 1000;
    QTimer                          checkupTimer;
    Json::Value                     m_headers;

    int m_logVerbosityLevel = 0;
    // 0 - no logs at all
    // 1 - just request url and response status code (plus size and time)
    // 2 - same as previous + headers for reply only
    // 3 - same as previous + request readers too
    // 4 - everything, except response body is limited to 256 characters
    // 5 - everything

    // auth data
    QString m_accessToken;
    QString m_apiKey;

    // special callbacks
    std::function<void()> m_updateAppCallback;
    std::function<void()> m_sessionInvalidated;

    void setUserAgentHeader();

    void evaluateQueue();
    void sendRequestToManager(Request* request);
    void retryRequest(Request* request);

    void requestFinished(Response* response);
    void uploadProgress(QString requestId, double progress);

    void cancellAllRequests();

    QString resolveUrl(QString url) {
        if (url.at(0) == QChar('/'))
            return DEFINE_BASE_URL + url;
        else
            return url;
    }

    void emitInformativeSignals(Request* request, Response* response);
    void log(Request* request, Response* response);

private slots:
    void checkActiveRequests();
};
#endif // REQUESTSERVICE_H
