#ifndef INETWORKMANAGER_H
#define INETWORKMANAGER_H

#include <functional>
class Request;
class Response;
class QString;

class INetworkManager
{
public:
    virtual void sendRequest(Request* request) = 0;
    virtual void cancelRequest(QString requestId) = 0; // or maybe send full request?

protected:
    std::function<void(Response*)> m_readyReadCallback; // rename
    std::function<void(Response*)> m_finishedCallback; // rename
    std::function<void(Response*)> m_errorCallback;
    std::function<void(QString, double)> m_uploadProgressCallback;
};

#endif // INETWORKMANAGER_H
