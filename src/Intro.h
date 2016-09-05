#ifndef PREGAME_H_Q9JIKEWU
#define PREGAME_H_Q9JIKEWU

#include "Qor/ResourceCache.h"
#include "Qor/Node.h"
#include "Qor/State.h"
#include "Qor/Input.h"
#include "Qor/Camera.h"
#include "Qor/Pipeline.h"
#include "Qor/Mesh.h"
#include "Qor/Menu.h"


class Qor;
class Sound;
class Canvas;

class Intro: public State {
    public:
        // Constructor
        Intro(Qor* engine);

        // Destructor
        virtual ~Intro();

        // Overriden virtual methods
        virtual void preload() override;
        virtual void enter() override;
        virtual void logic(Freq::Time t) override;
        virtual void render() const override;
        virtual bool needs_load() const override { return true; }


    private:
        // Variables
        //MenuContext m_MenuContext;
        //Menu m_MainMenu;
        //Menu m_LevelMenu;
        //Menu m_OptionsMenu;
        //std::shared_ptr<MenuGUI> m_pMenuGUI;

        // Pointers
        Qor* m_pQor = nullptr;
        Input* m_pInput = nullptr;
        Pipeline* m_pPipeline = nullptr;
        ResourceCache* m_pResources = nullptr;
        Controller* m_pController = nullptr;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Camera> m_pCamera;
        std::shared_ptr<Sound> m_pMusic;
        std::shared_ptr<Canvas> m_pCanvas;
        std::shared_ptr<std::string> m_pVolumeText;
        std::shared_ptr<std::string> m_pSoundText;
        std::shared_ptr<std::string> m_pMusicText;
};

#endif
