#include "board-hud.h"
#include "scenes.h"
#include "IconsMaterialDesign.h"
#include "IconsFontAwesome5.h"

BoardHud::BoardHud(PlayerContext &ctx, IBoardEvent &event)
    : mCtx(ctx)
    , mEvent(event)
{

}

void BoardHud::ShowOptionsWindow()
{
    static bool firstTime = true;
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (firstTime)
        {
            firstTime = false;
            mTempOptions = mCtx.mOptions;
        }

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("OptionsTabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Jeu"))
            {
                ImGui::Checkbox("Auto play", &mTempOptions.autoPlay);
                ImGui::Checkbox("End of trick validation by clicking", &mTempOptions.clickToClean);
                ImGui::SliderInt("slider int2", &mTempOptions.delayBeforeCleaning, 0, 5000);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Bots"))
            {
                ImGui::Text("This is the bots tab!\nblah blah blah blah blah");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            // On applique la configuration
            mCtx.mOptions = mTempOptions;
            firstTime = true;
            mEvent.ConfigChanged();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            firstTime = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void BoardHud::PlayerBoxUI(const std::string &boxName, const std::string &nickName, const std::string &text, float x, float y, bool select)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove;

    if (select)
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(72.0 / 255.0, 224.0 / 255.0, 214.0 / 255.0, 1.00f));
    }
    else
    {
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    }
    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);

    std::string windowName = "Box" + boxName;
    if (ImGui::Begin(windowName.c_str(), NULL, window_flags))
    {
        ImGui::Text("%s", nickName.c_str());
        ImGui::Separator();
        ImGui::Text("%s", text.c_str());
    }
    ImGui::End();
    if (select)
    {
        ImGui::PopStyleColor();
    }
}


void BoardHud::DrawBidChooser(IBoardEvent &event)
{
    ImGui::SetNextWindowPos(ImVec2(200, 120));
    ImGui::SetNextWindowSize(ImVec2(250, 200));

    ImGuiWindowFlags window_flags = 0
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoSavedSettings
        ;
    ImGui::Begin("BidWindow", NULL, window_flags);
    ImGui::Text("Votre enchère");
    ImGui::Separator();

    if (ImGui::Button("Passe"))
    {
        mCtx.mMyBid.contract = Contract(Contract::PASS);
        event.SendMyBid();
    }
    if (ImGui::Button("Garde"))
    {
        mCtx.mMyBid.contract = Contract(Contract::PASS);
        event.SendMyBid();
    }
    if (ImGui::Button("Garde sans"))
    {
        mCtx.mMyBid.contract = Contract(Contract::PASS);
        event.SendMyBid();
    }
    if (ImGui::Button("Garde contre"))
    {
        mCtx.mMyBid.contract = Contract(Contract::PASS);
        event.SendMyBid();
    }

    if (ImGui::Checkbox("Chelem", &mCtx.mMyBid.slam))
    {

    }

    ImGui::End();
}


void BoardHud::DrawResult(IBoardEvent &event)
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGui::SetNextWindowSize(ImVec2(400, 200));

    ImGuiWindowFlags window_flags = 0
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoSavedSettings
        ;
    ImGui::Begin("ResultWindow", NULL, window_flags);
    ImGui::Text("Résultats");
    ImGui::Separator();

    ImGui::Text("(25 + %d + %d) * %d + %d + %d",
                abs(mCtx.mPoints.Difference()),
                mCtx.mPoints.GetLittleEndianPoints(),
                mCtx.mPoints.handlePoints,
                mCtx.mPoints.GetSlamPoints(mCtx.mBid),
                Tarot::GetMultiplier(mCtx.mBid.contract));

    // Show the formulat and final scoring
    ImGui::Text("Points de l'attaque : %d", mCtx.AttackPoints());
    ImGui::Text("Points de la défense : %d", mCtx.DefensePoints());

    ImGui::Text("Gagnant: %s", ( mCtx.mPoints.Winner() == Team::ATTACK) ? "Attaque" : "Défense");

    ImGui::Separator();
    if (ImGui::Button("Fermer"))
    {
        mEvent.ClickOnBoard();
    }

    ImGui::End();
}


