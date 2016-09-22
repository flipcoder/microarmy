#include "Qor/Mesh.h"
#include "HUD.h"

using namespace std;
using namespace glm;

const std::vector<int> HUD :: STAR_LEVELS = { 7, 6, 2 };

HUD :: HUD(Window* window, Input* input, Cache<Resource,std::string>* cache, Player* player):
    m_pWindow(window),
    m_pInput(input),
    m_pCache(cache),
    m_pPlayer(player)
{
    auto sw = m_pWindow->size().x;
    auto sh = m_pWindow->size().y;

    //m_pCanvas = make_shared<Canvas>(sw, sh);
    //add(m_pCanvas);

    m_pFont = std::make_shared<Font>(
        cache->transform(string("PressStart2P-Regular.ttf:") +
            to_string(int(sw / 36.0f + 0.5f))),
        cache
    );
    m_pText = std::make_shared<Text>(m_pFont);
    add(m_pText);
    
    set(0, 0, 0);

    auto _this = this;
    m_HealthCon = m_pPlayer->on_health_change.connect([_this](int){_this->m_bDirty = true; });
}


void HUD :: redraw() {
    auto sw = m_pWindow->size().x;
    auto sh = m_pWindow->size().y;
    
    //// clear transparent
    //auto ctext = m_pCanvas->context();
    //m_pCanvas->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));
    
    //auto layout = m_pCanvas->layout();
    //layout->set_wrap(Pango::WRAP_WORD);
    
    //m_FontDesc = Pango::FontDescription("Press Start 2P " + to_string(sw / 36));
    //m_pCanvas->layout()->set_font_description(m_FontDesc);
    
    //ctext->set_source_rgba(1.0, 1.0, 1.0, 0.75);
    //layout->show_in_cairo_context(ctext);

    //m_pCanvas->dirty(true);
    
    m_pText->set("  " + to_string(m_Stars) + "/" + to_string(m_MaxStars) + "\n"
        + "Health: " + to_string(m_pPlayer->health()) + "/100" + "\n"
        + "Lives: " + to_string(m_pPlayer->lives()));
}


void HUD :: logic_self(Freq::Time) {
    if (m_bDirty) {
        redraw();
        m_bDirty = false;
    }
}


void HUD :: set(int star_lev, int stars, int max_stars) {
    if (star_lev != m_StarLev) {
        m_StarLev = star_lev;
        
        auto sw = m_pWindow->size().x;
        auto sh = m_pWindow->size().y;
        auto mat = make_shared<MeshMaterial>("items.png", m_pCache);

        if (m_pMesh)
            m_pMesh->detach();

        m_pMesh = make_shared<Mesh>(
            make_shared<MeshGeometry>(Prefab::quad(vec2(sw / 24, sw / 24))),
            vector<shared_ptr<IMeshModifier>>{
                make_shared<Wrap>(Prefab::tile_wrap(
                    // Y Y (height is tile size for both dims)
                    uvec2(mat->texture()->size().y, mat->texture()->size().y),
                    // X Y
                    uvec2(mat->texture()->size().x, mat->texture()->size().y),
                    STAR_LEVELS[m_StarLev]
                ))
            }, mat
        );
        add(m_pMesh);
    }
    m_Stars = stars;
    m_MaxStars = max_stars;
    m_bDirty = true;
}
