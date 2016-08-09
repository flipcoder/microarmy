solution("microarmy")
    targetdir("bin")
    
    configurations {"Debug", "Release"}

        -- Debug Config
        configuration "Debug"
            defines { "DEBUG", "BACKWARD_HAS_BFD=1", "GLM_FORCE_RADIANS", "QOR_NO_PYTHON", "QOR_NO_PHYSICS" }
            flags { "Symbols" }
            linkoptions { }

            configuration "gmake"
                links {
                    "z",
                    "bfd",
                    "iberty"
                }

        -- Release Config
        configuration "Release"
            defines { "NDEBUG" }
            flags { "OptimizeSpeed" }
            targetname("microarmy_dist")

        -- gmake Config
        configuration "gmake"
            buildoptions { "-std=c++11" }
            -- Uncomment the following line to get in depth debugging 
            -- buildoptions { "-std=c++11", "-pedantic", "-Wall", "-Wextra", '-v', '-fsyntax-only'}
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
                "RakNetDLL",
            }
            includedirs {
                "lib/Qor/",
                "lib/Qor/lib/kit",
                "/usr/local/include/",
                "/usr/include/bullet/",
                "/usr/include/raknet/DependentExtensions"
            }

            libdirs {
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
        -- OS X Config
        configuration "macosx"
            buildoptions { "-U__STRICT_ANSI__", "-stdlib=libc++" }
            linkoptions { "-stdlib=libc++" }

        configuration "windows"
            links {
                "ws2_32",
                "glibmm.dll.lib",
                "cairomm.dll.lib",
                "pangomm.dll.lib",
                "SDL2main",
                "OpenGL32",
                "GLU32",
                "SDL2",
                "GLEW32",
                "assimp",
                "freeimage",
                "openal32",
                "alut",
                "libogg",
                "libvorbis",
                "libvorbisfile",
                "boost_system-vc140-mt-1_61",
                "boost_thread-vc140-mt-1_61",
                "boost_python-vc140-mt-1_61",
                "boost_coroutine-vc140-mt-1_61",
                "boost_regex-vc140-mt-1_61",
                "lib_json",
                "BulletSoftBody_vs2010",
                "BulletDynamics_vs2010",
                "BulletCollision_vs2010",
                "LinearMath_vs2010",
            }

            includedirs {
                "c:/Python27/include",
                "c:/gtkmm/lib/pangomm/include",
                "c:/gtkmm/lib/sigc++/include",
                "c:/gtkmm/lib/cairomm/include",
                "c:/gtkmm/include/pango",
                "c:/gtkmm/include/pangomm",
                "c:/gtkmm/include/sigc++",
                "c:/gtkmm/include",
                "c:/gtkmm/include/cairo",
                "c:/gtkmm/lib/glib/include",
                "c:/gtkmm/include/glib",
                "c:/gtkmm/lib/glibmm/include",
                "c:/gtkmm/include/glibmm",
                "c:/gtkmm/include/cairomm",
                "c:/gtkmm/include",
                "c:/local/boost_1_61_0",
                "c:/Program Files (x86)/OpenAL 1.1 SDK/include",
                "c:/msvc/include",
            }
            libdirs {
                "c:/msvc/lib32/debug",
                "c:/Program Files (x86)/OpenAL 1.1 SDK/libs/Win32",
                "c:/msvc/lib32",
                "c:/gtkmm/lib",
                "c:/local/boost_1_61_0/lib32-msvc-14.0",
            }
            -- buildoptions {
                -- "/MP",
                -- "/Gm-",
            -- }
            
            configuration { "Debug" }
                links {
                    "libboost_filesystem-vc140-mt-gd-1_61",
                    "RakNet_VS2008_LibStatic_Debug_Win32",
                }
            configuration { "Release" }
                links {
                    "libboost_filesystem-vc140-mt-1_61",
                    "RakNet_VS2008_LibStatic_Release_Win32",
                }

    project "microarmy"
        kind "WindowedApp"
        language "C++"

        -- Project Files
        files {
            "src/**.h",
            "src/**.cpp",
            "lib/Qor/Qor/**.h",
            "lib/Qor/Qor/**.cpp",
            "lib/Qor/lib/kit/**.h",
            "lib/Qor/lib/kit/**.cpp"
        }

        -- Exluding Files
        excludes {
            "lib/Qor/Qor/Main.cpp",
            "lib/Qor/Qor/Info.cpp",
            "lib/Qor/Qor/DemoState.*",
            "lib/Qor/Qor/tests/**",
            "lib/Qor/Qor/scripts/**",
            "lib/Qor/Qor/addons/**",
            "lib/Qor/lib/kit/tests/**",
            "lib/Qor/lib/kit/toys/**"
        }
        
        includedirs {
            "lib/Qor/",
            "lib/Qor/lib/kit",
            "/usr/local/include/",
            "/usr/include/bullet/",
            "/usr/include/raknet/DependentExtensions"
        }

    project "test"
        kind "ConsoleApp"
        language "C++"

        -- Project Files
        files {
            "src/**.h",
            "src/**.cpp",
            "lib/Qor/Qor/**.h",
            "lib/Qor/Qor/**.cpp",
            "lib/Qor/lib/kit/**.h",
            "lib/Qor/lib/kit/**.cpp"
        }

        -- Exluding Files
        excludes {
            "src/Main.cpp", -- run test suite instead
            
            "lib/Qor/Qor/Main.cpp",
            "lib/Qor/Qor/Info.cpp",
            "lib/Qor/Qor/DemoState.*",
            --"lib/Qor/Qor/tests/**", -- don't exclude test suite
            "lib/Qor/Qor/scripts/**",
            "lib/Qor/Qor/addons/**",
            "lib/Qor/lib/kit/tests/**",
            "lib/Qor/lib/kit/toys/**"
        }
        
        includedirs {
            "lib/Qor/",
            "lib/Qor/lib/kit",
            "/usr/local/include/",
            "/usr/include/bullet/",
            "/usr/include/raknet/DependentExtensions"
        }

