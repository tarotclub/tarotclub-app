#include "ai-editor-scene.h"
#include "SDL2_gfxPrimitives.h"
#include "assets.h"
#include "IconsMaterialDesign.h"
#include "IconsFontAwesome5.h"
#include "imgui_internal.h"

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
    mView.OnStop();
}


void AiEditorScene::OnCreate(SDL_Renderer *renderer)
{
    Scene::OnCreate(renderer);

    mView.OnStart();
}

void AiEditorScene::OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args)
{
    Scene::OnActivate(renderer);
}

void AiEditorScene::Update(double deltaTime)
{
//    mCar->SetPos(200, 200);


}

void AiEditorScene::Draw(SDL_Renderer *renderer)
{
    static bool resetDockspace = true;

    float menuHeight = 0;

    if(ImGui::BeginMainMenuBar())
    {
        menuHeight = ImGui::GetWindowSize().y;

      if (ImGui::BeginMenu("Actions"))
      {
         if(ImGui::MenuItem("Quit"))
         {
             mEvent.ExitGame();
         }
         ImGui::EndMenu();
       }

       ImGui::EndMainMenuBar();
    }


    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;



    ImGui::DockSpaceOverViewport();
    if (resetDockspace)
    {
        resetDockspace = false;

        ImGuiID id = ImGui::GetID("Dock Main");


        ImGui::DockBuilderRemoveNode(id);
        ImGui::DockBuilderAddNode(id, ImGuiDockNodeFlags_CentralNode);//ImGuiDockNodeFlags_CentralNode | ImGuiDockNodeFlags_NoResize);

        ImGui::DockBuilderSetNodePos(id, { 0, menuHeight });
        ImGui::DockBuilderSetNodeSize(id, { (float)GetSystem().GetWindowSize().w, (float)GetSystem().GetWindowSize().h - menuHeight});

//        ImGuiID dockMainID = id;
//        const ImGuiID dockLeft = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Left, 1.0, nullptr, nullptr);

        ImGui::DockBuilderDockWindow("EditorView", id);


        ImGui::DockBuilderFinish(id);
    }

    if (ImGui::Begin("EditorView", NULL, window_flags))
    {
        mView.OnFrame();
    }
    ImGui::End();



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


