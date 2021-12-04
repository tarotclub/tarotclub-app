#ifndef I_APPLICATION_H
#define I_APPLICATION_H

class IApplication
{
public:
    virtual ~IApplication() {}

    virtual bool IsLogged() = 0;
};


#endif // I_APPLICATION_H
