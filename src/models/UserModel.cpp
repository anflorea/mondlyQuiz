#include "UserModel.h"

UserModel::UserModel(QObject *parent)
    : BaseModel (parent)
{

}

UserModel::UserModel(Json::Value data, QObject *parent)
    : UserModel (parent)
{
    parse(data);
}

void UserModel::parse(Json::Value data)
{
    update_username(QString::fromStdString(data.get("username", "").asString()));
    update_points(data.get("points", 0).asInt());
}