void BoardHud::ToolbarUI(IBoardEvent &event)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(80, 40));

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

    ShowOptionsWindow();

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_SIGN_OUT_ALT))
    {
        event.ExitGame();
    }


    ImGui::End();
}


void BoardHud::DrawChatWindow(GfxSystem &system, IBoardEvent &event)
{
    static char text[500] = "\0";

    Rect rect = system.GetWindowSize();
    static const int windowWidth = 300;

    // position the controls widget in the top-right corner with some margin
    ImGui::SetNextWindowPos(ImVec2(rect.w - windowWidth, 0), ImGuiCond_Always);
    // here we set the calculated width and also make the height to be
    // be the height of the main window also with some margin
    ImGui::SetNextWindowSize(
                ImVec2(windowWidth, rect.h),
                ImGuiCond_Always
                );
    // create a window and append into it
    ImGui::Begin("ChatWindow", NULL, ImGuiWindowFlags_NoResize);


    if (ImGui::CollapsingHeader("Joueurs"))
    {
        if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            static uint32_t item_current_idx = 0;
            for (uint32_t n = 0; n < mCtx.mUsers.Size(); n++)
            {
                const bool is_selected = (item_current_idx == n);

                Users::Entry entry;
                if (mCtx.mUsers.GetEntryByIndex(n, entry))
                {
                    if (ImGui::Selectable(entry.identity.nickname.c_str(), is_selected))
                    {
                        item_current_idx = n;
                    }

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
            }
            ImGui::EndListBox();
        }
    }


    static bool AutoScroll = true;
    static bool ScrollToBottom = false;

    // Reserve enough left-over height for 1 separator + 1 input text
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

    // Display every line as a separate entry so we can change their color or add custom widgets.
    // If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
    // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
    // to only process visible items. The clipper will automatically measure the height of your first item and then
    // "seek" to display only items in the visible area.
    // To use the clipper we can replace your standard loop:
    //      for (int i = 0; i < Items.Size; i++)
    //   With:
    //      ImGuiListClipper clipper;
    //      clipper.Begin(Items.Size);
    //      while (clipper.Step())
    //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
    // - That your items are evenly spaced (same height)
    // - That you have cheap random access to your elements (you can access them given their index,
    //   without processing all the ones before)
    // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
    // We would need random-access on the post-filtered list.
    // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
    // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
    // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
    // to improve this example code!
    // If your items are of variable height:
    // - Split them into same height items would be simpler and facilitate random-seeking into your list.
    // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
    // if (copy_to_clipboard)
    //     ImGui::LogToClipboard();
    for (unsigned int i = 0; i < mCtx.mMessages.size(); i++)
    {
        PlayerContext::Message &item = mCtx.mMessages[i];

        // Normally you would store more information in your item than just a string.
        // (e.g. make Items[] an array of structure, store color/type etc.)
//        ImVec4 color;
//        bool has_color = false;
//        if (strstr(item, "[error]"))          { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
//        else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
//        if (has_color)
//            ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(item.msg.c_str());
//        if (has_color)
//            ImGui::PopStyleColor();
    }

    if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
    {
        ImGui::SetScrollHereY(1.0f);
    }
    ScrollToBottom = false;

    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Separator();

    // Command-line
    ImGui::SetNextItemWidth(-FLT_MIN);
    bool reclaim_focus = false;
    ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
    if (ImGui::InputText("##Input", text, IM_ARRAYSIZE(text), input_text_flags))
    {
        if (strlen(text) > 0)
        {
            event.ChatMessage(text);
            strcpy(text, "");
            reclaim_focus = true;
        }
    }

    if (reclaim_focus)
    {
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
    }
    ImGui::End();
}
