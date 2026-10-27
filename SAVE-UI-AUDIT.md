# Auditoria da Etapa 2 — interface de Save Game

Data: 27/10/2026. Base: projeto `e145968`, framework `d14f8fc`.
**Resultado: parcialmente atendida; Etapa 2 não concluída.** Esta revisão verifica
a implementação existente e acrescenta um teste de invariância do overlay.
Não implementa os recursos ausentes e não modifica o instalador entregue.

| Item solicitado | Situação | Evidência ou pendência |
|---|---|---|
| Menu “Salvar jogo” | Parcial | O overlay F11 oferece SAVE; não há entrada própria com esse nome em português nem no launcher. |
| Menu “Carregar jogo” | Parcial | O mesmo overlay oferece LOAD; falta a integração e a apresentação solicitadas. |
| Cinco slots manuais | Atendido | `SNES_SAVESTATE_SLOTS=5` no CMake; navegador usa esse total. |
| Quick Save separado | Atendido | Sexto slot exclusivo, além dos cinco manuais. |
| Miniatura privada | Implementado | `<arquivo>.state.thumb`, 128×112 pixels, no mesmo diretório privado do save. Escrita não atômica: após falha pode ficar ausente ou desatualizada. |
| Data e hora | Implementado | `format_slot_status` mostra `mtime` local em `YYYY-MM-DD HH:MM`; é a data do arquivo, não um timestamp independente no cabeçalho. |
| Vazio/ocupado/incompatível/corrompido | Não atendido integralmente | O navegador só verifica existência e tamanho maior que zero. Arquivo inválido não vazio aparece ocupado; zero bytes aparece vazio. A carga é recusada pelo backend, mas a interface mostra apenas LOAD FAILED. |
| Confirmação para sobrescrever | Parcial | Existe para slots manuais ocupados. Quick Save sobrescreve diretamente; o texto da Etapa 2 não explicita essa exceção. |
| Exclusão com confirmação | Não atendido | Não há comando de excluir save nem seu thumbnail. |
| Português e inglês | Não atendido | O navegador e suas mensagens estão em inglês. A mensagem bilíngue de erro de diretório não traduz a interface de saves. |
| Navegação por teclado | Implementado/testado parcialmente | Atalhos e seleção de slots têm cobertura automatizada; não foi homologada toda a interface gráfica. |
| Xbox/PlayStation | Parcial | Eventos SDL e ações de pad estão implementados. Testes usam palavras de entrada; não comprovam operação física das duas famílias. |
| Interface gráfica existente | Atendido no jogo | Usa `snes_savestate_menu` e compositor existentes, sem outro toolkit. |
| Integração ao launcher | Não atendido | `src/main.c` define `sram_path=NULL`; o launcher condiciona seu painel SAVES a SRAM/password. Não existe navegador dos seis save states integrado ao launcher. Reativar um painel de SRAM não resolveria isso. |
| Menu não altera a partida | Lógica do overlay aprovada | Novo teste compara todo o estado serializado e pixels antes/depois de abrir, navegar, desenhar e cancelar. Falta executar esse teste através do loop gráfico/eventos real. |
| Áudio não interrompido permanentemente | Pendente de homologação | O host pausa áudio na entrada, limpa a fila do host e restaura `g_paused` na saída. Não foi feita escuta nem validação end-to-end da recuperação do dispositivo. |
| Não perder foco/controles | Pendente de homologação | Há tratamento de eventos e máscaras de liberação dos botões. Não há comprovação nesta auditoria de Alt+Tab, foco, reconexão e dispositivos físicos. |
| Diretório LocalAppData | Implementado | O ponto de entrada Windows define `%LOCALAPPDATA%/TMNTIVRecomp/saves` para saves e quicksave; thumbnails seguem a mesma raiz. |
| Reinstalar/atualizar preserva saves | Previsto pelo instalador | Dados fora de `{app}`, sem exclusão genérica de LocalAppData. Falta testar reinstalação/atualização com arquivos `.state` e `.state.thumb` reais. |
| Resoluções/modos de exibição | Parcial | Geometria aprovada em 1920×1080, 2520×1080, 800×600 e 900×1200. Não é validação de todos os modos, DPI, redimensionamento e fullscreen. |
| Testar cada comando | Não atendido integralmente | Save/load, isolamento quick/manual e confirmação manual têm testes. Exclusão não existe; integração launcher e idiomas também não. |
| Continuidade com dois jogadores | Aprovada no harness | Máquina nova carrega checkpoint real 2P e reproduz 180 quadros de estado e imagem, mais 120 ciclos de restauração. Não substitui teste dos menus com dois controles físicos. |
| Commit separado e versão anterior recuperável | Auditoria separada | A versão anterior é `e145968`, preservada também pela tag `stage1-savegame-test`. Este commit contém auditoria/teste, não uma implementação completa da Etapa 2. |

## Testes executados nesta auditoria

1. Recompilado e executado `snesrecomp/tests/host/savestate_menu_test.c` com
   `SNESRECOMP_PERSISTENT_SAVE_PROFILE=1` e `SNES_SAVESTATE_SLOTS=5`: PASS,
   saída 0. Exercita geometria, seleção, save/load, thumbnail, isolamento do
   quick e confirmação/cancelamento de sobrescrita manual.
2. Recompilado o ponto de entrada de teste `tests/save_runtime_probe.c`, linkado
   aos objetos Release do runtime usados no candidato de instalador. Executada
   a ROM local verificada, carregando checkpoint de combate com dois jogadores:
   PASS, saída 0. Houve 180 comparações consecutivas de estado completo e frame
   visível e 120 ciclos de captura/avanço/restauração/reexecução.
3. Novo teste no probe: abrir o overlay por Select+R, navegar, obter o painel
   renderizado e cancelar. Snapshot completo e pixels permaneceram iguais:
   PASS. A API do overlay foi exercitada diretamente; o teste não chama
   `RunSavestateMenuLoop`, não exercita o dispositivo de áudio nem eventos SDL
   de foco/controles físicos.

Logs privados: `C:/TMNT4Recomp/save-validation/stage2-audit/menu.log`,
`probe-build.log`, `runtime-2p.log`. O jogo foi executado no harness sem janela;
não foi realizada validação interativa do executável instalado nesta auditoria.
O bloqueio de leitura de `src/gen` e a limitação do build incremental continuam
descritos em `SAVE-GAME-REPORT.md`. Saves/imagens não foram versionados.

## Evidências no código

- `src/main.c`: raiz LocalAppData e `sram_path=NULL`.
- `CMakeLists.txt`: perfil persistente e cinco slots.
- `snesrecomp/runner/src/snes_savestate_menu.c`: `slot_exists`,
  `format_slot_status`, `rasterize_panel`, `menu_submit`, `write_thumb` e navegação.
- `snesrecomp/runner/src/desktop/host_main.c`: `RunSavestateMenuLoop`,
  `PumpOverlayEvents`, `HandleDeviceEvent`, `OverlayNoteClosed` e áudio.
- `recomp-ui/src/common/launcher_model.c`: `saves_supported` depende de SRAM.
- `recomp-ui/src/common/backends/imgui/launcher_imgui.cpp`: painel SAVES existente.
- `installer/tmnt.iss`: diretório do programa e preservação dos dados privados.

Para concluir a Etapa 2 faltam principalmente integração real ao launcher,
ações/menus localizados, inspeção não destrutiva dos quatro estados de slot,
exclusão confirmada e testes gráficos de áudio/foco/controles, resoluções e
reinstalação. A implementação funcional da Etapa 1 permanece recuperável.
