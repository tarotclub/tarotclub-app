#ifndef I_APPLICATION_H
#define I_APPLICATION_H

#include "Identity.h"
#include "Network.h"

class IApplication
{
public:
    virtual ~IApplication() {}

    virtual bool IsLogged() = 0;
    virtual bool IsInternetDetected() = 0;
    virtual void SetLogged(const Identity &ident) = 0;
    virtual std::string GetHost() const = 0;
    virtual void ConnectToServer(const std::string &serverId) = 0;
    virtual std::vector<ServerState> GetServers() = 0;
};


#endif // I_APPLICATION_H
