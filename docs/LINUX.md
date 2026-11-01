# Linux x86-64 (experimental)

O pacote `TMNT-IV-Turtles-in-Time-PC-v1.0.2-linux-x86_64.tar.gz` contém o
executável Linux já compilado, os recursos do launcher e as licenças. Não
contém ROM, código gerado, ferramentas de compilação nem os fontes privados
do framework. O jogador não precisa recompilar o jogo.

## Usar

1. Extraia o arquivo inteiro em uma pasta gravável pelo seu usuário.
2. Execute `./run.sh` dentro da pasta extraída.
3. Selecione sua própria ROM USA compatível no launcher e inicie o jogo.

Requisitos: Linux x86-64, glibc 2.35 ou mais recente, libstdc++ com
GLIBCXX_3.4.29 e driver OpenGL. Ubuntu 22.04 é a base de compatibilidade.
O SDL3 está ligado estaticamente ao executável. Outras distribuições e
Steam Deck ainda precisam de testes específicos.

O script `run.sh` fixa a pasta de execução para que configurações e saves
usem a pasta extraída. Mantenha os arquivos juntos e preserve `saves/` ao
atualizar. Não presuma compatibilidade de save states Windows/Linux.

F1/F2 fazem Quick Save/Load; F11 abre os saves; F6 alterna Original/HD;
F10 abre as opções gráficas. A campanha, áudio e controles ainda precisam
de validação manual em Linux.

## Compilar em Linux

Inicialize os submódulos e instale CMake, Ninja, compiladores C/C++, Python 3,
Rust (versão de `snesrecomp/recompiler-rs/rust-toolchain.toml`) e as dependências
gráficas listadas em `.github/workflows/build-linux.yml`.

```bash
python3 snesrecomp/tools/build_native_analyzer.py
bash tools/regen.sh --rom "/caminho/sua-rom-usa.sfc"
cmake -S . -B build-linux-game -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DSNESRECOMP_SETUP_HOST=OFF
cmake --build build-linux-game --parallel 2
cd build-linux-game
./TmntIvTurtlesInTimeSNESRecomp
```

## Compilar Linux a partir do Windows

A ROM é verificada e o código é gerado localmente. A compilação cruzada usa
Clang 22.1.8 para Windows, o sysroot Ubuntu e o SDL3 estático da toolchain
portátil `RetroPortingToolKit/RetroPorting-Toolchains`, versão `v1.0.14`.

Em `build-linux-tools/`, extraia `bin/` da toolchain Windows em `windows/`,
e `sysroot/`, `deps/` e `lib/clang/` da toolchain Linux em `linux/`.
Verifique os arquivos ZIP por SHA-256 antes de extrair:

| ZIP | SHA-256 |
|---|---|
| `cmake-clang-v1-windows-x64.zip` | `28da9742385e7ff875b3d9311e8ed89dbdc84f27b6ecba2bc0d0acc11f6d2b4d` |
| `cmake-clang-v1-linux-x64.zip` | `597c8d343a3cf02ba6f6b2ae7cf6fe2fef125dde8feff62a144e8dd3da3d484e` |

Depois de gerar `src/gen/` com a sua ROM, execute em PowerShell:

```powershell
cmake -S . -B build-linux-game -G Ninja `
  "-DCMAKE_MAKE_PROGRAM=$PWD/build-linux-tools/windows/bin/ninja.exe" `
  "-DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/linux-cross-windows.cmake" `
  '-DCMAKE_BUILD_TYPE=Release' '-DSNESRECOMP_SETUP_HOST=OFF' `
  '-DSNESRECOMP_SDL3_FETCH=OFF'
cmake --build build-linux-game --parallel 4
python scripts/package_linux.py
```

O empacotador valida o executável ELF x86-64, as fontes/imagens obrigatórias
do launcher e a ausência de ROM, código-fonte, caches e saves. Produz o TAR.GZ,
um manifesto por arquivo e um checksum SHA-256 em `dist/`.

O workflow `Verify playable Linux package` baixa o pacote de uma release em
rascunho, verifica o checksum e manifesto, resolve as bibliotecas e abre o
launcher em um display virtual Ubuntu 22.04, capturando sua janela. Nenhuma
ROM nem fonte privado é enviado ao runner para esse teste.

## Pacote alternativo de setup

O workflow `Build Linux setup pack` também pode compilar apenas o setup host,
sem usar ROM. Ele usa o secret `LINUX_FRAMEWORK_READ_KEY`, uma deploy key
somente de leitura do framework privado, respeitando o commit do submódulo.

Execuções por push apenas compilam e verificam o launcher. A opção manual
`include_sources=true` gera o ZIP com fontes e recompilador para o jogador
compilar com sua ROM. Essa alternativa exige autorização para distribuir os
fontes privados; não foi usada para o pacote jogável descrito acima.
