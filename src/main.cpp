

#include <iostream>

// Tarot
#include "Log.h"
#include "Lobby.h"
#include "Server.h"
#include "TournamentConfig.h"
#include "System.h"
#include "GetOptions.h"
#include "Embedded.h"
#include "application.h"

static const std::string localGameKey = "1234567890abcdef";
static const std::string localPassPhrase = "lacaravanepasse";
static const std::string localWebId1 = "southId";

// Classe passe-plat entre divers objets
class ClientProxy : private INetClient
{
public:
    ClientProxy()
        : mApp(*this)
        , mSession(mApp)
    {
        Log::RegisterListener(mApp);
    }

    ~ClientProxy()
    {
        TLogInfo("[CLIENT_PROXY] Exit");
    }

    virtual void Initialize(const std::string &webId, const std::string &key, const std::string &passPhrase) override
    {
        mSession.Initialize(webId, key, passPhrase);
    }

    void Start()
    {
        mSession.Initialize(localWebId1, localGameKey, localPassPhrase);
        // Initialize SDL & ImGui
        if (!mApp.Initialize())
        {
            std::cout << "Initialization failure" << std::endl;
        }
        else
        {
            mApp.Loop();
        }

        mSession.Disconnect();
        mSession.Close();
    }

private:
    virtual void Send(uint32_t my_uid, const std::vector<Reply> &replies) override {
        mSession.Send(my_uid, replies);
    }

    virtual void ConnectToHost(const std::string &host, uint16_t tcp_port) override {
        mSession.ConnectToHost(host, tcp_port);
    }

    virtual void Disconnect() override {
        mSession.Disconnect();
    }

    Application mApp;
    Session mSession;
};

static const Identity ident[3] = {
    Identity("Bender", "", Identity::cGenderRobot),
    Identity("T800", "", Identity::cGenderRobot),
    Identity("C3PO", "", Identity::cGenderRobot)
};

static std::list<std::uint32_t> mIds;
static BotManager mBots;
static ThreadQueue<bool> gServerStarted;
static asio::io_context io_context;

void task_server()
{
    try
    {
        asio::executor_work_guard<decltype(io_context.get_executor())> work{io_context.get_executor()};
        ServerOptions options = ServerConfig::GetDefault();
        options.localHostOnly = true;

        Server s(io_context, options);

        s.AddClient(localWebId1, localGameKey, localPassPhrase);

        for (std::uint32_t i = 0U; i < (sizeof(ident) / sizeof(ident[0])); i++)
        {
            s.AddClient(ident[i].username, localGameKey, localPassPhrase);
        }

        gServerStarted.Push(true);

        TLogInfo("[SERVER] Thread started");

     //   io_context.run();

        TLogError("[SERVER] Thread halted");
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

extern "C" int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    std::string homePath = System::HomePath();
    System::Initialize(); // default home path
    Log::SetLogPath(System::LogPath());
    TLogInfo("Using home path: " + homePath);

    // Application client, initialisée très tôt pour avoir les logs redirigés au plus tôt
    ClientProxy client;

    // On démarre le serveur local
    std::thread thread_server(task_server);

    // Wait for server start
    bool started = false;
    gServerStarted.WaitAndPop(started);

    ArrayPtr<const std::uint8_t> array = gen::GetFile();
    std::string buffer((const char *)array.Data(), array.Size());
    for (std::uint32_t i = 0U; i < (sizeof(ident) / sizeof(ident[0])); i++)
    {
        std::uint32_t botId = mBots.AddBot(Protocol::TABLES_UID, ident[i], 0U, buffer);
        mIds.push_back(botId);
        mBots.Initialize(botId, ident[i].username, localGameKey, localPassPhrase);
        mBots.ConnectBot(botId, "127.0.0.1", 4269U);
    }

    client.Start();

    mBots.Close();
    mBots.KillBots();

    io_context.stop();
    if (thread_server.joinable())
    {
        thread_server.join();
    }
    return 0;
}
