#include "Pregame.h"
#include "Qor/BasicPartitioner.h"
#include "Qor/Input.h"
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
    m_pPipeline(engine->pipeline()),
    m_pController(engine->session()->active_profile(0)->controller().get())
{}

void Pregame :: preload()
{
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera);

    m_pMusic = m_pQor->make<Sound>("score.ogg");
    m_pRoot->add(m_pMusic);
}

Pregame :: ~Pregame()
{
    m_pPipeline->partitioner()->clear();
}

void Pregame :: enter()
{
    m_pMusic->play();
    
    m_pCamera->ortho();
    m_pPipeline->winding(true);
    m_pPipeline->bg_color(Color::black());
    m_pInput->relative_mouse(false);
}

void Pregame :: logic(Freq::Time t)
{
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    if(m_pInput->key(SDLK_SPACE) ||
       m_pInput->key(SDLK_RETURN) ||
       m_pController->button("select").pressed_now()
    ){
        m_pQor->change_state("game");
    }
    
    m_pRoot->logic(t);
}

void Pregame :: render() const
{
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}

