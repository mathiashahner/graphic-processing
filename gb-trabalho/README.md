# Coloque o lixo na lixeira

Trabalho desenvolvido para a disciplina **Processamento Gráfico**.

## 👥 Integrantes do Grupo

- Guilherme Costa Tarrasconi
- Mathias Daniel Hahner

## 🚀 Como executar

Este programa foi feito em **C++** com **OpenGL**, **GLFW** e **GLEW**. O projeto já inclui as bibliotecas necessárias na pasta `libs/`.

### Windows

```bash
# build
g++ -o main.exe main.cpp gl_utils.cpp maths_funcs.cpp -I. -Ilibs/include -Llibs/lib -lglfw3 -lglew32 -lopengl32 -lgdi32

# run
.\main.exe
```

### Linux

```bash
# build
g++ -o main main.cpp gl_utils.cpp maths_funcs.cpp -lGLEW -lglfw -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lm

# run
./main
```

## 💡 Sobre o Programa

O jogo apresenta um cenário em tilemap isométrico em que o jogador precisa levar o lixo até a lixeira sem tocar na água ou nos veículos em movimento. Se houver colisão com um carro ou com uma área não caminhável, o jogador perde. Ao alcançar a lixeira, vence a partida.

## 📌 Observações

- O mapa é carregado do arquivo `assets/tileset.tmap`.
- Os sprites e o tileset ficam na pasta `assets/`.
- Durante o jogo, use `W`, `A`, `S`, `D` ou as setas para mover o personagem.
- Pressione `R` para reiniciar e `ESC` para sair.
