#ifndef APPLICATION_H
#define APPLICATION_H

#include <list>
#include <deque>
#include <string>


#include "Log.h"
#include "i-application.h"

// Gfx
#include "title-scene.h"


#define TAROTCLUB_APP_VERSION   "3.0.0"

class Application : public Observer<Log::Infos>
        , public IApplication
{
public:
    Application();
    ~Application();

    bool Initialize();
    int Loop();
    void Stop();

    // From IApplication
    bool IsLogged() override { return mLogged; }
    bool IsInternetDetected() override;

    std::string GetHost() const override;
    virtual void ConnectToServer(const std::string &serverId) override;


private:
    enum GameType {
        GAME_TYPE_LOCAL,
        GAME_TYPE_HOSTED,
        GAME_TYPE_ONLINE,
    };

    // From Observer
    virtual void Update(const Log::Infos &info) override;

    // From IBoardEvent (board events ---> application ---> game server

    GameType mGameType = GAME_TYPE_LOCAL;

    bool mLogged = false;

    GfxEngine mGfx;

    std::mutex mMutex;

    void TimeoutEndOfTrickDelay();
    void ApplyOptions();
};

#endif // APPLICATION_H
