A minimal asteroids game in C using Raylib and Emscripten for wasm.

Play here: https://sir-irk.itch.io/asteroids

Minimal C compilation(dynamic linking with raylib)
```
clang src/asteroids.c -o asteroids -lraylib -lm
```

The `emcc` command I used for the itch.io page
```
emcc -g -o game.html src/asteroids.c -Os -Wall web/libraylib.a -I. -Isrc/ -L. -Lweb/  -s USE_GLFW=3 -s --shell-file minshell.html -DPLATFORM_WEB --preload-file sounds -sASSERTIONS -s 'EXPORTED_RUNTIME_METHODS=["HEAPF32"]'
```

More info about using Raylib for the web https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)

Sound effects were made with https://raylibtech.itch.io/rfxgen
