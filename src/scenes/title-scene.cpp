#include "title-scene.h"
#include "scenes.h"
#include <string>


#include <cstdlib>
#include <iostream>
#include <string>

class Background : public Entity
{

public:
    Background(GfxSystem &s)
        : Entity(s)
    {

    }

    virtual void OnCreate(SDL_Renderer *renderer) override
    {
        Entity::OnCreate(renderer);

        mScreenWidth = GetSystem().GetWindowSize().w;
        mScreenHeight = GetSystem().GetWindowSize().h;

        Uint32 pixelFormat;
        int mTileWidth = 0;
        int mTileHeight = 0;

        SDL_Texture *tileTexture = Image::LoadRasterImage(renderer, "assets/hud/tile_bg1.png");


        // get the width and height of the texture
        if (SDL_QueryTexture(tileTexture, &pixelFormat, NULL, &mTileWidth, &mTileHeight) == 0)
        {
            SetSize(mTileWidth, mTileHeight);
        }

        mBigTexture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_TARGET, mScreenWidth, mScreenHeight);

        SDL_SetRenderTarget(renderer, mBigTexture);
        // Your draw loop here
        SDL_Rect r;
        r.w = mTileWidth;
        r.h = mTileHeight;
        r.x = 0;
        r.y = 0;
        // La tile est multiple de la résolution
        for (int i = 0; i < mScreenWidth / mTileWidth; i++)
        {
            r.y = 0;
            for (int j = 0; j < GetSystem().GetWindowSize().h / mTileHeight; j++)
            {
                SDL_RenderCopyEx(renderer, tileTexture, NULL, &r, 0.0, NULL, SDL_FLIP_NONE);
                r.y += mTileHeight;
            }
            r.x += mTileWidth;
        }

        SDL_SetRenderTarget(renderer, NULL);
        SDL_DestroyTexture(tileTexture);

    }

    virtual void Update(double deltaTime) override
    {
        move -= 0.03 * deltaTime;

        if (move <= mScreenWidth * -1) {
            move = 0;
        }
    }

    virtual void Draw(SDL_Renderer *renderer) override
    {
        SDL_Rect r;

        r.w = mScreenWidth;
        r.h = mScreenHeight;
        r.x = move;
        r.y = 0;
        SDL_RenderCopyEx(renderer, mBigTexture, NULL, &r, 0.0, NULL, SDL_FLIP_NONE);
        r.x += mScreenWidth;
        SDL_RenderCopyEx(renderer, mBigTexture, NULL, &r, 0.0, NULL, SDL_FLIP_NONE);
    }
private:
    double move = 0.0;
    SDL_Texture* mBigTexture = nullptr;

    int mScreenWidth = 0;
    int mScreenHeight = 0;
};


class Logo : public Image
{

public:
    Logo(GfxSystem &s)
        : Image(s, "assets/hud/logo.png")
    {
        mYPos = GetSystem().GetWindowSize().h / 3;
        SetPos(200, mYPos);
        SetScale(0.5, 0.5);
    }

    virtual void Draw(SDL_Renderer *renderer) override
    {
      //  DrawEx(renderer, GetX(), mYPos + 6*SDL_sin(SDL_GetTicks()*(3.14/1500)));
    }
private:
    int mYPos;
};


TitleScene::TitleScene(GfxSystem &system, IApplication &app, const std::string &version)
    : Scene(system)
    , mApp(app)
    , mVersion(version)
{
    auto bg = std::make_shared<Background>(GetSystem());
    mLogo = std::make_shared<Logo>(GetSystem());

    mLogo->SetVisible(false);

    AddEntity(bg);
    AddEntity(mLogo);

//    mHttpThread = std::thread(&TitleScene::RunHttp, this);
}

TitleScene::~TitleScene()
{
//    mHttpQueue.Push(HttpClient::Request(true));
//    if (mHttpThread.joinable())
//    {
//        mHttpThread.join();
//    }
}

void TitleScene::OnCreate(SDL_Renderer *renderer)
{
    Scene::OnCreate(renderer);

    gMusic = Mix_LoadMUS( "assets/audio/Parisian.mp3" );
    if( gMusic == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );

    }
}

void TitleScene::OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args)
{
    Scene::OnActivate(renderer);

    Mix_PlayMusic( gMusic, -1 );
}

void TitleScene::Draw(SDL_Renderer *renderer)
{
    Scene::Draw(renderer);
    if (mMenu == MENU_MAIN)
    {
        DrawMainMenu();
        DrawInfoMenu();
    }

}

void TitleScene::DrawMainMenu()
{
    // get the window size as a base for calculating widgets geometry
    int controls_width = 0;

    // position the controls widget in the top-right corner with some margin
    ImGui::SetNextWindowPos(ImVec2(45, 45), ImGuiCond_Always);
    // here we set the calculated width and also make the height to be
    // be the height of the main window also with some margin
    ImGui::SetNextWindowSize(
                ImVec2(static_cast<float>(740), static_cast<float>(340)),
                ImGuiCond_Always
                );
    // create a window and append into it
    ImGui::Begin("MainMenu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

    ImGui::Image((void*) mLogo->GetTexture(), ImVec2(mLogo->GetWidth(), mLogo->GetHeight()));

    ImGui::Dummy(ImVec2(0.0f, 50.0f));

    if (ImGui::Button("Jouer", ImVec2(80, 40)))
    {
        SwitchToScene(SCENE_STORY_MODE);
    }

    if (ImGui::Button("Quitter", ImVec2(80, 40)))
    {
        SwitchToScene(SCENE_EXIT);
    }

    ImGui::End();
}




void TitleScene::DrawInfoMenu()
{
    Rect rect = GetSystem().GetWindowSize();
    const int windowHeight = 30;

    ImGui::SetNextWindowPos(ImVec2(0, rect.h - windowHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(100, windowHeight), ImGuiCond_Always);

    ImGui::Begin("InfoMenu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground  | ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("%s", mVersion.c_str());

    ImGui::End();
}
