#ifndef _TESTSTATE_H
#define _TESTSTATE_H

#include "Qor/Node.h"
#include "Qor/State.h"
#include "Qor/Input.h"
#include "Qor/Camera.h"
#include "Qor/Pipeline.h"
#include "Qor/Mesh.h"

class Qor;

class Test:
    public State
{
    public:
        
        Test(Qor* engine);
        virtual ~Test();

        virtual void preload() override;
        virtual void enter() override;
        virtual void logic(Freq::Time t) override;
        virtual void render() const override;
        virtual bool needs_load() const override {
            return true;
        }

    private:
        
        Qor* m_pQor = nullptr;
        Input* m_pInput = nullptr;
        Pipeline* m_pPipeline = nullptr;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Camera> m_pCamera;
};

#endif


