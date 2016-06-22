#ifndef HUD_H_KW10MMV6
#define HUD_H_KW10MMV6

#include <boost/circular_buffer.hpp>
#include "Qor/Window.h"
#include "Qor/Canvas.h"
#include "Qor/Input.h"

class HUD: public Node {
    public:

        HUD(
            Window* window,
            Input* input,
            Cache<Resource,std::string>* cache
        );
        virtual ~HUD() {}

        virtual void logic_self(Freq::Time) override;

        void set(int star_lev, int stars, int max_stars);
        
    private:
        
        void redraw();

        Window* m_pWindow = nullptr;
        Input* m_pInput = nullptr;
        std::shared_ptr<Canvas> m_pCanvas;
        Cache<Resource, std::string>* m_pCache;
        Pango::FontDescription m_FontDesc;
        std::shared_ptr<Mesh> m_pMesh;

        bool m_bDirty = true;

        int m_StarLev = -1;
        int m_Stars = 0;
        int m_MaxStars = 0;

        static const std::vector<int> STAR_LEVELS;
};

#endif
