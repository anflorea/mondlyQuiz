#ifndef REQUEST_H
#define REQUEST_H

#include <functional>
#include <QUrl>

#include "Response.h"
#include "Defines.h"
#include "Utils.h"

enum HttpMethods {
    GET,
    POST,
    PATCH,
    PUT,
    DELETE
};

class RequestService;

class Request
{
    friend class RequestService;
private:
    const QString id = Utils::randomUUID();
    QString url;
    HttpMethods httpMethod;
    Json::Value payload = Json::nullValue;
    QString uploadPath;
    QString uploadFileName = QString();
    int delayMs = 0;
    qint64 sendTimestampMs = 0;
    bool retryOnServerError = true;
    bool retryOnTimeout = true;
    int cancelAfterMs = 10 * 1000;
    int logVerbosity = 0;
    bool wasManuallyCanceled = false;

    std::function<void(Response*)> success = nullptr;
    std::function<void(Response*)> fail = nullptr;
    std::function<void(double)> progress = nullptr;
    std::function<void(Response*)> connectionError = nullptr;

public:
    QHash<QString, QString> headers;

    Request(QString url, HttpMethods method, RequestService* initiatingService);

    QString methodAsString();

    Request* setPayload(Json::Value data);
    Request* uploadFile(QString path);
    Request* setUploadFileName(QString fileName);

    void onSuccess(std::function<void(Response*)> successCallback);
    void onFail(std::function<void(Response*)> failCallback);
    void onProgress(std::function<void(double)> progressCallback);
    void onConnectionError(std::function<void(Response*)> connectionErrorCallback);

    void setNoRetries();
    void setNoRetriesOnTimeout();
    void setNoRetriesOnServerError();
    void setNoTimeout();
    void setLogging(bool value = true) {
        if (value)
            logVerbosity = 5;
        else
            logVerbosity = 0;
    }

    QString getId() const
    {
        return id;
    }

    QString getUrl() const
    {
        return url;
    }

    HttpMethods getHttpMethod() const
    {
        return httpMethod;
    }

    Json::Value getPayload() const
    {
        return payload;
    }

    QString getUploadPath() const
    {
        return uploadPath;
    }

    QString getUploadFileName() const
    {
        return uploadFileName;
    }

    QString getTimeout() const
    {
        return QString::number(cancelAfterMs);
    }

    QString synthesizeUrlWithGETParams() const
    {
        QString urlWithParams = this->getUrl() + "?";
        Json::Value data = this->getPayload();

        for (auto itr = data.begin(); itr != data.end(); itr++ ) {
            QString key = QString::fromStdString(itr.key().asString());
            QString value;
            Json::Value itrVal = data[itr.key().asString()];
            if (itrVal.isArray()) {
                for (Json::Value innerValue : itrVal) {
                    value += key + "=" + QString::fromUtf8(QUrl::toPercentEncoding(QString::fromStdString(innerValue.asString()))) + "&";
                }

                if (value.length())
                    value.chop(1);
                urlWithParams += ( value + "&");
            } else {
                value = QString::fromStdString(data[itr.key().asString()].asString());
                value = QString::fromUtf8(QUrl::toPercentEncoding(value));
                urlWithParams += (key + "=" + value + "&");
            }

        }
        urlWithParams.chop(1);

        return urlWithParams;
    }

    void setTimeout(int ms) {
        cancelAfterMs = ms;
    }

private:
    RequestService* m_requestService;
};

#endif // REQUEST_H

