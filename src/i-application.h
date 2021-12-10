#ifndef I_APPLICATION_H
#define I_APPLICATION_H

#include "Identity.h"

class IApplication
{
public:
    virtual ~IApplication() {}

    virtual bool IsLogged() = 0;
    virtual void SetLogged(const Identity &ident) = 0;
};


#endif // I_APPLICATION_H
