#ifndef _PREGAMESTATE_H
#define _PREGAMESTATE_H

#include "Qor/Node.h"
#include "Qor/State.h"
#include "Qor/Input.h"
#include "Qor/Camera.h"
#include "Qor/Pipeline.h"
#include "Qor/Mesh.h"
#include "Qor/TileMap.h"
#include "Qor/Sound.h"
#include "Qor/Sprite.h"
#include "HUD.h"


class Qor;
class Thing;
class Monster;
class Player;

class Game: public State {
    public:
        // Definitions
        enum ObjectTypes {
            STATIC,
            LEDGE,
            FATAL,
            CHARACTER,
            CHARACTER_FEET,
            CHARACTER_SIDES,
            BULLET,
            THING,
            MONSTER,
            SENSOR
        };

        struct ParallaxLayer {
            std::shared_ptr<Node> root;
            std::shared_ptr<Light> light;
            float scale = 1.0f;
        };

        // Signal
        boost::signals2::signal<void()> on_reset;

        
        // Constructor        
        Game(Qor* engine);
        

        // Destructor
        virtual ~Game();

        
        // Overriden virtual methods
        virtual void preload() override;
        virtual void enter() override;
        virtual void logic(Freq::Time t) override;
        virtual void render() const override;
        virtual bool needs_load() const override { return true; }
        virtual std::shared_ptr<Node> root() override { return m_pRoot; }

        // Setup methods
        void setup_player(std::shared_ptr<Player> player);
        void setup_thing(std::shared_ptr<Thing> thing);
        void setup_monster(std::shared_ptr<Monster> monster);
        void setup_player_to_thing(std::shared_ptr<Player> player, std::shared_ptr<Thing> thing);
        void setup_player_to_monster(std::shared_ptr<Player> player, std::shared_ptr<Monster> monster);
        

        // Methods
        void reset();
        void end();
        std::vector<Node*> get_static_collisions(Node* a);
        std::vector<const Node*> vnodes() { return m_VisibleNodes; }
        std::vector<std::shared_ptr<Player>>& players() { return m_Players; }
        Qor* qor() { return m_pQor; }
        void checkpoint(Node* chk);

        // Callbacks
        void cb_to_static(Node* a, Node* b, Node* m);
        void cb_to_ledge(Node* a, Node* b);
        void cb_to_tile(Node* a, Node* b);
        void cb_to_fatal(Node* a, Node* b);
		void cb_touch_to_fatal(Node* a, Node* b);
        void cb_thing(Node* a, Node* b);
        void cb_bullet_to_static(Node* a, Node* b);
        

    private:
        // Variables
        int m_StarLevel = 0;
        unsigned m_Shader = 0;
        std::vector<int> m_Stars;
        std::vector<int> m_MaxStars;

        std::vector<std::shared_ptr<Thing>> m_Things;
        std::vector<std::shared_ptr<Monster>> m_Monsters;   
        std::vector<std::shared_ptr<Player>> m_Players;
        std::vector<ParallaxLayer> m_ParallaxLayers;

        std::vector<Node*> m_Spawns;
        std::vector<Node*> m_AltSpawns;
        
        std::vector<const Node*> m_VisibleNodes;

        // Pointers
        Qor* m_pQor = nullptr;
        ResourceCache* m_pResources = nullptr;
        Input* m_pInput = nullptr;
        Pipeline* m_pPipeline = nullptr;
        BasicPartitioner* m_pPartitioner = nullptr;
        TileLayer* m_pMainLayer = nullptr;
        Freq::Timeline* m_pTimeline;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Node> m_pOrthoRoot;
        std::shared_ptr<Camera> m_pOrthoCamera;
        std::shared_ptr<Camera> m_pCamera;
        std::shared_ptr<TileMap> m_pMap;
        std::shared_ptr<Controller> m_pController;
        std::shared_ptr<Player> m_pChar;
        std::shared_ptr<Light> m_pViewLight;
        std::shared_ptr<Sound> m_pMusic;
        std::shared_ptr<HUD> m_pHUD;
};

#endif
