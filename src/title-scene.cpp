#include "title-scene.h"
#include "scenes.h"
#include "http-client.h"

class Background : public Entity
{

public:
    Background(GfxSystem &s)
        : Entity(s)
    {

    }

    virtual void Initialize(SDL_Renderer *renderer) override
    {
        Entity::Initialize(renderer);

        mScreenWidth = GetSystem().GetWindowSize().w;
        mScreenHeight = GetSystem().GetWindowSize().h;

        Uint32 pixelFormat;
        int mTileWidth = 0;
        int mTileHeight = 0;

        SDL_Texture *tileTexture = GfxEngine::LoadImage(renderer, "assets/hud/tile_bg1.png");


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


TitleScene::TitleScene(GfxSystem &system, IApplication &app, const std::string &version)
    : Scene(system)
    , mApp(app)
    , mVersion(version)
    , resolver(io_context)
    , socket(io_context)
    , ssl_ctx(asio::ssl::context::tls)
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

    if (mMenu == MENU_MAIN)
    {
        DrawMainMenu();
    }
    else if (mMenu == MENU_ONLINE)
    {
        DrawOnlineMenu();
    }
    DrawInfoMenu();
}

void TitleScene::DrawMainMenu()
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

    if (ImGui::Button("Online game"))
    {
        mMenu = MENU_ONLINE;
    }

    if (ImGui::Button("Quit"))
    {
        SwitchToScene(SCENE_EXIT);
    }
    ImGui::End();
}


void TitleScene::ConnectToWebsite()
{
#ifdef TAROT_DEBUG
    std::string hostName = "127.0.0.1";
#else
    std::string hostName = "tarotclub.fr";
#endif
    hostName = "tarotclub.fr";

    io_context.restart();

    // give it some work, to prevent premature exit
    asio::executor_work_guard<decltype(io_context.get_executor())> work{io_context.get_executor()};
//    mHttpThread = std::thread([&] {
        try
        {
//            tcp::resolver::query query(tcp::v4(), hostName,  "", asio::ip::resolver_query_base::numeric_service);

            asio::ip::tcp::resolver::query query(hostName, "443");
            asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

//            auto endpoints = resolver.resolve(query);

//            for (auto &ep : endpoints)
//            {
//                std::cout << "EP: " << ep.endpoint() << std::endl;
//            }
            HttpClient c(io_context, ssl_ctx, iterator);
            io_context.run();

        //    mHttpThread = std::thread(&asio::io_context::run, &io_context);



            std::cout << "EXIT"<<std::endl;
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << "\n";
        }
//    });



    // tcp::resolver::results_type& endpoints
/*
    asio::async_connect(socket, endpoints,
        [this](std::error_code error, asio::ip::tcp::endpoint)
        {
            if ((asio::error::eof == error) || (asio::error::connection_reset == error))
            {
                Disconnect();
            }
            else if (!error)
            {
                TLogInfo("Client " + mWebId + " connected");
                SendToHost(BuildConnectionPacket());
                ReadHeader();
            }
        }
    );

    while(!mQuitThread)
    {
        if (!connected)
        {
            client.Initialize();
            if (client.Connect(mHost, 8989))
            {
                connected = true;
            }
            else
            {
                TLogError("[WEBSITE] Cannot join server");
            }
        }
        else
        {
//            TLogInfo("[WEBSITE] Sending register or status request");
            std::string response;
            if (client.RecvWithTimeout(response, 2048, 500))
            {
                JsonReader reader;
                JsonValue json;

                if (reader.ParseString(json, response))
                {
                    if (json.IsObject())
                    {
                        JsonObject rcvObj = json.GetObj();

                        if (rcvObj.HasValue("cmd"))
                        {
                            JsonObject replyObj;
                            if (HandleCommand(rcvObj.GetValue("cmd").GetString(), rcvObj, replyObj))
                            {
                                client.Send(replyObj.ToString(0));
                            }
                        }
                        else
                        {
                            TLogError("[WEBSITE] Invalid reply. Error: " + rcvObj.GetValue("message").GetString());
                        }
                    }
                    else
                    {
                        TLogError("[WEBSITE] Reply body is not an object");
                    }
                }
                else
                {
                    TLogError("[WEBSITE] Reply body is not JSON: " + response);
                }
            }
            else
            {
                bool req_success = client.Send(UpdateServerStatus());

                if (!req_success)
                {
                    client.Close();
                    connected = false;
                    TLogError("[WEBSITE] Sending request failure");
                }
            }


        } // if connected

        std::this_thread::sleep_for(std::chrono::seconds(1));
        */
}

void TitleScene::DrawOnlineMenu()
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
    ImGui::Begin("OnlineMenu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    if (! mApp.IsLogged())
    {
        ImGui::Text("Not connected! You cannot join any game server");
        static char password[64] = "";
        static char buf1[64] = "";
        ImGui::InputText("Login",     buf1, 64);
        ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

        if (ImGui::Button("Connect"))
        {
            ConnectToWebsite();
        }

    }
    else
    {
        if (ImGui::BeginTable("table1", 3, flags))
        {
            // Display headers so we can inspect their interaction with borders.
            // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for details)
                ImGui::TableSetupColumn("One");
                ImGui::TableSetupColumn("Two");
                ImGui::TableSetupColumn("Three");
                ImGui::TableHeadersRow();


            for (int row = 0; row < 5; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    char buf[32];
                    sprintf(buf, "Hello %d,%d", column, row);
                    ImGui::TextUnformatted(buf);

                }
            }
            ImGui::EndTable();
        }
    }

    if (ImGui::Button("Return to main menu"))
    {
        mMenu = MENU_MAIN;
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
