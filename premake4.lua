solution("microarmy")
    targetdir("bin")
    
    configurations {"Debug", "Release"}

        -- Debug Config
        configuration "Debug"
            defines { "DEBUG", "BACKWARD_HAS_BFD=1", "GLM_FORCE_RADIANS" }
            flags { "Symbols" }
            links {
                "z",
                "bfd",
                "iberty"
            }
            linkoptions { "`llvm-config --libs core` `llvm-config --ldflags`" }

        -- Release Config
        configuration "Release"
            defines { "NDEBUG" }
            flags { "OptimizeSpeed" }
            targetname("microarmy_dist")

        -- gmake Config
        configuration "gmake"
            buildoptions { "-std=c++11" }
            -- Uncomment the following line to get in depth debugging 
            --buildoptions { "-std=c++11", "-pedantic", "-Wall", "-Wextra" }

        -- OS X Config
        configuration "macosx"
            buildoptions { "-U__STRICT_ANSI__", "-stdlib=libc++" }
            linkoptions { "-stdlib=libc++" }


    project "microarmy"
        kind "WindowedApp"
        language "C++"
        links {
            "pthread",
            "GL",
            "GLU",
            "SDL2",
            "GLEW",
            "assimp",
            "freeimage",
            "openal",
            "alut",
            "ogg",
            "vorbis",
            "vorbisfile",
            "boost_system",
            "boost_thread",
            "boost_filesystem",
            "boost_python",
            "boost_coroutine",
            "boost_regex",
            "jsoncpp",
            "BulletSoftBody",
            "BulletDynamics",
            "BulletCollision",
            "LinearMath",
            "z",
            "RakNetDLL"
        }

        -- Project Files
        files {
            "src/**.h",
            "src/**.cpp",
            -- "lib/**.h",
            -- "lib/**.cpp"
        }

        -- Exluding Files
        excludes {
            "src/Qor/Main.cpp",
            "src/Qor/Info.cpp",
            "src/Qor/DemoState.*",
            "src/Qor/tests/**",
            "src/Qor/scripts/**",
            "src/Qor/addons/**",
            "src/Qor/shaders/**",
            "lib/Qor/src/Main.cpp",
            "lib/Qor/src/Info.cpp",
            "lib/Qor/src/DemoState.*",
            "lib/Qor/src/tests/**",
            "lib/Qor/src/scripts/**",
            "lib/Qor/src/addons/**",
            "lib/Qor/src/shaders/**"
        }

        includedirs {
            "vendor/include/",
            "/usr/local/include/",
            "/usr/include/bullet/",
            "/usr/include/raknet/DependentExtensions"
        }

        libdirs {
            -- "lib",
            "/usr/local/lib",
            "/usr/local/lib64/",
        }
        
        buildoptions {
            "`python2-config --includes`",
            "`pkg-config --cflags cairomm-1.0 pangomm-1.4`"
        }

        linkoptions {
            "`python2-config --libs`",
            "`pkg-config --libs cairomm-1.0 pangomm-1.4`"
        }
