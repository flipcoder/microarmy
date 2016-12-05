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
#include "Qor/Text.h"

class Qor;

class Pregame: public State {
    public:
        // Constructor
        Pregame(Qor* engine);


        // Destructor
        virtual ~Pregame();


        // Overriden virtual methods
        virtual void preload() override;
        virtual void enter() override;
        virtual void logic(Freq::Time t) override;
        virtual void render() const override;
        virtual bool needs_load() const override { return true; }

    private:

        //static const std::vector<int> STAR_LEVELS;
        
        // Variables
        bool m_Win = false;
        Freq::Alarm m_Transition;

        // Pointers
        Qor* m_pQor = nullptr;
        Input* m_pInput = nullptr;
        Pipeline* m_pPipeline = nullptr;
        ResourceCache* m_pResources = nullptr;
        Controller* m_pController = nullptr;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Camera> m_pCamera;
        std::shared_ptr<Sound> m_pMusic;
        //std::shared_ptr<Canvas> m_pCanvas;

        std::shared_ptr<Font> m_pFont;
        std::shared_ptr<Text> m_pText;

        std::vector<int> m_Stars;
        std::vector<int> m_MaxStars;
};

#endif

