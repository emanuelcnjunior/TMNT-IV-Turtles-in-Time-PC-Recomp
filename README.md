# TMNT IV: Turtles in Time — PC Recomp

[**🇧🇷 Português**](README.md) | [**🇺🇸 English**](README.en.md)

Port para Windows de **Teenage Mutant Ninja Turtles IV: Turtles in Time** (SNES, versão USA), desenvolvido com o [SNESRecomp](https://github.com/emanuelcnjunior/snesrecomp-tmnt4). Oferece jogo local para até **dois jogadores** e um instalador para Windows com validação da ROM.

> **A ROM não está incluída.** Você deve fornecer sua própria cópia da versão USA compatível. O projeto não é afiliado à Konami, Nintendo ou aos detentores dos direitos de Teenage Mutant Ninja Turtles.

## ⬇️ Baixar e instalar

**[Baixar o instalador para Windows — v1.0.2](https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp/releases/download/v1.0.2/TMNT-IV-Turtles-in-Time-PC-Setup-v1.0.2.exe)**

**Requisitos:** Windows 10 (1903+) ou mais recente, **64 bits**, driver gráfico compatível com OpenGL e uma ROM USA válida. Para instalar e jogar, não é necessário instalar Git, Python, Rust, CMake ou Visual Studio.

1. Baixe e execute o instalador.
2. Quando solicitado, selecione **sua própria ROM USA** do jogo.
3. O instalador verifica o arquivo e o importa para a pasta de dados do usuário.
4. Abra o jogo pelo atalho criado e ajuste vídeo, áudio e controles no launcher.

O nome de origem do arquivo pode ser diferente: **o conteúdo é verificado por SHA-256**, e uma cópia válida é armazenada com o nome padronizado. ROMs de outras regiões, modificadas ou com cabeçalho de copiador não são aceitas.

[Releases anteriores](https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp/releases) · [Notas e checksums da v1.0.2](installer/RELEASE-v1.0.2.md) · [Guia detalhado do instalador](installer/README.md)

## 🎮 Controles e recursos

| Ação | Teclado | Controle |
|---|---|---|
| Salvamento rápido (Quick Save) | **F1** | **LT / L2** |
| Carregamento rápido (Quick Load) | **F2** | **RT / R2** |
| Lista de saves | **F11** | Consulte o mapeamento no launcher |
| Alternar Original / HD | **F6** | **Start** (também mantém a função Start do jogo) |
| Configurações gráficas | **F10** | Via interface |
| Abrir o launcher | **Ctrl + L** | — |

Os atalhos do teclado podem ser configurados em **Settings > Hotkeys**. Os comandos disponíveis podem depender da configuração do controle.

### 💾 Save Game

O port dispõe de **cinco slots manuais** e **um Quick Save**. São *save states* (fotografias do estado de execução), não saves nativos em SRAM do cartucho.

Os arquivos ficam em:

```text
%LOCALAPPDATA%\TMNTIVRecomp\saves\
```

Os estados possuem verificações de integridade e compatibilidade. **Uma atualização do runtime pode tornar saves antigos incompatíveis**; faça backup antes de atualizar. Consulte o [relatório técnico de saves](SAVE-GAME-REPORT.md) e a [auditoria da interface](SAVE-UI-AUDIT.md) para os testes e limitações.

### 🖼️ Gráficos Original e HD

- **Original:** mantém a apresentação clássica do SNES.
- **HD:** aplica upscale/filtros pela GPU (o filtro inicial é **Scale 2x**).
- **F10:** permite selecionar filtro, escala, proporção, VSync, janela/tela cheia e pacote HD.
- A opção **WIDE FILL / PREENCHER** preenche telas widescreen, mas **estica a imagem 4:3**. Desative-a para preservar a proporção.

**Importante:** o modo HD atual melhora o escalonamento; **não significa que os sprites, tiles e cenários tenham sido remasterizados manualmente**. Pacotes HD opcionais devem usar recursos autorais ou autorizados.

## ✅ Estado e testes

O instalador e a validação da ROM estão implementados. A documentação registra testes de instalação, importação, integridade e inicialização, além de testes específicos do sistema de save. **Isso não equivale à homologação de uma campanha completa em todas as máquinas.** Há verificações manuais e de integração pendentes descritas nos relatórios.

- [Testes do instalador](installer/TEST-RESULTS.md)
- [Relatório de Save Game](SAVE-GAME-REPORT.md)
- [Notas da versão 1.0.2](installer/RELEASE-v1.0.2.md)

Encontrou um defeito? Abra uma [issue](https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp/issues) informando a versão, o Windows, os passos para reproduzir e, se possível, um log **sem anexar ROMs ou dados extraídos do jogo**.

## 🧩 ROM compatível

| Propriedade | Valor |
|---|---|
| Jogo | Teenage Mutant Ninja Turtles IV: Turtles in Time |
| Região | USA / North America |
| Formato | LoROM, `.sfc`, sem cabeçalho |
| Nome de destino | `Teenage Mutant Ninja Turtles IV - Turtles in Time (USA).sfc` |
| Tamanho | 1.048.576 bytes |
| CRC32 | `5940BD99` |
| SHA-256 | `5b82cdd6f2da56f43680d6a5021faebe2e06036d30602c1a7917aa414cf8b5f4` |

A ROM validada é armazenada em `%LOCALAPPDATA%\TMNTIVRecomp\`. A desinstalação preserva essa pasta, incluindo configurações e saves. Excluí-la manualmente **apaga também a cópia importada da ROM**.

## 🛠️ Compilar a partir do código-fonte

O caminho abaixo é para **desenvolvedores**, não para quem usa o instalador. Exige as ferramentas de build descritas no [guia de instalação](installer/README.md) e uma ROM compatível obtida pelo próprio desenvolvedor.

```bash
git clone --recurse-submodules https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp.git
cd TMNT-IV-Turtles-in-Time-PC-Recomp
bash tools/regen.sh --rom "/c/caminho/para/sua-rom-usa.sfc"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

O comando acima utiliza um shell Bash para a regeneração; no Windows, compile em ambiente configurado com MSVC/CMake. O código gerado em `src/gen/` não é versionado, e **a ROM nunca deve ser incluída em commits**.

Para gerar o instalador, consulte [installer/README.md](installer/README.md). O projeto principal usa os submódulos [snesrecomp-tmnt4](https://github.com/emanuelcnjunior/snesrecomp-tmnt4) e [recomp-ui](https://github.com/emanuelcnjunior/recomp-ui).

### Estrutura principal

| Diretório / arquivo | Descrição |
|---|---|
| `src/` | Código do port e integração com o host |
| `recomp/` | Configuração e símbolos da recompilação |
| `src/gen/` | Código gerado localmente a partir da ROM (não versionado) |
| `snesrecomp/` | Framework SNESRecomp (submódulo) |
| `recomp-ui/` | Launcher (submódulo) |
| `installer/` | Scripts, instruções e testes do instalador |
| `tools/regen.sh` | Verificação da ROM e geração do código |
| `rom_identity.txt` | Identidade da ROM aceita |

## 📄 Licença e direitos

O código próprio do projeto segue [LICENSE](LICENSE). Dependências e submódulos possuem licenças próprias. **A licença do código não concede direitos sobre o jogo original, seus personagens, áudio, sprites ou ROM.** Nenhuma ROM é oferecida aqui.
