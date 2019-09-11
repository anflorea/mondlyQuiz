#ifndef GENERALSTORE_H
#define GENERALSTORE_H

#include "Store.h"
#include "LobbyModel.h"
#include "QuestionModel.h"
#include "UserModel.h"

class GeneralStore : public Store
{
    Q_OBJECT

public:
    explicit GeneralStore(StoreIdsHelper::StoreId id, IDispatcher* dispatcher);

    QML_READONLY_VAR_PROPERTY(bool, isLoggedIn, false)
    QML_READONLY_VAR_PROPERTY(QString, currentUser, QString())
    QML_READONLY_VAR_PROPERTY(int, currentPoints, 0)

    QML_OBJMODEL_PROPERTY(LobbyModel, currentLobbies, Q_NULLPTR)
    QML_READONLY_VAR_PROPERTY(int, joinedLobby, -1)

    QML_READONLY_PTR_PROPERTY(QuestionModel, currentQuestion, Q_NULLPTR)

    QML_OBJMODEL_PROPERTY(UserModel, lastGameStats, Q_NULLPTR)

    void notify(const Action &action) override;
    void clear() override;

    void handleEvent(Json::Value event);

signals:
    void authError();

    void joinedLobby();
    void leftLobby();

    void gameStarted();
    void gameEnded();

    void gotNewQuestion();

    void answerCorrect();
    void answerWrong();

    void returnToLobby();
};

#endif // GENERALSTORE_H
