gcc glyphgen.c -o glyphgen.exe
gcc main.c gg_read.c -I"./lib/GLFW/include" -L"./lib/GLFW/bin" -luser32 -lglfw3 -lopengl32 -lgdi32 -o main.exe 