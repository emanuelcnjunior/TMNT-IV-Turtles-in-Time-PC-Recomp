# Etapa 3 — apresentação HD (em validação)

Branch `feature/hd-renderer`, derivada da implementação local de Save Game
equivalente à distribuição 1.0.1. A distribuição aprovada permanece intacta.

## Arquitetura

A PPU rasteriza sprites/OAM e BG1–BG4 a partir de tiles planares e CGRAM na
CPU. O campo visível do TMNT IV é 256×224. As camadas resolvem prioridades,
janelas, transparência, color math e brilho antes da apresentação. Existem
caminhos para mosaic, tiles grandes e Mode 7 no framework. Não se alterou a
resolução lógica, os ciclos, as colisões, o Beam Frame Driver nem o NMI.

SDL3 permanece responsável pela janela, eventos, áudio e controles. Este
perfil utiliza o presenter OpenGL 3.3, com um upload do framebuffer por desenho.
O presenter SDL convencional permanece disponível aos outros projetos.

Original usa nearest; HD oferece Sharp (mistura convexa sem overshoot),
Scale2x e nearest. Comparação divide verticalmente o mesmo quadro, sem executar
duas partidas. Isto é upscale/filtragem, **não texturas remasterizadas**.

## Operação

- F6: Original/HD; F7: comparação; F10: painel gráfico; F4: recarregar pacote.
- F5/F9/F11 continuam reservados aos saves. Novos comandos são configuráveis
  em `[KeyMap]`: GraphicsMenu, GraphicsToggle, GraphicsCompare, HDReload.
- Launcher: filtros, escala inteira, proporção e pacote; Ctrl+L durante jogo.
- Painel: setas/Enter/Esc ou D-pad/A/B do controle já mapeado pelo framework.
- Saídas: Auto, 960×720, 1280×720, 1920×1080, 3840×2160; fullscreen usa o
  desktop do monitor. Proporções 4:3, 8:7, 1:1; widescreen desabilitado.
- Configuração: `%LOCALAPPDATA%\TMNTIVRecomp\config.ini`. Saves não mudam.

O painel congela a simulação, pausa áudio temporariamente e usa o caminho
existente de retomada/limpeza dos eventos. O controle de jogador 2 usa o
roteamento existente; verificação física dos controles ainda é necessária.

## Verificação executada

Compilação incremental MSVC `/O2 /MD /DNDEBUG` dos cinco arquivos modificados e
link Release com os objetos da versão funcional. Build limpo CMake continua
bloqueado por acesso negado ao diretório preexistente `src/gen`.

`tests/graphics_gpu_probe.c`: contexto GL real oculto, NVIDIA GTX 1650,
OpenGL 3.3 NVIDIA 617.42; 27 combinações de modo/filtro/resolução, **zero
falhas**. Original comparado com nearest independente (empates exatos entre
texels excluídos por precisão de interpolação); nearest HD idêntico ao
Original; metade esquerda de comparação idêntica ao Original. Readback é
exclusivo do teste. Dados de entrada inteiramente sintéticos.

Medição inicial de 30 draws por combinação, sem VSync, com `glFinish` somente
no teste: 1080p Original ~0,05 ms, Sharp ~0,15 ms, Scale2x ~0,17 ms;
4K Original ~0,20 ms, Sharp ~0,60 ms, Scale2x ~0,68 ms. Não são frame times da
partida. Framebuffer e textura nativa continuam 229.376 bytes cada; shaders
não alocam intermediários por quadro. RAM/VRAM total do processo não medida.

Ainda pendentes: validação visual humana de cenários, chefes, transições,
efeitos, áudio/foco, controles físicos, monitor 4K, fluxo completo de menu e
save/load com a nova apresentação. A etapa não é declarada concluída.

## Infraestrutura externa de assets (etapa 4 em validação)

O registro da etapa 3 precede três commits separados da etapa 4: infraestrutura
de manifesto/PNG, integração com renderização de origem e, por fim, ferramenta,
assets e testes. A PPU mantém `Ppu` e a serialização intactos; trilhas de origem
são buffers de host derivados novamente por quadro. IDs são SHA-256 de tile
planar, paleta, profundidade e camada. VRAM não é identidade. A atlas privada
tem limite de 255 imagens; recarga inválida mantém o pacote anterior.

O teste Release sintético executou 58 verificações: conteúdo e paleta alterados
produzem IDs novos; reutilização do endereço não herda a substituição; limites
de slot OAM/retângulo, fallback de camada, oclusão BG/HUD/sprite, color math,
mosaic e Mode 7; manifesto inválido, caminho traversal e recarga transacional;
persistência de configurações, comandos F4/F5/F6/F7/F9/F10/F11 e viewports em
4K. O teste OpenGL real executou 27 combinações e testes da atlas autoral,
alpha, brilho, divisão e máscara, **zero erros/falhas**. Os PNGs usados são
sintéticos.

Com instrumentação e pacote autoral carregado (sem IDs encontrados na ROM), o
teste privado da sessão ativa ainda restaurou os 180 quadros futuros. O perfil
de 600 quadros mediu 4,01 ms/frame Original e 4,62 ms/frame com busca habilitada
(cerca de 15% neste processo e cena). Working set medido foi 14,8 MB contra
15,7 MB; memória privada 20,4 MB contra 24,7 MB. Variação da execução/sistema
não foi isolada. O mapa adicional da GPU é 229.376 bytes; a atlas aloca até
4 MiB na CPU e 4 MiB na GPU. O custo depende da quantidade de substituições.

O executável de prévia inicializou a janela e OpenGL, desenhou 480 quadros com
entrada automatizada e saiu com código zero. Screenshot privado validado foi
256×224, mas a sequência ainda estava na tela de título. O ambiente não tinha
controles físicos conectados. Cenários de fase, chefes, transições, dois
jogadores com pacote realmente mapeado, áudio/foco durante o painel e teste
visual de 1080p/4K no jogo continuam pendentes. Os testes de dois jogadores e
NMI da etapa 1 passam no runtime headless, mas não substituem essa validação
visual/interativa. A etapa 4 também não é declarada concluída.

Shredder's Revenge é uma produção de Tribute Games, publicada por Dotemu em
parceria com Nickelodeon, segundo os [créditos oficiais de Tribute Games](https://tributegames.com/presskit/sheet.php?p=TMNT+Shredder%27s+Revenge).
O esquema atual pode receber PNGs autorizados, mas não converte automaticamente
a arte desse jogo. É necessário criar um mapeamento manual de IDs e licenciar
separadamente cada arte; nenhuma foi incluída.
