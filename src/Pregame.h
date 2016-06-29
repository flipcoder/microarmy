#ifndef PREGAME_H_OTKL19RW
#define PREGAME_H_OTKL19RW

#include "Qor/ResourceCache.h"
#include "Qor/Node.h"
#include "Qor/State.h"
#include "Qor/Input.h"
#include "Qor/Camera.h"
#include "Qor/Pipeline.h"
#include "Qor/Mesh.h"
#include "Qor/Sound.h"
#include "Qor/Canvas.h"

class Qor;

class Pregame: public State {
    public:
        
        Pregame(Qor* engine);
        virtual ~Pregame();

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
        ResourceCache* m_pResources = nullptr;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Camera> m_pCamera;
        std::shared_ptr<Sound> m_pMusic;
        
        Controller* m_pController = nullptr;

        Freq::Alarm m_Transition;

        std::shared_ptr<Canvas> m_pCanvas;
};

#endif
