# TMNT IV: Turtles in Time Recomp

Recompilação nativa para PC de **Teenage Mutant Ninja Turtles IV: Turtles in Time** (versão USA), baseada no framework [SNESRecomp](https://github.com/RetroPortingToolKit/snesrecomp).

O projeto recria o jogo para Windows e oferece modo cooperativo local para dois jogadores.

> **É necessário possuir legalmente uma cópia do jogo.** Este repositório e seus lançamentos não incluem dados da ROM. O código recompilado é gerado localmente a partir da ROM do próprio usuário e não é enviado ao repositório.

## Instalador para Windows

[**Baixar o instalador Windows v1.0.2 — Hotkeys e Save Game**](https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp/releases/download/v1.0.2/TMNT-IV-Turtles-in-Time-PC-Setup-v1.0.2.exe)

Consulte as [notas e o checksum da versão 1.0.2](installer/RELEASE-v1.0.2.md). A [v1.0.1](https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp/releases/tag/v1.0.1) e a [v1.0.0](https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp/releases/tag/v1.0.0) continuam disponíveis.

O instalador requer Windows 10 1903 ou superior, 64 bits, e uma ROM USA original do usuário. A ROM não acompanha o instalador; o assistente de instalação valida e importa o arquivo selecionado.

Para instruções de instalação, importação da ROM, compilação e publicação, consulte [installer/README.md](installer/README.md). A ROM validada fica em LocalAppData, fora da pasta de instalação.

## Configurações gráficas

O jogo inicia em tela cheia, preenchendo a tela widescreen. Isso estica a imagem SNES 4:3 para preencher 16:9; no menu de gráficos, desative **WIDE FILL / PREENCHER** para preservar a proporção 4:3 sem distorção.

O modo HD usa upscale pela GPU e começa com o filtro **Scale 2x**. Ele melhora a apresentação dos pixels, mas não inclui texturas remasterizadas. Pacotes HD externos são opcionais e devem conter recursos autorais ou autorizados.

- **F6** ou **Start** em qualquer controle alterna instantaneamente entre Original e HD. Start continua funcionando normalmente no jogo.
- **F10** abre o menu de gráficos, onde é possível escolher Original/HD, filtro, escala, proporção, VSync, janela ou tela cheia e pacote HD.
- A comparação em tela dividida foi removida.

## Atalhos de salvamento

- **F1** ou **LT/L2**: Quick Save.
- **F2** ou **RT/R2**: Quick Load.
- **Ctrl+L** abre o launcher durante o jogo; em **Settings > Hotkeys** é possível reconfigurar os atalhos de teclado.
- Os saves ficam em `%LOCALAPPDATA%\TMNTIVRecomp\saves\`.

O sistema oferece cinco slots manuais e um slot rápido. Os estados são vinculados à identidade da ROM e à versão do formato. Consulte o [relatório de validação de saves](SAVE-GAME-REPORT.md) para os resultados e limitações dos testes.

## Situação do projeto

O instalador Windows e a importação/validação da ROM estão implementados. A validação automatizada cobre identidade da ROM, instalação, remoção, preservação de dados e inicialização após reinstalação. Veja [os resultados dos testes](installer/TEST-RESULTS.md) e seus limites: validar a inicialização não comprova compatibilidade completa de gameplay.

## Identidade da ROM

| Campo | Valor |
|---|---|
| Arquivo | `Teenage Mutant Ninja Turtles IV - Turtles in Time (USA).sfc` |
| Publicadora | Konami |
| Desenvolvedora | Konami |
| Ano | 1992 |
| Mapeamento | lorom |
| Região | USA (América do Norte) |
| Coprocessador | nenhum |
| CRC32 | `5940bd99` |
| SHA-256 | `5b82cdd6f2da56f43680d6a5021faebe2e06036d30602c1a7917aa414cf8b5f4` |

`tools/regen.sh` recusa qualquer ROM que não corresponda a essa identidade.

## Compilação

```sh
git submodule update --init --recursive
bash tools/regen.sh --rom "/caminho/para/Teenage Mutant Ninja Turtles IV - Turtles in Time (USA).sfc"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

A ROM pode ficar fora do repositório. Defina `SNESRECOMP_ROM` para informar o caminho. Sem essa variável, `tools/regen.sh` procura o arquivo na raiz do projeto e, depois, no caminho gravado em `rom.cfg`. O `.gitignore` impede que a ROM e esse arquivo local sejam enviados ao Git.

O script valida a ROM, gera `src/gen/*.c` e sincroniza `recomp/funcs.h`. Execute-o novamente ao alterar arquivos em `recomp/`.

## Execução

```sh
./build/TmntIvTurtlesInTimeSNESRecomp
./build/TmntIvTurtlesInTimeSNESRecomp "/caminho/para/Teenage Mutant Ninja Turtles IV - Turtles in Time (USA).sfc"
```

Sem caminho de ROM na linha de comando, o launcher abre um seletor com verificação da ROM e configurações de vídeo, áudio e controles. A escolha fica salva em `rom.cfg` ao lado do executável. Se o launcher não estiver incluído na compilação (`--no-recomp-ui`), o host usa o seletor de arquivos nativo.

A ROM é conferida pelos hashes acima antes da execução. Uma cópia incompatível é recusada.

## Estrutura do repositório

| Caminho | Conteúdo |
|---|---|
| `recomp/` | Entradas de análise: `bank*.cfg`, `symbols.toml` e `funcs.h` gerado |
| `rom_identity.txt` | Identidade da ROM usada pelo build, pelo script de geração e pelo CI |
| `src/` | Código do host específico deste projeto |
| `src/gen/` | Código C gerado localmente; não é enviado ao repositório |
| `snesrecomp/` | Submódulo do framework SNESRecomp |
| `recomp-ui/` | Submódulo da interface do launcher |
| `tools/` | Script `regen.sh`, que gera o código a partir da ROM |
| `scripts/` | Ferramentas de empacotamento |
| `framework_pins.txt` | Revisões do framework usadas como base pelo projeto |

## Diretrizes para a recompilação

1. O driver de quadros específico do jogo está em `src/game_rtl.c` e usa o driver alinhado ao feixe de vídeo do framework.
2. Identifique rotinas em `recomp/symbols.toml` e execute `tools/regen.sh` para sincronizar a análise.
3. Resolva destinos indiretos não identificados antes de promover funções para recompilação AOT.
4. Não invente resultados para contornar código não analisado e não edite `src/gen/` manualmente. Corrija a configuração ou o framework e gere novamente.

## Multijogador

Dois jogadores locais, um controle por jogador.

## Licença

O código deste projeto segue a licença em [LICENSE](LICENSE). O framework possui seus próprios termos; consulte `snesrecomp/LICENSE` e `snesrecomp/THIRD_PARTY_ATTRIBUTION.md`. Essas licenças não abrangem os dados do jogo, que não são distribuídos aqui.
