#include "LobbyModel.h"

LobbyModel::LobbyModel(QObject *parent)
    : BaseModel (parent)
{
    m_users = new QQmlObjectListModel<UserModel>(this);
}

LobbyModel::LobbyModel(Json::Value data, QObject *parent)
    : LobbyModel (parent)
{
    parse(data);
}

void LobbyModel::parse(Json::Value data)
{
    if (data.isString()) {
        Json::Reader reader;
        std::string old = data.asString();
        reader.parse(old, data);
    }
    Json::Value lobby = data.get("lobby", Json::Value());

    update_ref_id(QString::fromStdString(lobby.get("id", "").asString()));
    update_ownerId(QString::fromStdString(lobby.get("ownerID", "").asString()));

    update_ownerUsername(QString::fromStdString(data.get("ownerName", "").asString()));
    update_playersNo(data.get("playersCount", 0).asInt());

    Json::Value users = data.get("players", Json::arrayValue);

    if (users.isArray()) {
        for (Json::Value user : users) {
            UserModel *newUser = new UserModel(user);
            m_users->append(newUser);
        }
    }
}

void LobbyModel::update(LobbyModel *newData)
{
    update_ownerId(newData->get_ownerId());
    update_ownerUsername(newData->get_ownerUsername());
    update_playersNo(newData->get_playersNo());

    m_users->clear();
    for (UserModel *user : newData->get_users()->toList()) {
        m_users->append(user);
    }
}
