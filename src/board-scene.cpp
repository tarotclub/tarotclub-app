#include "board-scene.h"
#include "Deck.h"
#include "scenes.h"

#include "IconsMaterialDesign.h"
#include "IconsFontAwesome5.h"

/**
 * interpolate 2 RGB colors
 * @param color1    integer containing color as 0x00RRGGBB
 * @param color2    integer containing color as 0x00RRGGBB
 * @param fraction  how much interpolation (0..1)
 * - 0: full color 1
 * - 1: full color 2
 * @return the new color after interpolation
 */
SDL_Color interpolate(SDL_Color color1, SDL_Color color2, float fraction)
{
    SDL_Color c;
    c.a = 255;

    c.r = (int) ((color2.r - color1.r) * fraction + color1.r);
    c.g = (int) ((color2.g - color1.g) * fraction + color1.g);
    c.b = (int) ((color2.b - color1.b) * fraction + color1.b);

    return c;
}

BoardScene::BoardScene(GfxSystem &system, IBoardEvent &event, PlayerContext &ctx, uint32_t scene)
    : Scene(system)
    , mEvent(event)
    , mCtx(ctx)
    , mSceneType(scene)
    , mHud(ctx, mEvent)
{
    Deck deck;
    deck.CreateTarotDeck();

    for (const auto &c : deck)
    {
        auto card = std::make_shared<GfxCard>(GetSystem(), "assets/cards/default/" + c.ToString() + ".png", c.ToString());
        card->SetVisible(false);
       // card->SetScale(0.75, 0.75);
        AddEntity(card);
        mFullDeck[c.ToString()] = card;
    }
}

float euclidean_distance(SDL_Point center, SDL_Point point){
    float distance = std::sqrt(
        std::pow(center.x - point.x, 2) + std::pow(center.y - point.y, 2));
    return distance;
}


void BoardScene::OnCreate(SDL_Renderer *renderer)
{
    Scene::OnCreate(renderer);

    Uint32 pixelFormat = SDL_PIXELFORMAT_RGBA32;
    mBg = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_TARGET, GetSystem().GetWindowSize().w, GetSystem().GetWindowSize().h);

    SDL_PixelFormat * fmt = SDL_AllocFormat(pixelFormat);

    SDL_Point center;

    center.x = GetSystem().GetWindowSize().w / 2;
    center.y = GetSystem().GetWindowSize().h / 2;

    SDL_SetRenderTarget(renderer, mBg);
    SDL_Point point;
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    SDL_Color fromColor;
    fromColor.r = 0x09;
    fromColor.g = 0x72;
    fromColor.b = 0x00;
    fromColor.a = 255;

    SDL_Color toColor;
    toColor.r = 0x05;
    toColor.g = 0x2f;
    toColor.b = 0x01;
    toColor.a = 255;

    for(int row=0; row<GetSystem().GetWindowSize().h; row++){
        for(int col=0; col<GetSystem().GetWindowSize().w; col++){
            point.x = col;
            point.y = row;

            float d = euclidean_distance(center, point);

          //  float a = (d - center.y) / (d - GetSystem().GetWindowSize().h);

            float a = d * (50.0 / GetSystem().GetWindowSize().w + 0.1) * 0.01;

            SDL_Color c = interpolate(fromColor, toColor, a);

            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
            SDL_RenderDrawPoint(renderer, col, row);
        }
    }

    SDL_SetRenderTarget(renderer, NULL);

    SDL_FreeFormat(fmt);
}

void BoardScene::OnActivate(SDL_Renderer *renderer)
{
    Scene::OnActivate(renderer);
}

static const SDL_Point gCardPositions[5] = {
  { 400, 210 }, // south
  { 600, 120 }, // east
  { 400, 20 } , // north
  { 180, 120}, // west
  { 0, 0} // fifth
};

static const SDL_Point gBoxPositions[5] = {
  { 20, 400 }, // south
  { 700, 200 }, // east
  { 250, 50 } , // north
  { 20, 200}, // west
    {0, 0} // fifth
};


void BoardScene::Update(double deltaTime)
{
    Scene::Update(deltaTime);
    ShowSouthCards();

    if ((mCtx.mMode == PlayerContext::TABLE_MODE_PLAY) ||
        (mCtx.mMode == PlayerContext::TABLE_MODE_WAIT_TIMER_END_OF_TRICK) ||
        (mCtx.mMode == PlayerContext::TABLE_MODE_WAIT_CLICK_END_OF_TRICK))
    {
        // on affiche les cartes au milieu
        uint32_t z = 100;
        Place p = mCtx.mFirstPlayer;
        for (auto & c : mCtx.mCurrentTrick)
        {
            auto sprite = mFullDeck[c.ToString()];
            sprite->SetZ(z++);
            sprite->SetVisible(true);

            Place rel = mCtx.mMyself.place.Relative(p, mCtx.mGameState.mNbPlayers);
            sprite->SetPos(gCardPositions[rel.Value()].x, gCardPositions[rel.Value()].y);
            sprite->SetAngle(0);
            p = p.Next(mCtx.mGameState.mNbPlayers);
        }
    }
}


