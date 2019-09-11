#ifndef SERVICEPROVIDER_H
#define SERVICEPROVIDER_H

#include "IServiceProvider.h"
#include "IAuthService.h"
#include "GeneralService.h"

class RequestService;

class ServiceProvider : public IServiceProvider
{
public:
    ServiceProvider();

    RequestService *getRequestService() override;
    IAuthService *getAuthService() override;
    GeneralService *getGeneralService() override;

private:
    RequestService *m_requestService = nullptr;
    IAuthService *m_authService = nullptr;
    GeneralService *m_generalService = nullptr;
};

#endif // SERVICEPROVIDER_H
