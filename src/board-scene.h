#ifndef BOARDSCENE_H
#define BOARDSCENE_H

#include <vector>
#include <map>
#include <memory>
#include "gfx-engine.h"
#include "i-board-event.h"
#include "PlayerContext.h"
#include "board-hud.h"
#include "SDL2_gfxPrimitives.h"

class GfxCard : public Image
{
public:
    GfxCard(GfxSystem &system, const std::string &path, const std::string &name)
        : Image(system, path)
        , mCard(name)
    {

    }

    virtual void Update(double deltaTime) override {
        (void) deltaTime;

        if (mSelectable && mHovered)
        {
            SetHighlight(true);
        }
        else
        {
            SetHighlight(false);
        }
    }

    virtual void Draw(SDL_Renderer *renderer) override {
        Image::Draw(renderer); // rendu de la carte

        // Par dessus on rend une sélection (un rectangle semi-opaque), si c'est activé
        if (IsHighlighted())
        {
            const SDL_Rect &r = GetRect();
            roundedBoxRGBA(renderer, r.x, r.y, r.x + r.w, r.y + r.h,  10, 255, 255, 255, 160);
        }
    }

    bool IsEqual(const Card &c) const
    {
        return c == mCard;
    }

    void SetSelectable(bool selectable)
    {
        mSelectable = selectable;
    }

    void SetHovered(bool hovered)
    {
        mHovered = hovered;
    }

    bool IsSelected() const {
        return mSelectable && mHovered;
    }

    bool IsOver(const std::shared_ptr<GfxCard> other) const
    {
        bool onTop = false;

        SDL_Rect intersect;

        if (other)
        {
            if (SDL_IntersectRect(&this->GetRect(), &other->GetRect(), &intersect))
            {
                if (this->GetZ() > other->GetZ())
                {
                    onTop = true;
                }
            }
        }

        return onTop;
    }

    bool IsPointInCard(const SDL_Point &pos) const
    {
        return SDL_PointInRect(&pos, &GetRect());
    }

private:
    Card mCard;
    bool mSelectable = false;
    bool mHovered = false;
};


class BoardScene : public Scene
{
public:   
    BoardScene(GfxSystem &system, IBoardEvent &event, PlayerContext &ctx, uint32_t scene);

    virtual void OnCreate(SDL_Renderer *renderer) override;
    virtual void OnActivate(SDL_Renderer *renderer) override;
    virtual void Update(double deltaTime) override;
    virtual void Draw(SDL_Renderer *renderer) override;
    virtual void ProcessEvent(const SDL_Event &event) override;

private:
    IBoardEvent &mEvent;
    PlayerContext &mCtx;
    uint32_t mSceneType;
    BoardHud    mHud;

    SDL_Texture *mBg;

    std::map<std::string, std::shared_ptr<GfxCard>> mFullDeck;

    void ShowSouthCards();
    ImGuiTextFilter       Filter;
    void DrawChatWindow();
    void ToolbarUI();
    void PlayerBoxUI(const std::string &boxName, const std::string &nickName, const std::string &text, float x, float y, bool select = false);
    void DrawCentralGame();
    void DrawBidChooser();
};


#endif // BOARDSCENE_H
