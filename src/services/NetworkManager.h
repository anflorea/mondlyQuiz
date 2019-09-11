#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <functional>
#include "INetworkManager.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkSession>
#include <QNetworkConfiguration>
#include <QHttpMultiPart>
#include <QMimeDatabase>
#include <QTimer>

#include "json/json.h"

class Request;
class Response;

class NetworkManager : public QObject, public INetworkManager
{
    Q_OBJECT

public:
    NetworkManager(std::function<void(Response*)> finishedCallback,
                   std::function<void (QString, double)> progressCallback,
                   std::function<void(Response*)> errorCallback,
                   std::function<void(Response*)> readyReadCallback = nullptr
                   );

    void sendRequest(Request* request) override;

    void cancelRequest(QString requestId) override;

private:
    QNetworkAccessManager* innerManager;
    QHash<QString, QNetworkReply*> activeRequests;
    QTimer checkupTimer;
    Json::Reader jsonReader;

    QNetworkReply* buildAndSendRequest(Request* request);
    void setupReplyConnections(Request* request, QNetworkReply* reply);

private slots:
    void replyFinished(QString requestId);
    void replyError(QString requestId);
    void replyReadyRead(QString requestId);
    void replyUploadProgress(QString requestId, qint64 bytesSent, qint64 bytesTotal);
};

#endif // NETWORKMANAGER_H
