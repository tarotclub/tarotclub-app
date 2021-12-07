#ifndef TITLESCENE_H
#define TITLESCENE_H

#include "gfx-engine.h"
#include "i-application.h"
#include <iostream>
#include "asio.hpp"
#include "asio/ssl.hpp"
#include <thread>

class TitleScene : public Scene
{
public:
    TitleScene(GfxSystem &system, IApplication &app, const std::string &version);
    ~TitleScene();

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void OnActivate(SDL_Renderer *renderer) override;

    virtual void Draw(SDL_Renderer *renderer) override;

private:
    enum Menus {
        MENU_MAIN,
        MENU_ONLINE,
    };
    IApplication &mApp;

    ImFont* mMenuFont;
    std::string mVersion;

    Menus mMenu = MENU_MAIN;

    asio::io_context io_context;
    asio::ip::tcp::resolver resolver;
    asio::ip::tcp::socket socket;
    asio::ssl::context ssl_ctx;
    std::thread mHttpThread;

    void DrawInfoMenu();
    void DrawMainMenu();
    void DrawOnlineMenu();
    void ConnectToWebsite();
};


#endif // TITLESCENE_H
