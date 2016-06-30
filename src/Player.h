#ifndef PLAYER_H_E74SGBRG
#define PLAYER_H_E74SGBRG

#include "Qor/Sprite.h"
#include "Qor/Input.h"
#include "Qor/Camera.h"

class Game;

class Player: public Sprite {
    
    public:

        Player(
            std::string fn,
            Cache<Resource, std::string>* resources,
            Camera* camera,
            Controller* ctrl,
            Freq::Timeline* timeline,
            IPartitioner* part,
            Game* game
        );
        virtual ~Player();

        void enter();
        
        virtual void logic_self(Freq::Time t) override;
        //virtual void render(Pass* pass) const override;

        std::shared_ptr<Node> focus_right() { return m_pCharFocusRight; };
        std::shared_ptr<Node> focus_left() { return m_pCharFocusLeft; };
        
        void shoot();
        void battery(int b) { m_Power += b; }
        void reset();
        
        static void cb_to_bullet(Node* player_node, Node* bullet);
        
        void reset_walljump();
        
    private:
        
        Freq::Timeline* m_pTimeline;
        
        Cache<Resource, std::string>* m_pResources;
        
        std::shared_ptr<Node> m_pCharFocusLeft;
        std::shared_ptr<Node> m_pCharFocusRight;
        
        Camera* m_pCamera;
        int m_LastWallJumpDir = 0;
        Freq::Alarm m_JumpTimer;
        Freq::Alarm m_ShootTimer;
        bool m_WasInAir = false;
        unsigned m_Power = 0;
        
        Controller* m_pController;
        IPartitioner* m_pPartitioner;

        Game* m_pGame;
};

#endif

