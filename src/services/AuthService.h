#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <QTimer>
#include <QtWebSockets/QtWebSockets>

#include "IAuthService.h"
#include "Defines.h"

class RequestService;

class AuthService : public IAuthService
{
    Q_OBJECT
public:
    AuthService(RequestService*);

    void signIn(const QString& username,
                const QString& password,
                const std::function<void(QString, QString)>& success = nullptr,
                const std::function<void(Response*)>& fail = nullptr) override;

    void signOut(QString accessToken,
                 const std::function<void(void)>& success = nullptr,
                 const std::function<void(Json::Value)>& fail = nullptr) override;


    void connectToEventStream(const std::function<void(void)>& success = nullptr,
                              const std::function<void(Json::Value)>& gotEvent = nullptr,
                              const std::function<void()>& fail = nullptr) override;

protected:
    RequestService *m_requestService;

    QWebSocket m_quizSocket;
    QTimer m_quizPingTimer;

    QString m_baseWsUrl = QString("ws://" + QString(DEFINE_BASE_URL).replace("http://", "").replace("https://", ""));
};

#endif // AUTHSERVICE_H
