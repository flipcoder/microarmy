#include <iostream>
#include <memory>
#include "kit/kit.h"
#include "Qor/Qor.h"
#include "Info.h"
#include "Game.h"
#include "Intro.h"
#include "Pregame.h"

using namespace std;
using namespace kit;


int main(int argc, char* argv[]) {
    
    Args args(argc, (const char**)argv);
    args.set("mod", PACKAGE);
    args.set("title", "Micro Army");
    
    Texture::DEFAULT_FLAGS = Texture::TRANS | Texture::MIPMAP;
    
#ifndef DEBUG
    try {
#endif
        auto engine = kit::make_unique<Qor>(args);
        engine->states().register_class<Intro>("intro");
        engine->states().register_class<Pregame>("pregame");
        engine->states().register_class<Game>("game");
        engine->run("intro");

#ifndef DEBUG
    } catch (const Error&) {
        // already logged
    } catch (const std::exception& e) {
        LOGf("Uncaught exception: %s", e.what());
    }
#endif
    return 0;
}
