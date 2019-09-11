#ifndef ISERVICEPROVIDER_H
#define ISERVICEPROVIDER_H

class RequestService;
class IAuthService;
class GeneralService;

class IServiceProvider
{
public:
    virtual RequestService *getRequestService() = 0;
    virtual IAuthService *getAuthService() = 0;
    virtual GeneralService *getGeneralService() = 0;

};

#endif // ISERVICEPROVIDER_H
