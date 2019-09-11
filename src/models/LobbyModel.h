#ifndef LOBBYMODEL_H
#define LOBBYMODEL_H

#include "BaseModel.h"
#include "UserModel.h"

class LobbyModel : public BaseModel
{
    Q_OBJECT

    QML_OBJMODEL_PROPERTY(UserModel, users, Q_NULLPTR)
    QML_READONLY_VAR_PROPERTY(QString, ownerId, QString())
    QML_READONLY_VAR_PROPERTY(QString, ownerUsername, QString())
    QML_READONLY_VAR_PROPERTY(int, playersNo, 0)


public:
    LobbyModel(QObject* parent = 0);
    LobbyModel(Json::Value data, QObject *parent = 0);

    void parse(Json::Value data) override;

    void update(LobbyModel *newData);
};

#endif // LOBBYMODEL_H
