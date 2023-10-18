#ifndef TITLESCENE_H
#define TITLESCENE_H

#include "gfx-engine.h"
#include "i-application.h"

#include "SDL2/SDL_mixer.h"
#include <thread>
#include <iostream>

class TitleScene : public Scene
{
public:
    TitleScene(GfxSystem &system, IApplication &app, const std::string &version);
    ~TitleScene();

    virtual void OnCreate(SDL_Renderer *renderer) override;
    virtual void OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args) override;
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

    Mix_Music* gMusic = NULL;

    Menus mMenu = MENU_MAIN;

//    HttpClient mHttpClient;
//    ThreadQueue<HttpClient::Request> mHttpQueue;
//    std::thread mHttpThread;

    void DrawInfoMenu();
    void DrawMainMenu();

};


#endif // TITLESCENE_H
