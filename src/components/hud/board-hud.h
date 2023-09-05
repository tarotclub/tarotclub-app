#ifndef BOARDHUD_H
#define BOARDHUD_H

#include "gfx-engine.h"
#include "PlayerContext.h"
#include "i-board-event.h"

class BoardHud
{
public:
    BoardHud(PlayerContext &ctx, IBoardEvent &event);
    void ShowOptionsWindow();
    void DrawBidChooser(IBoardEvent &event);
    void ToolbarUI(IBoardEvent &event);
    void DrawChatWindow(GfxSystem &system, IBoardEvent &event);
    void PlayerBoxUI(const std::string &boxName, const std::string &nickName, const std::string &text, float x, float y, bool select);
    void DrawResult(IBoardEvent &event);
private:
    PlayerContext &mCtx;
    IBoardEvent &mEvent;
    ClientOptions mTempOptions;
};

#endif // BOARDHUD_H
