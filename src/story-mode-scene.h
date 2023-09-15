#ifndef STORYMODESCENE_H
#define STORYMODESCENE_H


#include <vector>
#include <map>
#include <memory>
#include "gfx-engine.h"
#include "i-board-event.h"
#include "DataBase.h"

class IFranceEvent
{
public:
    virtual  ~IFranceEvent() {}
    virtual void SelectCity(int id) = 0;

};

class Car : public Entity
{
public:
    Car(GfxSystem &s)
        : Entity(s)
    {

    }
    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void Draw(SDL_Renderer *renderer) override;

private:
    std::string mDeuxCvSVG;
    SDL_Texture *mTexture = nullptr;
};


class FranceMap : public Image
{

public:
    FranceMap(GfxSystem &s)
        : Image(s, "assets/story/france.png")
    {
        m_cursor_x = 0;
        m_cursor_y = 0;
        m_zoom = 1.0;
    }

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void Draw(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event) override;

    int GetWZoomed() { return mWZoomed; }
    int GetHZoomed() { return mHZoomed; }

    int GetCursorX() {return m_cursor_x; }
    int GetCursorY() {return m_cursor_y; }

private:
    float m_zoom;
    float m_cursor_x;
    float m_cursor_y;
    float m_map_x;
    float m_map_y;
    int mWZoomed;
    int mHZoomed;
};

class DenisHead : public Image
{

public:
    DenisHead(GfxSystem &s)
        : Image(s, "assets/story/denis_lunettes.png")
    {

    }
private:

};

class City : public Image
{

public:
    City(GfxSystem &s, IFranceEvent &ev, int id);

    int GetId() const { return m_id; }

    void SetHovered(bool hovered)
    {
        if (hovered != m_hovered)
        {
            m_hovered = hovered;

            if (m_hovered)
            {
                SetScale(0.2, 0.2);
                SetPos(m_scaledRect.x, m_scaledRect.y);
            }
            else
            {
                SetScale(0.1, 0.1);
                SetPos(m_normalRect.x, m_normalRect.y);
            }
        }
    }

    void SetSelected(bool selected)
    {
        m_selected = selected;
        m_selection->SetVisible(selected);
    }

    void SetName(const std::string &name) {
        m_name = name;
    }

    std::string GetCityName() const  {
        return m_name;
    }

    void Place(int x, int y)
    {
        // Memorize position in pixels of the GPS point
        m_x = x;
        m_y = y;

        // Precompute normal rect, not scaled
        m_normalRect = GetRect();
        m_scaledRect = m_normalRect;

        ComputePos(m_normalRect, 0.1, 0.1);
        ComputePos(m_scaledRect, 0.2, 0.2);

        SetPos(m_normalRect.x, m_normalRect.y);

        // Place selection square

        SDL_Rect r = m_selection->GetRect();
        ComputePos(r, 0.3, 0.3);
        m_selection->SetScale(0.3, 0.3);
        m_selection->SetPos(r.x, r.y);
    }

    void ComputePos(SDL_Rect &rect, float scale_x, float scale_y)
    {
        // Compute real position, center City on the position
        rect.w *= scale_x;
        rect.h *= scale_y;

        rect.x = m_x - rect.w / 2;
        rect.y = m_y - rect.h / 2;
    }

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event) override;

    virtual void Update(double deltaTime) override;

    virtual void Draw(SDL_Renderer *renderer) override;

    double lon, lat;
private:

    std::shared_ptr<Image> m_selection;
    IFranceEvent &m_ev;
    int m_id{0};
    int m_x, m_y;
    SDL_Rect m_normalRect;
    SDL_Rect m_scaledRect;
    bool m_hovered{false};
    bool m_selected{false};
    std::string m_name;

    int m_magazines{100};

};

class Velo : public Image
{

public:
    Velo(GfxSystem &s)
        : Image(s, "assets/story/velo.png")
    {

    }
private:

};

class StoryModeScene : public Scene, public IFranceEvent
{
public:
    StoryModeScene(GfxSystem &system, IBoardEvent &event);

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args) override;

    virtual void Update(double deltaTime) override;

    virtual void Draw(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event) override;

    // From IFranceEvent
    virtual void SelectCity(int id);

private:
    IBoardEvent &mEvent;

    DataBase mDb;

    double lon, lat;
    SDL_Point city;

    double west;
    double east;
    double south;
    double north;
    double ymin;
    double ymax;

    double xFactor{1.0};
    double yFactor{1.0};

    bool m_showPopup{true};

    std::shared_ptr<Car> mCar;
    std::shared_ptr<FranceMap> m_map;
    std::shared_ptr<DenisHead> m_head;
    std::shared_ptr<Velo> m_velo;

    std::shared_ptr<Image> m_ivan;

    std::shared_ptr<Text> m_questsTitle;
    std::shared_ptr<Text> m_infosTitle;

    std::string m_currentSelection{"-"};

    std::vector<std::shared_ptr<City>> m_cities;
    void GeneratePath();
    SDL_Point GpsToPoint(double lon, double lat);
    void DrawQuestsMenu();
    void DrawToolBar();
    void DrawInfosMenu();
    void DrawPopupEvent();
};

#endif // STORYMODESCENE_H
