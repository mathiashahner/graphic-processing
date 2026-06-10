### Integrantes:
- Guilherme Costa Tarrasconi
- Mathias Daniel Hahner

### Comandos para rodar:

- No Windows:
```bash
# build
g++ -o main.exe main.cpp gl_utils.cpp maths_funcs.cpp -I. -Ilibs/include -Llibs/lib -lglfw3 -lglew32 -lopengl32 -lgdi32

# run
.\main.exe
```

- No Linux:
```bash
# build
g++ -o main main.cpp gl_utils.cpp maths_funcs.cpp -lGLEW -lglfw -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lm

# run
./main
```
