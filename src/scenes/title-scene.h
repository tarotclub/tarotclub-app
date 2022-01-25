#ifndef TITLESCENE_H
#define TITLESCENE_H

#include "gfx-engine.h"
#include "i-application.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "http-client.h"
#include "websocket-client.h"
#include "ThreadQueue.h"
#include "Network.h"

#include <JsonValue.h>

#include <thread>
#include <iostream>

class TitleScene : public Scene, public WebSocketClient::IReadHandler
{
public:
    TitleScene(GfxSystem &system, IApplication &app, const std::string &version);
    ~TitleScene();

    virtual void OnCreate(SDL_Renderer *renderer) override;
    virtual void OnActivate(SDL_Renderer *renderer) override;
    virtual void Draw(SDL_Renderer *renderer) override;

private:
    enum tribool: uint8_t {False = 0, True = 1, Unknown = 2};

    enum Menus {
        MENU_MAIN,
        MENU_ONLINE,
    };

    IApplication &mApp;

    tribool mConnectState = tribool::Unknown;

    ImFont* mMenuFont;
    std::string mVersion;

    std::shared_ptr<Image> mLogo;

    Menus mMenu = MENU_MAIN;

    HttpClient mHttpClient;
    ThreadQueue<HttpClient::Request> mHttpQueue;
    std::thread mHttpThread;

    WebSocketClient mWsClient;
    std::thread mWsThread;

    std::mutex mMutex;
    std::vector<ServerState> mServers;

    // From WebSocketClient::IReadHandler
    virtual void OnWsData(const std::string &data) override;

    void DrawInfoMenu();
    void DrawMainMenu();
    void DrawOnlineMenu();

    void RunHttp();
    void RunWebSocket();
    void Login(const std::string &login, const std::string &password);
    void HandleHttpReply();
};


#endif // TITLESCENE_H
