#ifndef STORYMODESCENE_H
#define STORYMODESCENE_H


#include <vector>
#include <map>
#include <memory>
#include "gfx-engine.h"
#include "i-board-event.h"
#include "DataBase.h"
#include "dbtweener.h"

#include <SDL2/SDL_mixer.h>

class IFranceObject
{
public:
    virtual  ~IFranceObject() {}
    virtual void SelectCity(int id) = 0;
    virtual int GetTotalMagazines() const = 0;
    virtual void DenisArrivedInCity() = 0;

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


class City : public Image
{

public:
    City(GfxSystem &s, IFranceObject &ev, int id);

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

    void Initialize(bool initialized);

    bool Initialized() const {
        return m_initialized;
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

    void SetDenisInCity(bool is_here)
    {
        m_denisIsHere = is_here;
    }

    bool IsDenisHere() const {
        return m_denisIsHere;
    }

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event) override;

    virtual void Update(double deltaTime) override;

    virtual void Draw(SDL_Renderer *renderer) override;

    int GetMagazines() const { return m_magazines; }
    void SetMagazines(int m) { m_magazines = m; }

    double lon, lat;
private:

    std::shared_ptr<Image> m_selection;
    IFranceObject &m_ev;
    int m_id{0};
    int m_x, m_y;
    SDL_Rect m_normalRect;
    SDL_Rect m_scaledRect;
    bool m_hovered{false};
    bool m_selected{false};
    std::string m_name;

    bool m_initialized{false};

    bool m_denisIsHere{false};

    int m_magazines{100};

};

class Denis : public Image , public CDBTweener::IListener
{

public:
    Denis(GfxSystem &s, IFranceObject &franceEvent)
        : Image(s, "assets/story/denis_lunettes.png")
        , m_franceEvent(franceEvent)
    {

    }

    void onTweenFinished(CDBTweener::CTween *pTween) override
    {
        isRouling = false;
        m_franceEvent.DenisArrivedInCity();
    }

    bool IsRouling() const { return isRouling; }

    virtual void Update(double deltaTime) override
    {
        Image::Update(deltaTime);
        if (isRouling)
        {
            oTweener.step(deltaTime);
            SetPos(m_x, m_y);
        }
    }

    double lon, lat;
    std::shared_ptr<City> m_inCity;

    void MoveTo(int x, int y)
    {
        isRouling = true;
        m_x = GetX();
        m_y = GetY();

        // CTweenListener oListener;
        CDBTweener::CTween *pTween = new CDBTweener::CTween();

        pTween->setEquation(&CDBTweener::TWEQ_ELASTIC, CDBTweener::TWEA_OUT, 2000.0f);
        pTween->addValue(&m_x, x);
        pTween->addValue(&m_y, y);

        oTweener.addTween(pTween);
        oTweener.addListener(this);
    }

private:
    IFranceObject &m_franceEvent;
    bool isRouling{false};
    CDBTweener oTweener;
    float m_x;
    float m_y;

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

class Montargis : public Image
{

public:
    Montargis(GfxSystem &s)
        : Image(s, "assets/story/montargis.png")
    {

    }
    double lon, lat;
private:

};

class Piscine : public Image
{

public:
    Piscine(GfxSystem &s)
        : Image(s, "assets/story/piscine.png")
    {

    }
private:

};



class Quest
{

public:
    Quest(const std::string &description);
    virtual ~Quest() {
        std::cout << "Delete quest" << std::endl;
    }

    virtual void CheckObjective() = 0;

    void SetFinished(bool finished) { m_finished = finished; }
    void SetSuccess(bool success) { m_success = success; }

    float GetMinutesLeft() const { return m_minutesLeft; }

    bool GetFinished() const { return m_finished; }

    bool GetSuccess() const
    {
        return m_success;
    }

    std::string GetDescription() const { return m_description; }

    void SetMinutes(float min) { m_minutesLeft = min; }
    void AddMinutes(float min) {
        m_minutesLeft += min;
        if (m_minutesLeft < 0.0)
        {
            m_minutesLeft = 0.0;
        }
    }

private:
    std::string m_description;
    bool m_finished{false};
    bool m_success{false};
    float m_minutesLeft{0};
};

class MainQuest : public Quest
{

public:

    MainQuest(IFranceObject &franceEvent, const std::string &description)
        : Quest(description)
        , m_franceEvent(franceEvent)
    {
        SetMinutes(17280);
    }

    virtual void CheckObjective() override {
        if (GetMinutesLeft() == 0.0)
        {
            SetFinished(true);
            SetSuccess(false);
        } else if (m_franceEvent.GetTotalMagazines() == 0)
        {
            SetFinished(true);
            SetSuccess(true);
        }
    }

private:
    IFranceObject &m_franceEvent;
};



class StoryModeScene : public Scene, public IFranceObject
{
public:
    StoryModeScene(GfxSystem &system, IBoardEvent &event);

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args) override;

    virtual void Update(double deltaTime) override;

    virtual void Draw(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event) override;

    // From IFranceEvent
    virtual void SelectCity(int id) override;
    virtual int GetTotalMagazines() const override;
    virtual void DenisArrivedInCity() override;

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

    float m_endurance{100.0};

    bool m_showPopup{true};

    std::string m_popupText;

    std::shared_ptr<Car> mCar;
    std::shared_ptr<FranceMap> m_map;
    std::shared_ptr<Denis> m_denis;
    std::shared_ptr<Velo> m_velo;
    std::shared_ptr<Montargis> m_montargis;

    std::vector<std::shared_ptr<Piscine>> m_piscines;

    int m_currentDay{1};

    std::shared_ptr<Image> m_ivan;

    std::shared_ptr<Text> m_questsTitle;
    std::shared_ptr<Text> m_infosTitle;
    std::shared_ptr<Text> m_pointsTitle;

    std::string m_currentSelection{"-"};
    std::shared_ptr<City> m_citySel;
    float m_distanceVoyage;
    float m_tempsVoyage;
    int m_points{0};
    int m_totalMagazines{0};
    bool m_lastPopup{false};

    std::vector<std::shared_ptr<Quest>> m_quests;

    std::vector<std::shared_ptr<City>> m_cities;
    void GeneratePath();
    SDL_Point GpsToPoint(double lon, double lat);
    void DrawQuestsMenu();
    void DrawToolBar();
    void DrawInfosMenu();
    void DrawPopupEvent();
    std::string MinutesToString(float decimalMinutes) const;
};

#endif // STORYMODESCENE_H
