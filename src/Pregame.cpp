#include "Pregame.h"
#include "Qor/BasicPartitioner.h"
#include "Qor/Input.h"
#include "Qor/Material.h"
#include "Qor/Qor.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>

using namespace std;
using namespace glm;


Pregame :: Pregame(Qor* engine):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pResources(engine->resources()),
    m_pPipeline(engine->pipeline()),
    m_pController(engine->session()->active_profile(0)->controller().get()),
    m_Transition(engine->timer()->timeline())
{}


void Pregame :: preload() {
    auto win = m_pQor->window();
    float sw = m_pQor->window()->size().x;
    float sh = m_pQor->window()->size().y;

    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera);

    m_pMusic = m_pQor->make<Sound>("score.ogg");
    m_pRoot->add(m_pMusic);

    auto mat = make_shared<Material>("title2.png", m_pResources);
    auto bg = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(sw, sh), vec2(0.0f, 0.0f))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(vec2(1.0f, 0.0f), vec2(0.0f, 1.0f)))
        },
        make_shared<MeshMaterial>(mat)
    );

    bg->material()->ambient(Color::white() * 0.25f);
    mat->diffuse(Color(1.0f, 0.25f));
    bg->position(vec3(0.0f,0.0f,-1.0f));
    m_pRoot->add(bg);

    m_Transition.set(Freq::Time::seconds(11.0f));
}


Pregame :: ~Pregame() {
    m_pPipeline->partitioner()->clear();
}


void Pregame :: enter() {
    m_pMusic->play();
    
    m_pCamera->ortho();
    m_pPipeline->winding(true);
    m_pPipeline->bg_color(Color::black());
    m_pInput->relative_mouse(false);
}


void Pregame :: logic(Freq::Time t) {
    if (m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    if (m_pInput->key(SDLK_SPACE) ||
        m_pInput->key(SDLK_RETURN) ||
        m_pController->button("select").pressed_now() ||
        m_Transition.elapsed()
    ){
        m_pQor->change_state("game");
    }
    
    m_pRoot->logic(t);
}


void Pregame :: render() const {
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}
