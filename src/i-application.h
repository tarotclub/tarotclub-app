#ifndef I_APPLICATION_H
#define I_APPLICATION_H

#include <string>
class IApplication
{
public:
    virtual ~IApplication() {}

    virtual bool IsLogged() = 0;
    virtual bool IsInternetDetected() = 0;
    virtual std::string GetHost() const = 0;
    virtual void ConnectToServer(const std::string &serverId) = 0;
};


#endif // I_APPLICATION_H
