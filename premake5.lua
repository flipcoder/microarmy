workspace("microarmy")
    targetdir("bin")
    debugdir("bin")
    
    configurations {"Debug", "Release"}

        defines {
            "GLM_FORCE_RADIANS",
            "GLM_ENABLE_EXPERIMENTAL",
            "GLM_FORCE_CTOR_INIT",
            "NOMINMAX",
            "QOR_NO_PYTHON",
            "QOR_NO_CAIRO",
            "QOR_NO_PHYSICS",
            --"QOR_NO_NET",
        }
        
        -- Debug Config
        configuration "Debug"
            defines { "DEBUG" }
            symbols "On"
            linkoptions { }

            configuration "linux"
                links {
                    "z",
                    "bfd",
                    "iberty"
                }
        
        -- Release Config
        configuration "Release"
            defines { "NDEBUG" }
            optimize "Speed"
            floatingpoint "Fast"
            targetname("microarmy_dist")

        -- gmake Config
        configuration "gmake"
            buildoptions { "-std=c++11" }
            -- buildoptions { "-std=c++11", "-pedantic", "-Wall", "-Wextra", '-v', '-fsyntax-only'}
            links {
                "pthread",
                "SDL2_ttf",
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
                "boost_filesystem",
                "boost_coroutine",
                "boost_python",
                "boost_regex",
                "boost_chrono",
                "jsoncpp",
                "raknet",
                --"BulletSoftBody",
                --"BulletDynamics",
                --"BulletCollision",
                --"LinearMath",
            }
            includedirs {
                "lib/Qor/",
                "lib/Qor/lib/kit",
                "/usr/local/include/",
                "/usr/include/bullet/",
                "/usr/include/rapidxml/",
                "/usr/include/raknet/DependentExtensions"
            }

            libdirs {
                "/usr/local/lib"
            }
            
            buildoptions {
                "`python2-config --includes`",
                --"`pkg-config --cflags cairomm-1.0 pangomm-1.4`"
            }

            linkoptions {
                "`python2-config --libs`",
                --"`pkg-config --libs cairomm-1.0 pangomm-1.4`"
            }
            
        configuration "macosx"
            links {
                "boost_thread-mt",
            }
            buildoptions { "-framework OpenGL" }
            linkoptions { "-framework OpenGL" }
            --buildoptions { "-U__STRICT_ANSI__", "-stdlib=libc++" }
            --linkoptions { "-stdlib=libc++" }

        configuration "linux"
            links {
                "GL",
                "boost_thread",
            }

        configuration "windows"
            toolset "v141"
            flags { "MultiProcessorCompile" }

            defines {
                "_WINSOCKAPI_"
            }
            links {
                "ws2_32",
                --"glibmm.dll.lib",
                --"cairomm.dll.lib",
                --"pangomm.dll.lib",
                "SDL2main",
                "OpenGL32",
                "GLU32",
                "SDL2",
                "SDL2_ttf",
                "GLEW32",
                "assimp",
                "freeimage",
                "OpenAL32",
                "alut",
                "libogg",
                "libvorbis",
                "libvorbisfile",
                --"boost_system-vc141-mt-1_67",
                --"boost_filesystem-vc141-mt-1_67",
                --"boost_thread-vc141-mt-1_67",
                "python27",
                --"boost_python-vc141-mt-1_67",
                --"boost_coroutine-vc141-mt-1_67",
                --"boost_regex-vc141-mt-1_67",
                "lib_json",
            }

            includedirs {
                --"c:/Python27/include",
                --"c:/gtkmm/lib/pangomm/include",
                --"c:/gtkmm/lib/sigc++/include",
                --"c:/gtkmm/lib/cairomm/include",
                --"c:/gtkmm/include/pango",
                --"c:/gtkmm/include/pangomm",
                --"c:/gtkmm/include/sigc++",
                --"c:/gtkmm/include",
                --"c:/gtkmm/include/cairo",
                --"c:/gtkmm/lib/glib/include",
                --"c:/gtkmm/include/glib",
                --"c:/gtkmm/lib/glibmm/include",
                --"c:/gtkmm/include/glibmm",
                --"c:/gtkmm/include/cairomm",
                --"c:/gtkmm/include",
                "c:/local/boost_1_67_0",
                "c:/Program Files (x86)/OpenAL 1.1 SDK/include",
                "c:/msvc/include",
            }
            configuration { "windows", "Debug" }
                libdirs {
                    "c:/msvc/lib32/debug"
                }
            configuration "windows"
            includedirs {
                "C:/Python27/include",
            }
            libdirs {
                "c:/Program Files (x86)/OpenAL 1.1 SDK/libs/Win32",
                "c:/msvc/lib32",
                --"c:/gtkmm/lib",
                "C:/local/boost_1_67_0/lib32-msvc-14.1",
                "C:/Python27/libs",
            }
            
            configuration { "windows", "Debug" }
                links {
                    "RakNet_VS2008_LibStatic_Debug_Win32",
                    --"BulletSoftBody_vs2010",
                    --"BulletDynamics_vs2010_debug",
                    --"BulletCollision_vs2010_debug",
                    --"LinearMath_vs2010_debug",
                }
            configuration {}
            configuration { "windows", "Release" }
                links {
                    "RakNet_VS2008_LibStatic_Release_Win32",
                    --"BulletSoftBody_vs2010",
                    --"BulletDynamics_vs2010",
                    --"BulletCollision_vs2010",
                    --"LinearMath_vs2010",
                }

    project "microarmy"
        kind "ConsoleApp"
        language "C++"

        -- Project Files
        files {
            "src/**.h",
            "src/**.cpp",
            "lib/Qor/**.h",
            "lib/Qor/**.cpp",
            "lib/Qor/lib/kit/**.h",
            "lib/Qor/lib/kit/**.cpp"
        }

        -- Exluding Files
        excludes {
            "lib/*~/**",
            "lib/Qor/lib/*~/**",
            "lib/Qor/Qor/Main.cpp",
            "lib/Qor/Qor/Info.cpp",
            "lib/Qor/Qor/DemoState.*",
            "lib/Qor/Qor/addons/**",
            "lib/Qor/Qor/tests/**",
            "lib/Qor/gui/**",
            "lib/Qor/scripts/**",
            "lib/Qor/templates/**",
            "lib/Qor/lib/kit/tests/**",
            "lib/Qor/lib/kit/toys/**",
            "lib/Qor/lib/kit/kit/net/**"
        }
        
        includedirs {
            "lib/Qor",
            "lib/Qor/lib/kit",
            "/usr/local/include/",
            "/usr/include/bullet/",
            "/usr/include/rapidxml/",
            "/usr/include/raknet/DependentExtensions"
        }

