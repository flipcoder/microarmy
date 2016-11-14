#include "Intro.h"
#include "Qor/BasicPartitioner.h"
#include "Qor/Input.h"
#include "Qor/Qor.h"
#include "Qor/Sound.h"
#include "Qor/Canvas.h"
#include "Qor/Menu.h"
#include "Qor/Material.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>

using namespace std;
using namespace glm;
using namespace Filesystem;

Intro :: Intro(Qor* engine):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline()),
    m_pResources(engine->resources()),
    m_pController(engine->session()->active_profile(0)->controller().get()),
    //m_pCanvas(make_shared<Canvas>(
    //    engine->window()->size().x, engine->window()->size().y
    //)),
    m_pMenuGUI(make_shared<MenuGUI>(
        engine->session()->active_profile(0)->controller().get(),
        &m_MenuContext,
        &m_MainMenu,
        engine->pipeline()->partitioner(),
        engine->window(),
        engine->resources(),
        "PressStart2P-Regular.ttf",
        engine->window()->size().y / 30.0f,
        nullptr,
        7,
        1.5f,
        Text::LEFT,
        32.0f,
        MenuGUI::F_BOX
    ))
{}


void Intro :: preload() {
    auto win = m_pQor->window();
    float sw = m_pQor->window()->size().x;
    float sh = m_pQor->window()->size().y;
    
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera);
    m_pRoot->add(m_pMenuGUI);

    //m_pRoot->add(m_pCanvas);

    m_pMusic = m_pQor->make<Sound>("menu.ogg");
    m_pRoot->add(m_pMusic);

    auto mat = make_shared<Material>("title.png", m_pResources);
    auto bg = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(sw, sh), vec2(0.0f, 0.0f))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(vec2(1.0f, 0.0f), vec2(0.0f, 1.0f)))
        },
        make_shared<MeshMaterial>(mat)
    );
    mat->diffuse(Color(1.0f, 0.25f));
    //auto bg2 = bg->instance();
    bg->position(vec3(0.0f,0.0f,-1.0f));
    //bg2->position(vec3(0.0f,0.0f,-1.0f));
    m_pRoot->add(bg);
}


Intro :: ~Intro() {
    m_pPipeline->partitioner()->clear();
}


void Intro :: enter() {
    auto qor = m_pQor;
    m_pMusic->play();
    
    m_pCamera->ortho();
    m_pPipeline->winding(true);
    m_pPipeline->bg_color(Color::white());
    m_pInput->relative_mouse(false);
    
    //m_pPipeline->blend(true);
    
    m_MainMenu.options().emplace_back("PLAY", [this]{
        m_MenuContext.push(&m_LevelMenu);
    });
    m_MainMenu.options().emplace_back("OPTIONS", [this]{
        m_MenuContext.push(&m_OptionsMenu);
    });
    m_MainMenu.options().emplace_back("QUIT", [this]{
        m_pQor->pop_state();
    });

    #ifdef _WIN32
        auto lines = kit::lines(file_to_string("data\\maps\\maps.txt"));
    #else
        auto lines = kit::lines(file_to_string("data/maps/maps.txt"));
    #endif
   
    int count = 1;
    for (string& line : lines) {
        m_LevelMenu.options().emplace_back(boost::to_upper_copy(line), [qor, count] {
            qor->args().set("map", to_string(count));
            qor->change_state("pregame");
        });
        count++;
    }

    /*
    m_LevelMenu.options().emplace_back("HOUSE", [qor]{
        qor->args().set("map", "1");
        qor->change_state("pregame");
    });
    m_LevelMenu.options().emplace_back("BACKYARD", [qor]{
        qor->args().set("map", "2");
        qor->change_state("pregame");
    });
    */
    m_LevelMenu.options().emplace_back(
        "BACK",
        [this]{
            m_pQor->save_settings();
            m_MenuContext.pop();
        },
        std::function<bool(int)>(),
        string(), // no desc
        Menu::Option::BACK
    );

    m_pVolumeText = std::make_shared<string>(string("Global Vol: ") + to_string(
        m_pResources->config()->meta("audio")->at<int>("volume")
        ) + "%"
    );
    m_pSoundText = std::make_shared<string>(string("Sound Vol: ") + to_string(
        m_pResources->config()->meta("audio")->at<int>("sound-volume")
        ) + "%"
    );
    m_pMusicText = std::make_shared<string>(string("Music Vol: ") + to_string(
        m_pResources->config()->meta("audio")->at<int>("music-volume")
        ) + "%"
    );

    m_OptionsMenu.options().emplace_back(m_pVolumeText,
        [this]{
        },
        [this](int ofs){
            int old_v = m_pResources->config()->meta("audio")->at<int>("volume");
            int v = kit::clamp(old_v + ofs * 10, 0, 100);
            if(v!=old_v) {
                m_pResources->config()->meta("audio")->set<int>("volume", v);
                *m_pVolumeText = string("Global Vol: ") + to_string(v) + "%";
                Sound::play(m_pRoot.get(), "scroll.wav", m_pResources);
                return true;
            }
            return false;
        }
    );
    m_OptionsMenu.options().emplace_back(m_pMusicText,
        [this]{
            //Sound::play(m_pRoot.get(), "scroll.wav", m_pResources);
        },
        [this](int ofs){
            int old_v = m_pResources->config()->meta("audio")->at<int>("music-volume");
            int v = kit::clamp(old_v + ofs * 10, 0, 100);
            if(v!=old_v) {
                m_pResources->config()->meta("audio")->set<int>("music-volume", v);
                *m_pMusicText = string("Music Vol: ") + to_string(v) + "%";
                return true;
            }
            return false;
        });
    m_OptionsMenu.options().emplace_back(m_pSoundText,
        [this]{
        },
        [this](int ofs){
            int old_v = m_pResources->config()->meta("audio")->at<int>("sound-volume");
            int v = kit::clamp(old_v + ofs * 10, 0, 100);
            if(v!=old_v) {
                m_pResources->config()->meta("audio")->set<int>("sound-volume", v);
                *m_pSoundText = string("Sound Vol: ") + to_string(v) + "%";
                Sound::play(m_pRoot.get(), "scroll.wav", m_pResources);
                return true;
            }
            return false;
        });
    m_OptionsMenu.options().emplace_back(
        "Back",
        [this]{
            m_pQor->save_settings();
            m_MenuContext.pop();
        },
        std::function<bool(int)>(), // no adjust
        string(), // no desc
        Menu::Option::BACK
    );
    
    m_MenuContext.clear(&m_MainMenu);
    m_pRoot->add(m_pMenuGUI);
}


void Intro :: logic(Freq::Time t) {
    if (m_pInput->key(SDLK_ESCAPE).pressed_now()) {
        m_pQor->quit();
        return;
    }

    m_pRoot->logic(t);
}


void Intro :: render() const {
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}
