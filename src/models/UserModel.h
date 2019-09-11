#ifndef USERMODEL_H
#define USERMODEL_H

#include "BaseModel.h"

class UserModel : public BaseModel
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(QString, username, QString())
    QML_READONLY_VAR_PROPERTY(int, points, 0)

public:
    UserModel(QObject* parent = 0);
    UserModel(Json::Value data, QObject *parent = 0);

    void parse(Json::Value data) override;
};

#endif // USERMODEL_H