void BoardScene::ShowSouthCards()
{
    uint32_t i = 0;
    uint32_t offset = 0;
    mCtx.mDeck.Sort("SHDCT");
    uint32_t z = 100;
    uint32_t pos = 0;
    for (auto & c : mCtx.mDeck)
    {
        auto sprite = mFullDeck[c.ToString()];
        sprite->SetZ(z++);
        sprite->SetVisible(true);
        sprite->SetPos(150 + pos * 60, 400 + offset);

        // Si c'est mon tour, on autorise ou non la sélection des cartes
        if (mCtx.IsMyTurn())
        {
            sprite->SetSelectable(mCtx.IsValid(c));
        }
        else
        {
            sprite->SetSelectable(false);
        }

        i++;
        pos++;
        if (i == (mCtx.mDeck.Size()/2))
        {
            offset = 60;
            pos = 0;
        }
    }
    Sort();
}


// radial-gradient(#40845c, #026a2d); /* vert foncé: #026a2d; */
void BoardScene::Draw(SDL_Renderer *renderer)
{
    // Rendu du background
    SDL_Rect r;

    r.w = GetSystem().GetWindowSize().w;
    r.h = GetSystem().GetWindowSize().h;
    r.x = 0;
    r.y = 0;
    SDL_RenderCopyEx(renderer, mBg, NULL, &r, 0.0, NULL, SDL_FLIP_NONE);

    // Rendu des entités de la scène
    Scene::Draw(renderer);

    // HUD
    mHud.ToolbarUI(mEvent);

    if (mCtx.mMyself.IsInTable())
    {
        DrawCentralGame();

        if (mCtx.mMode == PlayerContext::TABLE_MODE_BID)
        {
            if (mCtx.IsMyBidTurn())
            {
                mHud.DrawBidChooser(mEvent);
            }
        }
        else if (mCtx.mMode == PlayerContext::TABLE_MODE_PLAY)
        {
            // FIXME: handle au premier tour

        }
        else if (mCtx.mMode == PlayerContext::TABLE_MODE_SHOW_RESULTS)
        {
            mHud.DrawResult(mEvent);
        }
    }

    mHud.DrawChatWindow(GetSystem(), mEvent);
}

void BoardScene::ProcessEvent(const SDL_Event &event)
{
    // Re-init sprites
    for (auto &s : mFullDeck)
    {
        s.second->SetVisible(false);
//        s.second->SetSelectable(false);
//        s.second->SetHovered(false);
    }

    // On va gérer les événements du board avant chaque entité
    SDL_Point mousePos;

    if (event.type == SDL_MOUSEMOTION)
    {
        mousePos.x = event.motion.x;
        mousePos.y = event.motion.y;

        if (mCtx.mMode == PlayerContext::TABLE_MODE_PLAY)
        {
            if (mCtx.IsMyTurn())
            {
                std::vector<std::shared_ptr<GfxCard>> collision;

                for (auto & c : mCtx.mDeck)
                {
                    auto sprite = mFullDeck[c.ToString()];

                    sprite->SetHovered(false);
                    if (sprite->IsPointInCard(mousePos))
                    {
                        collision.push_back(sprite);
                    }
                }

                if (collision.size() > 0)
                {
                    std::shared_ptr<GfxCard> onTop = collision.at(0);
                    uint32_t maxZ = onTop->GetZ();

                    for (uint32_t i = 1; i < collision.size(); i++)
                    {
                        if (collision.at(i)->GetZ() > maxZ)
                        {
                            onTop = collision.at(i);
                            maxZ = onTop->GetZ();
                        }
                    }

                    if (onTop)
                    {
                        onTop->SetHovered(true);
                    }
                }
            }
        }
    }

    if (event.type == SDL_MOUSEBUTTONUP)
    {
        if (mCtx.mMode == PlayerContext::TABLE_MODE_PLAY)
        {
            if (mCtx.IsMyTurn())
            {
                std::vector<std::shared_ptr<GfxCard>> collision;

                for (auto & c : mCtx.mDeck)
                {
                    auto sprite = mFullDeck[c.ToString()];

                    if (sprite->IsSelected())
                    {
                        sprite->SetSelectable(false);
                        // On bloque tout de suite
                        mEvent.SendMyCard(c);
                    }
                }
            }
        }
        else if (mCtx.mMode == PlayerContext::TABLE_MODE_WAIT_CLICK_END_OF_TRICK)
        {
            mEvent.ClickOnBoard();
        }
    }
}


void BoardScene::DrawCentralGame()
{
    std::vector<Users::Entry> oppo = mCtx.mUsers.GetTableUsers(Protocol::TABLES_UID);

    for (auto &o : oppo)
    {
        //   std::cout << "Oppo: " << o.identity.nickname << "Pos: " << o.place.ToString() << std::endl;
        Place rel = mCtx.mMyself.place.Relative(o.player.place, mCtx.mGameState.mNbPlayers);  // relative place
        bool select = mCtx.mCurrentPlayer == o.player.place;
        std::string taker;

        if (mCtx.mBid.taker == o.player.place)
        {
            taker = "(preneur)";
        }

        mHud.PlayerBoxUI(rel.ToString(),
                    o.identity.username + taker,
                    mCtx.mSits[o.player.place.Value()].contract.ToString(),
                    gBoxPositions[rel.Value()].x,
                    gBoxPositions[rel.Value()].y, select);
    }

}
