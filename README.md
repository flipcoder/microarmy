=====

# Micro Army

A cross between Contra and Super Meat Boy

Developed for 1-Game-A-Month Game Jam (1GAM) during June 2016

Copyright &copy; 2016 Grady O'Connell

![microarmy](https://pbs.twimg.com/media/CkmOJYZUoAQVBDj.jpg:large)

## Instructions

### Releases

No binary releases yet!  Check back soon!

### Build Scripts

[ArchLinux, Antergos, Manjaro, etc.](https://gist.github.com/flipcoder/041e534a1aa482ff1fd5cece57052f92)

### Manual Installation

- Initialize the submodules and checkout the master branches:

```
git submodule update --init --recursive
git submodule foreach --recursive git checkout master
```

- Get the libraries:

```
Vorbis, Boost, Jsoncpp, Pthread, Bullet, Sdl2, Raknet, Glew, Assimp, Freeimage, Glm, Freealut, Rapidxml, Openal, Cairomm, Ogg, Pangomm, Catch, Backward-cpp
```

- Build with [premake4](http://industriousone.com/premake/download):

```
premake4 gmake
make microarmy config=release -j`nproc`
```

- Run microarmy_dist from the bin folder

## Credits

### [Grady O'Connell](https://github.com/flipcoder)
Programming, Graphics, Music

### [Kevin Nelson](https://github.com/mrgirlyman)
Programming

### [Mark McDaniel](https://github.com/AlfredAnonymous)
Graphics

=====

