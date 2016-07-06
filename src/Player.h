#ifndef PLAYER_H_E74SGBRG
#define PLAYER_H_E74SGBRG

#include "Qor/Sprite.h"
#include "Qor/Input.h"
#include "Qor/Camera.h"

class Game;

class Player: public Sprite {
    public:
        // Constructor
        Player(
            std::string fn,
            Cache<Resource, std::string>* resources,
            Camera* camera,
            Controller* ctrl,
            Freq::Timeline* timeline,
            IPartitioner* part,
            Game* game
        );


        // Destructor
        virtual ~Player();

        
        // Overidden virtual methods
        virtual void logic_self(Freq::Time t) override;


        // Getters
        std::shared_ptr<Node> focus_right() { return m_pCharFocusRight; };
        std::shared_ptr<Node> focus_left() { return m_pCharFocusLeft; };
        

        // Methods
        void enter();
        void reset_walljump();
        void shoot();
        void battery(int b) { m_Power += b; }
        void reset();
        

        // Callbacks
        static void cb_to_bullet(Node* player_node, Node* bullet);
        
        
    private:
        // Variables
        unsigned m_Power = 0;
        int m_LastWallJumpDir = 0;
        bool m_WasInAir = false;

        Freq::Alarm m_JumpTimer;
        Freq::Alarm m_ShootTimer;


        // Pointers
        Game* m_pGame;
        Camera* m_pCamera;
        Controller* m_pController;
        IPartitioner* m_pPartitioner;
        Freq::Timeline* m_pTimeline;
        Cache<Resource, std::string>* m_pResources;

        std::shared_ptr<Node> m_pCharFocusLeft;
        std::shared_ptr<Node> m_pCharFocusRight;
};

#endif
