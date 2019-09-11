#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QUuid>

class Utils
{
public:
    static inline QString randomUUID() {
        QString uuid = QUuid::createUuid().toString();
        uuid = uuid.replace("{","");
        uuid = uuid.replace("}","");

        return uuid;
    }
};

#endif // UTILS_H
