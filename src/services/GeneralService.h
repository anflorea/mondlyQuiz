#ifndef GENERALSERVICE_H
#define GENERALSERVICE_H

#include <QObject>
#include <QQuickItem>
#include "Response.h"

class RequestService;

class GeneralService
{
public:
    GeneralService(RequestService*);

    void getLobbys(const std::function<void(Json::Value)>& success = nullptr,
                   const std::function<void(Response*)>& fail = nullptr);

    void createLobby(const std::function<void(Json::Value)>& success = nullptr,
                     const std::function<void(Response*)>& fail = nullptr);

    void joinLobby(QString lobbyId,
                   const std::function<void(QString)>& success = nullptr,
                   const std::function<void(Response*)>& fail = nullptr);

    void leaveLobby(const std::function<void()>& success = nullptr,
                    const std::function<void(Response*)>& fail = nullptr);

    void startGame(const std::function<void(Json::Value)>& success = nullptr,
                   const std::function<void(Response*)>& fail = nullptr);

    void answerQuestion(QString questionId,
                        QString answer,
                        const std::function<void(Json::Value)>& success = nullptr,
                        const std::function<void(Response*)>& fail = nullptr);

private:
    RequestService *m_requestService = nullptr;
};

#endif // GENERALSERVICE_H
