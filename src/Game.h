#ifndef _PREGAMESTATE_H
#define _PREGAMESTATE_H

#include "Qor/Node.h"
#include "Qor/State.h"
#include "Qor/Input.h"
#include "Qor/Camera.h"
#include "Qor/Pipeline.h"
#include "Qor/Mesh.h"
#include "Qor/Console.h"
#include "Qor/TileMap.h"
#include "Qor/Sound.h"
#include "Qor/Sprite.h"

class Qor;

class Game:
    public State
{
    public:

        enum ObjectTypes
        {
            GROUND,
            STATIC,
            FATAL,
            CHARACTER,
            CHARACTER_FEET,
            CHARACTER_SIDES,
            BULLET,
            THING
        };

        
        Game(Qor* engine);
        virtual ~Game();

        virtual void preload() override;
        virtual void enter() override;
        virtual void logic(Freq::Time t) override;
        virtual void render() const override;
        virtual bool needs_load() const override {
            return true;
        }

        void reset();
        
        void cb_to_static(Node* a, Node* b, Node* m);
        void cb_to_tile(Node* a, Node* b);
        void cb_to_fatal(Node* a, Node* b);
        void cb_to_thing(Node* a, Node* b);
        void cb_bullet_to_static(Node* a, Node* b);
        void setup_player(std::shared_ptr<Sprite> player);

    private:
        
        Qor* m_pQor = nullptr;
        Cache<Resource, std::string>* m_pResources = nullptr;
        Input* m_pInput = nullptr;
        Pipeline* m_pPipeline = nullptr;
        BasicPartitioner* m_pPartitioner = nullptr;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Console> m_pConsole;
        std::shared_ptr<Camera> m_pCamera;
        std::shared_ptr<TileMap> m_pMap;
        std::shared_ptr<Controller> m_pController;
        std::shared_ptr<Sprite> m_pChar;
        std::shared_ptr<Node> m_pCharFocusLeft;
        std::shared_ptr<Node> m_pCharFocusRight;
        std::shared_ptr<Light> m_pViewLight;
        std::shared_ptr<Sound> m_pMusic;
        std::vector<MapTile*> m_Spawns;
        std::vector<MapTile*> m_AltSpawns;

        int m_LastWallJumpDir = 0;
        Freq::Alarm m_JumpTimer;
        bool m_WasInAir = false;

        unsigned m_Shader = 0;
};

#endif


