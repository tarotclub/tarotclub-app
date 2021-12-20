#include "ai-editor-scene.h"
#include "SDL2_gfxPrimitives.h"
#include "assets.h"
#include "IconsMaterialDesign.h"
#include "IconsFontAwesome5.h"


AiEditorScene::AiEditorScene(GfxSystem &system, IBoardEvent &event)
    : Scene(system)
    , mEvent(event)
{
//    mMap = std::make_shared<FranceMap>(GetSystem());
//    mCar = std::make_shared<Car>(GetSystem());

////    AddEntity(mMap);
    //    AddEntity(mCar);
}

AiEditorScene::~AiEditorScene()
{
    ed::DestroyEditor(m_Context);
}


void AiEditorScene::OnCreate(SDL_Renderer *renderer)
{
    Scene::OnCreate(renderer);

    ed::Config config;
    config.SettingsFile = "Simple.json";
    m_Context = ed::CreateEditor(&config);
}

void AiEditorScene::OnActivate(SDL_Renderer *renderer)
{
    Scene::OnActivate(renderer);
}

void AiEditorScene::Update(double deltaTime)
{
//    mCar->SetPos(200, 200);
}

void AiEditorScene::Draw(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer,  0x09, 0x72, 0x00, 255);
    // Clear the entire screen to our selected color.
    SDL_RenderClear(renderer);

    Scene::Draw(renderer);

    ToolbarUI();


    auto& io = ImGui::GetIO();

    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

    ImGui::Separator();

    ed::SetCurrentEditor(m_Context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));
    int uniqueId = 1;
    // Start drawing nodes.
    ed::BeginNode(uniqueId++);
        ImGui::Text("Node A");
        ed::BeginPin(uniqueId++, ed::PinKind::Input);
            ImGui::Text("-> In");
        ed::EndPin();
        ImGui::SameLine();
        ed::BeginPin(uniqueId++, ed::PinKind::Output);
            ImGui::Text("Out ->");
        ed::EndPin();
    ed::EndNode();
    ed::End();
    ed::SetCurrentEditor(nullptr);


//  //  filledCircleRGBA(renderer, city.x , city.y, 6, 255, 0, 0, 255);

//    vlineRGBA(renderer, city.x, 0, mMap->GetHZoomed(), 255, 0, 0, 255 );
//    hlineRGBA(renderer, 0, mMap->GetWZoomed(), city.y, 255, 0, 0, 255 );

//    vlineRGBA(renderer, mMap->GetWZoomed(), 0, mMap->GetHZoomed(), 255, 0, 0, 255 );
}

void AiEditorScene::ProcessEvent(const SDL_Event &event)
{
    Scene::ProcessEvent(event);
}


void AiEditorScene::ToolbarUI()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(42, GetSystem().GetWindowSize().h));

    ImGuiWindowFlags window_flags = 0
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoSavedSettings
        ;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin("TOOLBAR", NULL, window_flags);
    ImGui::PopStyleVar();

    if (ImGui::Button(ICON_FA_COG))
    {
        ImGui::OpenPopup("Options");
    }

    if (ImGui::Button(ICON_FA_SIGN_OUT_ALT))
    {
        mEvent.ExitGame();
    }


    ImGui::End();
}


