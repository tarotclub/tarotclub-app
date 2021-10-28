#include "title-scene.h"
#include "scenes.h"

class Background : public Image
{

public:
    Background(GfxSystem &s)
        : Image(s, "assets/hud/background.png")
    {

    }

    virtual void Update(double deltaTime) override
    {
//        SetPos(GetPos().x += 0.1*deltaTime, GetPos().y);
        move -= 0.05*deltaTime;

        if (move <= GetRect().w * -2) {
            move = 0;
        }
    }

    virtual void Draw(SDL_Renderer *renderer) override
    {
        DrawEx(renderer, move, 0);
        DrawEx(renderer, move + GetRect().w, 0);
        DrawEx(renderer, move + GetRect().w * 2, 0);
        DrawEx(renderer, move + GetRect().w * 3, 0);
    }
private:
    double move;

};


class Logo : public Image
{

public:
    Logo(GfxSystem &s)
        : Image(s, "assets/hud/logo.png")
    {
        mYPos = GetSystem().GetWindowSize().h / 3;
        SetPos(50, mYPos);
        SetScale(0.5, 0.5);
    }

    virtual void Draw(SDL_Renderer *renderer) override
    {
        DrawEx(renderer, GetX(), mYPos + 6*SDL_sin(SDL_GetTicks()*(3.14/1500)));
    }
private:
    int mYPos;
};


TitleScene::TitleScene(GfxSystem &system)
    : Scene(system)
{
    auto bg = std::make_shared<Background>(GetSystem());
    auto logo = std::make_shared<Logo>(GetSystem());

    AddEntity(bg);
    AddEntity(logo);
}

void TitleScene::OnCreate(SDL_Renderer *renderer)
{
    Scene::OnCreate(renderer);
}

void TitleScene::OnActivate(SDL_Renderer *renderer)
{
    Scene::OnActivate(renderer);
}

void TitleScene::Draw(SDL_Renderer *renderer)
{
    Scene::Draw(renderer);
    DrawGui();
}

void TitleScene::DrawGui()
{
    // get the window size as a base for calculating widgets geometry
    int controls_width = 0;

    Rect rect = GetSystem().GetWindowSize();

    controls_width = rect.w;
    // make controls widget width to be 1/3 of the main window width
    if ((controls_width /= 3) < 300)
    {
        controls_width = 300;
    }

    // position the controls widget in the top-right corner with some margin
    ImGui::SetNextWindowPos(ImVec2(400, (rect.h /3)), ImGuiCond_Always);
    // here we set the calculated width and also make the height to be
    // be the height of the main window also with some margin
    ImGui::SetNextWindowSize(
                ImVec2(static_cast<float>(controls_width), static_cast<float>(rect.h / 3)),
                ImGuiCond_Always
                );
    // create a window and append into it
    ImGui::Begin("MainMenu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

    if (ImGui::Button("Start local game"))
    {
        SwitchToScene(SCENE_LOCAL_GAME);
    }

    if (ImGui::Button("Quit"))
    {
        SwitchToScene(SCENE_EXIT);
    }
    ImGui::End();
}
