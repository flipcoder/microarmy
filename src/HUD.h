#ifndef HUD_H_KW10MMV6
#define HUD_H_KW10MMV6

#include <boost/circular_buffer.hpp>
#include "Qor/Window.h"
#include "Qor/Canvas.h"
#include "Qor/Input.h"
#include "Qor/Text.h"
#include "Player.h"

class HUD: public Node {
    public:
        // Constructor
        HUD(
            Window* window,
            Input* input,
            Cache<Resource,std::string>* cache,
            Player* player
        );

        // Destructor
        virtual ~HUD() {}


        // Overidden virtual functions
        virtual void logic_self(Freq::Time) override;


        // Functions
        void set(int star_lev, int stars, int max_stars);
        

    private:
        
        static const std::vector<int> STAR_LEVELS;

        bool m_bDirty = true;
        int m_StarLev = -1;
        int m_Stars = 0;
        int m_MaxStars = 0;

        //Pango::FontDescription m_FontDesc;

        Window* m_pWindow = nullptr;
        Input* m_pInput = nullptr;
        //std::shared_ptr<Canvas> m_pCanvas;
        Cache<Resource, std::string>* m_pCache;
        std::shared_ptr<Mesh> m_pMesh;

        std::shared_ptr<Font> m_pFont;
        std::shared_ptr<Text> m_pText;
        Player* m_pPlayer = nullptr;

        boost::signals2::scoped_connection m_HealthCon;

        // Functions
        void redraw();
};

#endif
