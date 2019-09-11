#include "AuthService.h"

#include "RequestService.h"

AuthService::AuthService(RequestService* reqService)
    : m_requestService(reqService)
{
    m_quizPingTimer.setInterval(10000);
}

void AuthService::signIn(const QString &username, const QString &password, const std::function<void (QString, QString)> &success, const std::function<void (Response*)> &fail)
{
    QString trimmedUser = username.trimmed();

    QString concatenated = trimmedUser + ":" + password;
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;

    Request* request = m_requestService->POST("/login");

    request->setLogging(true);

    request->headers.insert("Authorization", headerData);

    request->onSuccess([=](Response* response) {
        Json::Value value = response->bodyAsJson();

        QString token = QString::fromStdString(value.get("token", "").asString());
        QString username = QString::fromStdString(value.get("username", "").asString());

        if (success)
            success(token, username);
    });

    request->onFail([=](Response* response) {
        if (fail)
            fail(response);
    });

    m_requestService->sendRequest(request);
}

void AuthService::signOut(QString accessToken, const std::function<void ()> &success, const std::function<void (Json::Value)> &fail)
{
    Q_UNUSED(accessToken)
    Q_UNUSED(success)
    Q_UNUSED(fail)
    // TODO: implement if needed
}

void AuthService::connectToEventStream(const std::function<void ()> &success, const std::function<void (Json::Value)> &gotEvent, const std::function<void ()> &fail)
{
    QString pollSocketUrlString(m_baseWsUrl + "/quiz");
    QUrl pollSocketUrl(pollSocketUrlString);

    connect(&m_quizSocket, &QWebSocket::textMessageReceived, [=](QString message) {
        Json::Value pollJson;
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(message.toStdString().c_str(), pollJson);
        if (!parsingSuccessful)
        {
            qDebug() << "PARSE ERROR: "
                     << QString::fromStdString(reader.getFormattedErrorMessages());
        }
        else {
            if (gotEvent)
                gotEvent(pollJson);
        }
        // TODO: GENERIC JSON PARSE FUNCTION
       qDebug() << "[SOCKET]Poll Message: " << message;
    });

    connect(&m_quizSocket, &QWebSocket::connected, [=]() {
       qDebug() << "[SOCKET]✅ Poll Connected";
       if (success)
           success();
       m_quizPingTimer.start();
    });

    connect(&m_quizSocket, &QWebSocket::sslErrors, [=](const QList<QSslError> &errors) {
       qDebug() << "[SOCKET]🚫 Poll SSL Error";
       if (fail)
           fail();
    });

    connect(&m_quizSocket, &QWebSocket::disconnected, [=]() {
       qDebug() << "[SOCKET]🚫 Poll Disconnected";
       m_quizPingTimer.stop();
    });

    connect(&m_quizSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
        [=](QAbstractSocket::SocketError error){
        qDebug() << "[SOCKET]🚫 Poll Error: " << error;
    });

    connect(&m_quizPingTimer, &QTimer::timeout, [this](){
        m_quizSocket.ping();
    });

    m_quizSocket.open(pollSocketUrl);
}
