#ifndef IAUTHSERVICE_H
#define IAUTHSERVICE_H

#include "json/json.h"
#include <QObject>
#include <QQuickItem>

class QString;
class Response;

class IAuthService : public QObject
{
public:
    virtual void signIn(const QString& username,
                        const QString& password,
                        const std::function<void(QString, QString)>& success = nullptr,
                        const std::function<void(Response*)>& fail = nullptr) = 0;

    virtual void signOut(QString accessToken,
                         const std::function<void(void)>& success = nullptr,
                         const std::function<void(Json::Value)>& fail = nullptr) = 0;

    virtual void connectToEventStream(const std::function<void(void)>& success = nullptr,
                                      const std::function<void(Json::Value)>& gotEvent = nullptr,
                                      const std::function<void()>& fail = nullptr) = 0;
};


#endif // IAUTHSERVICE_H
