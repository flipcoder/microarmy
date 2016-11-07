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

    m_pStarText = std::make_shared<Text>(m_pFont);
    m_pLivesText = std::make_shared<Text>(m_pFont);
    m_pHealthText = std::make_shared<Text>(m_pFont);
    m_pGodText = std::make_shared<Text>(m_pFont);

    m_pLivesText->position(glm::vec3(sw / 2.0f, 0.0f, 0.0f));
    m_pHealthText->position(glm::vec3(sw, 0.0f, 0.0f));
    m_pGodText->position(glm::vec3(0.0f, sh *.1, 0.2f));

    m_pStarText->align(Text::LEFT);
    m_pLivesText->align(Text::CENTER);
    m_pHealthText->align(Text::RIGHT);
    m_pGodText->align(Text::LEFT);

    add(m_pStarText);
    add(m_pLivesText);
    add(m_pHealthText);
    add(m_pGodText);

    set(0, 0, 0);

    auto _this = this;
    m_HealthCon = m_pPlayer->on_health_change.connect([_this](int){_this->m_bDirty = true; });
    m_GodCon = m_pPlayer->on_god_change.connect([_this](bool) {_this->m_bDirty = true; });

    auto mat = make_shared<MeshMaterial>("items.png", m_pCache);

    m_pHeart = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(sw / 24, sw / 24))),
        vector<shared_ptr<IMeshModifier>>{
        make_shared<Wrap>(Prefab::tile_wrap(
            // Y Y (height is tile size for both dims)
            uvec2(16, 16),
            // X Y
            uvec2(mat->texture()->size().x, mat->texture()->size().y),
            1
        ))
    }, mat
        );

    mat = make_shared<MeshMaterial>("guy-jump.png", m_pCache);
    m_pGuy = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(sw / 24, sw / 24))),
        vector<shared_ptr<IMeshModifier>>{
        make_shared<Wrap>(Prefab::quad_wrap(
            glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f)
        ))
        }, mat
    );
    
    add(m_pHeart);
    add(m_pGuy);
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
    
    m_pStarText->set("  " + to_string(m_Stars) + "/" + to_string(m_MaxStars));
    m_pHealthText->set(to_string(m_pPlayer->health()) + "%");
    m_pLivesText->set(" " + to_string(m_pPlayer->lives()));
    m_pGodText->set("God Mode: " + to_string(m_pPlayer->god()));

    m_pGodText->redraw();
    m_pLivesText->redraw();
    m_pHealthText->redraw();
    m_pGuy->position(glm::vec3(sw/2.0f - m_pLivesText->size().x, 0.0f, 0.0f));
    m_pHeart->position(glm::vec3(sw - m_pHealthText->size().x - 64.0f, 0.0f, 0.0f));
}

void HUD :: logic_self(Freq::Time) {
    if (m_bDirty) {
        redraw();

        //m_pHeart->position(m_pHealthText->position() - glm::vec3(

        //    m_pHealthText->children()[0]->box().size().x, 0.0f, 0.0f

        //));

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
                    uvec2(16,16),
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
