# Simple game made with opengl.
Controls:

  [WASD] - move
  
  [Space] - jump
  
  [Esc] = pause/unpause

# Prerequisites
Cmake 3.8 or higher

C++ standard used - c++ 23

# How to build
Open cmd.

Run command ```cd <path to repository>```

Create build folder with ```mkdir build```

Go to that folder ```cd build```

Run cmake build ```cmake build ..```


Primary executable is in the folder "app".

```Prebuilt binaries for windows can be found here:``` https://github.com/CodeForBeauty/AstronautGame/releases/tag/Test-build

# Libraries used
glfw/glad - graphics api

openal/libsnd - audio api

imgui - debugging UI api

assimp - 3d model loading

stb_image - textures loading

freetype - font loading
