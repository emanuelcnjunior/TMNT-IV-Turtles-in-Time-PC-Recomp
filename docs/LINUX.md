# Linux x86-64 (pacote de setup experimental)

O workflow `Build Linux setup pack` gera um ZIP Linux nativo com o launcher,
os fontes e o recompilador nativo. A ROM e o código gerado a partir dela não
fazem parte do ZIP. Este pacote usa o fluxo de setup do framework; não é o
instalador Windows nem um executável jogável já recompilado.

## Usar o pacote

1. Extraia o ZIP inteiro em uma pasta gravável pelo seu usuário.
2. Execute `./TmntIvTurtlesInTimeSNESRecomp` dentro dessa pasta.
3. Selecione sua ROM USA compatível e use o fluxo **Generate & rebuild**.
4. Aguarde a geração e a compilação local; o launcher reinicia com o jogo.

O setup pode baixar a toolchain no primeiro uso, portanto requer internet.
Mantenha os arquivos do pacote juntos. Configurações e a pasta `saves/`
ficam na pasta de execução; abra sempre o jogo a partir da pasta extraída.
Não presuma compatibilidade entre save states Windows e Linux.

O build é feito no Ubuntu 22.04 x86-64 (glibc 2.35). É necessário um ambiente
gráfico Linux e driver OpenGL. Outras distribuições e Steam Deck ainda
precisam de testes. O teste automatizado abre o launcher em um display
virtual e valida as bibliotecas, mas não testa uma partida, áudio ou controle.

## Compilar uma versão jogável localmente

Com os submódulos inicializados, instale CMake, Ninja, compiladores C/C++,
Python 3, a toolchain Rust indicada em `snesrecomp/recompiler-rs/rust-toolchain.toml`
e os pacotes de desenvolvimento OpenGL/SDL listados no workflow.

```bash
python3 snesrecomp/tools/build_native_analyzer.py
bash tools/regen.sh --rom "/caminho/sua-rom-usa.sfc"
cmake -S . -B build-linux-game -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux-game --parallel 2
cd build-linux-game
./TmntIvTurtlesInTimeSNESRecomp
```

Para produzir somente o pacote de setup, use um checkout sem `src/gen/*.c`:

```bash
cmake -S . -B build-linux -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DSNESRECOMP_SETUP_HOST=ON -DCMAKE_BUILD_RPATH='$ORIGIN'
cmake --build build-linux --parallel 2
bash scripts/package_release.sh build-linux linux-x64
```

O ZIP aparece em `dist/`. O workflow também gera `SHA256SUMS-linux.txt`.
