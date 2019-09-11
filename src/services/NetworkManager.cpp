#include "NetworkManager.h"

#include "RequestService.h"
#include <QFileInfo>

NetworkManager::NetworkManager(std::function<void (Response*)> finishedCallback, std::function<void (QString, double)> progressCallback, std::function<void (Response*)> errorCallback, std::function<void (Response*)> readyReadCallback)
{
    m_finishedCallback = finishedCallback;
    m_uploadProgressCallback = progressCallback;
    m_errorCallback = errorCallback;
    m_readyReadCallback = readyReadCallback;
}

void NetworkManager::sendRequest(Request *request)
{
    auto reply = buildAndSendRequest(request);
    activeRequests.insert(request->getId(), reply);
    setupReplyConnections(request, reply);
}

void NetworkManager::cancelRequest(QString requestId)
{
    if (activeRequests.contains(requestId)) {
        auto reply = activeRequests.value(requestId);
        if (reply)
            reply->abort();
        else
            qWarning() << "hash contains request id but reply is null!";
    }
}

QNetworkReply *NetworkManager::buildAndSendRequest(Request *request)
{
    innerManager = new QNetworkAccessManager(this);

    QNetworkRequest builtRequest;
    QNetworkReply* reply;

    // set url
    builtRequest.setUrl(request->getUrl());

    // set custom headers
    for (auto key : request->headers.keys()) {
        builtRequest.setRawHeader(key.toLocal8Bit(), request->headers.value(key).toLocal8Bit());
    }

    // set payload and send
    if (request->getUploadPath().isEmpty()) {

        QByteArray requestData;
        if (!request->getPayload().isNull()) {
            switch (request->getHttpMethod())
            {
                case HttpMethods::GET:
                {
                    builtRequest.setUrl(request->synthesizeUrlWithGETParams());
                    break;
                }
                case HttpMethods::POST:
                case HttpMethods::PATCH:
                case HttpMethods::PUT:
                case HttpMethods::DELETE:
                {
                    requestData = request->getPayload().toStyledString().c_str();
                    break;
                }
            }
        }
        reply = innerManager->sendCustomRequest(builtRequest, request->methodAsString().toUtf8(), requestData);
    } else {
        QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QFile* uploadFile = new QFile(request->getUploadPath());

        // maybe throw exceptions in these cases
        if (!uploadFile->exists())
            qWarning() << "File to upload does not exist -> " << request->getUploadPath();
        if (!uploadFile->open(QIODevice::ReadOnly))
            qWarning() << "Could not open file to upload -> " << request->getUploadPath();
        uploadFile->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

        QMimeDatabase mimeDb;
        QMimeType uploadMimeType = mimeDb.mimeTypeForData(uploadFile);
        // NOTE: maybe check if mime type is among those we support (using inherit as docs suggest)

        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(uploadMimeType.name()));
        QFileInfo fileInfo(request->getUploadPath());
        QString fileSuffix = fileInfo.suffix();
        QString uploadFileKey = "file";

        QString uploadFileFilename = request->getUploadFileName();
        if (uploadFileFilename.isEmpty())
            uploadFileFilename = uploadFileKey + "." + fileSuffix;

        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + uploadFileKey + "\"; filename=\"" + uploadFileFilename + "\""));
        filePart.setBodyDevice(uploadFile);

        multiPart->append(filePart);

        reply = innerManager->sendCustomRequest(builtRequest, request->methodAsString().toUtf8(), multiPart);
        multiPart->setParent(reply); // delete the multiPart with the reply
    }

    return reply;
}

void NetworkManager::setupReplyConnections(Request* request, QNetworkReply *reply)
{
    connect(reply, &QNetworkReply::finished, this, [this, request]() {
        this->replyFinished(request->getId());
    });
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, [this, request](QNetworkReply::NetworkError code) {
        Q_UNUSED(code)
        this->replyError(request->getId());
    });
    connect(reply, &QNetworkReply::readyRead, this, [this, request]() {
        this->replyReadyRead(request->getId());
    });
    connect(reply, &QNetworkReply::uploadProgress, this, [this, request](qint64 bytesSent, qint64 bytesTotal) {
        this->replyUploadProgress(request->getId(), bytesSent, bytesTotal);
    });
    // REQ TODO add other useful QNetworkReply signals
}

void NetworkManager::replyFinished(QString requestId)
{
    Q_ASSERT_X(activeRequests.contains(requestId), Q_FUNC_INFO, "Request id not found in active requests/replies");

    auto reply = activeRequests.value(requestId);

    // build response
    Response *r = new Response;
    r->id = requestId;
    r->statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    r->reasonPhrase = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

    // parse headers
    for (auto header : reply->rawHeaderList()) {
        r->headers.insert(QString(header).toLower(), reply->rawHeader(header));
    }

    // read reply
    r->rawResponse = reply->readAll();
    r->size = r->rawResponse.size();

    // parse to Json based on Content-Type
    //    if (r->headers.contains("Content-Type") && r->headers.value("Content-Type").contains("application/json")) {
    bool ok = jsonReader.parse(r->rawResponse.toStdString(), r->responseJson);
    if (!ok) {
        r->responseJson = Json::nullValue;
        qWarning() << "Could not parse json from response!";
        qWarning() << r->rawResponse;
    }

    m_finishedCallback(r);
    activeRequests.remove(requestId);
    reply->manager()->deleteLater();
    reply->deleteLater();
}

void NetworkManager::replyError(QString requestId)
{
    Q_ASSERT_X(activeRequests.contains(requestId), Q_FUNC_INFO, "Request id not found in active requests/replies");
}

void NetworkManager::replyReadyRead(QString requestId)
{
    Q_ASSERT_X(activeRequests.contains(requestId), Q_FUNC_INFO, "Request id not found in active requests/replies");
}

void NetworkManager::replyUploadProgress(QString requestId, qint64 bytesSent, qint64 bytesTotal)
{
    Q_ASSERT_X(activeRequests.contains(requestId), Q_FUNC_INFO, "Request id not found in active requests/replies");

    if (m_uploadProgressCallback) {

        if (bytesTotal > 0) {
            double progress = static_cast<double>(bytesSent) / static_cast<double>(bytesTotal);
            m_uploadProgressCallback(requestId, progress);
        }
    }
}
