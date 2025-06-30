A basic asteroids game in C using [Raylib](https://www.raylib.com/) and Emscripten for wasm.

Play here: https://sir-irk.itch.io/asteroids

Original version looked like this and the code is on the branch [original](https://github.com/Sir-Irk/Asteroids/tree/original). I've since added bloom and other stuff for fun.
![Til](https://github.com/Sir-Irk/Asteroids/blob/main/demo.gif)

Minimal C compilation(dynamic linking with raylib)
```
clang src/asteroids.c -o asteroids -lraylib -lm
```

The `emcc` command I used for the itch.io page
```
emcc -g -o index.html src/asteroids.c -Os -Wall web/libraylib.a -I. -Isrc/ -L. -Lweb/  -s USE_GLFW=3 -s --shell-file minshell.html -DPLATFORM_WEB --preload-file sounds --preload-file shaders -sASSERTIONS -s 'EXPORTED_RUNTIME_METHODS=["HEAPF32"]' -sFULL_ES3=1   
```
Note: to work on itch.io, you need to rename `game.html` to `index.html` and then make a zip containing `index.html`, `game.wasm`, `game.data`, `game.js` and upload the `.zip` to itch.io

More info about using Raylib for the web https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)

Sound effects were made with https://raylibtech.itch.io/rfxgen
