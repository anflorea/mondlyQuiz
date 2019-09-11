#ifndef RESPONSE_H
#define RESPONSE_H

#include <QString>
#include <QHash>
#include "json/json.h"

class Response
{
public:
    QString id = QString();
    int statusCode = -1;
    qint64 duration = 0; // ms
    int size = 0; // bytes
    QString reasonPhrase = QString();
    QHash<QString, QString> headers;
    QHash<QString, QString> requestHeaders; // temporary workaround for registering native headers
    QByteArray rawResponse = QByteArray();
    Json::Value responseJson = Json::nullValue;

    Json::Value bodyAsJson()
    {
        return responseJson;
    }
};

#endif // RESPONSE_H
