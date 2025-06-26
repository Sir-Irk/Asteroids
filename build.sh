warnings="-Wall -Werror -Wno-missing-braces -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable"
flags="-ffast-math -flto"
asm="-S -masm=intel"
#flags=""
echo "Building..."
clang -O3 -g $1 -lraylib -lm -o build/$2  $flags $warnings -march=native e
#clang $asm -O3 $1 $flags $warnings -march=native -mtune=native
