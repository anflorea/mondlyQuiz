#ifndef ACTIONTYPES_H
#define ACTIONTYPES_H

#include <QObject>

class ActionType : public QObject
{
    Q_OBJECT

public:

    enum Type {
        Notype = 0,

        AuthStarted,
        AuthSuccess,
        AuthError,

        LogoutSuccess,

        GotUsername,

        GetLobbysStarted,
        GetLobbysSuccess,
        GetLobbysError,

        CreateLobbySuccess,
        CreateLobbyFail,

        JoinLobbySuccess,
        JoinLobbyFail,

        LeaveLobbySuccess,
        LeaveLobbyFail,

        AnswerQuestionSuccess,
        AnswerQuestionFail,

        ReturnToLobby,

        GotEvent,
    };
    Q_ENUM(Type)
};
Q_DECLARE_METATYPE(ActionType::Type);

#endif // ACTIONTYPES_H
