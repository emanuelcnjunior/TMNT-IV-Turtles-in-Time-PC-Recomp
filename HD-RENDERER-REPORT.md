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
