#include "ServiceProvider.h"

#include "RequestService.h"
#include "AuthService.h"

ServiceProvider::ServiceProvider()
{
    m_requestService = new RequestService();
    m_authService = new AuthService(m_requestService);
    m_generalService = new GeneralService(m_requestService);
}

RequestService *ServiceProvider::getRequestService()
{
    return m_requestService;
}

IAuthService *ServiceProvider::getAuthService()
{
    return m_authService;
}

GeneralService *ServiceProvider::getGeneralService()
{
    return m_generalService;
}
