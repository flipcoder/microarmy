#include "Test.h"
#include "Qor/BasicPartitioner.h"
#include "Qor/Input.h"
#include "Qor/Qor.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>
using namespace std;
using namespace glm;

Test :: Test(Qor* engine):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline())
{}

void Test :: preload()
{
    auto sw = m_pQor->window()->size().x;
    auto sh = m_pQor->window()->size().y;
    
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera);

    auto mat = make_shared<MeshMaterial>("bg.png", m_pQor->resources());
    auto mesh = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(0.0f, 0.0f), vec2(16.0f, 16.0f))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(vec2(0.0f,1.0f), vec2(1.0f,0.0f)))
        }, mat
    );
    m_pMesh1 = mesh;
    m_pRoot->add(mesh);

    mat = make_shared<MeshMaterial>("bg.png", m_pQor->resources());
    mesh = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(0.0f, 0.0f), vec2(16.0f, 16.0f))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(vec2(0.0f,1.0f), vec2(1.0f,0.0f)))
        }, mat
    );
    m_pMesh2 = mesh;
    mesh->position(glm::vec3(10.0f, 10.0f, 0.0f));
    m_pRoot->add(mesh);

    m_pPipeline->partitioner()->on_touch(m_pMesh1->as_node(), m_pMesh2->as_node(),
        [](Node* a, Node* b){
            LOG("touch");
        },
        [](Node* a, Node* b){
            LOG("untouch");
        }
    );
}

Test :: ~Test()
{
    m_pPipeline->partitioner()->clear();
}

void Test :: enter()
{
    m_pCamera->ortho();
    m_pPipeline->winding(true);
    m_pPipeline->bg_color(Color::black());
    m_pInput->relative_mouse(false);
}

void Test :: logic(Freq::Time t)
{
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    m_pRoot->logic(t);

    m_pMesh2->move(glm::vec3(t.s(), 0.0f, 0.0f));
}

void Test :: render() const
{
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}

