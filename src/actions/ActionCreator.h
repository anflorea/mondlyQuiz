#ifndef BASEACTIONCREATOR_H
#define BASEACTIONCREATOR_H

#include <QObject>
#include "Action.h"
#include "IDispatcher.h"
#include "Defines.h"

// services
class IServiceProvider;
class RequestService;
class IAuthService;
class GeneralService;

class ActionCreator : public QObject
{
    Q_OBJECT
public:
    explicit ActionCreator(IDispatcher* d,
                           IServiceProvider *serviceProvider,
                           QObject *parent = nullptr);

    Q_INVOKABLE void login(QString username, QString password);
    Q_INVOKABLE void logout();

    Q_INVOKABLE void getLobbys();

    Q_INVOKABLE void connectToEventStream();

    Q_INVOKABLE void createLobby();

    Q_INVOKABLE void joinLobby(QString lobbyId = "");

    Q_INVOKABLE void leaveLobby();

    Q_INVOKABLE void startGame();

    Q_INVOKABLE void answerQuestion(QString questionId, QString answerId);

    Q_INVOKABLE void returnToLobby();

protected:
    IDispatcher* m_dispatcher = nullptr;

    // services
    RequestService *m_requestService = nullptr;
    IAuthService *m_authService = nullptr;
    GeneralService *m_generalService = nullptr;
};

#endif // BASEACTIONCREATOR_H
