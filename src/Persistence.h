#include "Qor/Session.h"

class Persistence:
    public Session::IModule
{
    public:

        virtual ~Persistence() {}

        std::vector<int> stars;
        std::vector<int> max_stars;
};

