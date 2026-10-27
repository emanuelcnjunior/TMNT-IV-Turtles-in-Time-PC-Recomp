# Etapa 1 — Save Game — 27/10/2026

Implementação e restauração aprovadas no harness de integração com a ROM local.
**Etapa ainda não homologada para distribuição:** faltam compilação limpa do
executável jogável, teste no executável instalado e teste após reiniciar o Windows.
O instalador estável não foi substituído. Um candidato de teste foi gerado
posteriormente por solicitação do usuário, conforme atualização ao final.

## Base e preservação do trabalho local

Branch `feature/save-game` criada no projeto e no submódulo utilizado pelo build.
A base local é a versão 1.0.0 com a correção de NMI indicada abaixo; não foi
possível estabelecer se corresponde à revisão mais recente no remoto.

| Repositório local | Revisão inicial |
|---|---|
| Projeto principal | `7b59f8ed6af0f8f824b369013812636d730ce85d` |
| Submódulo `snesrecomp`, com a correção de NMI do TMNT | `b46d2471a3dd1d4740e8b3415eb3dfa2418cf7f8` |
| Checkout separado `C:/TMNT4Recomp/snesrecomp` | `503063e443844409b055f4df1633f21aba63321b` |

O checkout separado acompanha o upstream e não é o framework selecionado pelo
projeto. As mudanças foram feitas no submódulo que aponta para
`emanuelcnjunior/snesrecomp-tmnt4`. Commit do framework:
`d14f8fc54e999a9420584ef361e01e6fd7489fdd`.

`fetch --all --prune` foi tentado no projeto e nos dois checkouts do framework.
As conexões a `github.com:443` falharam; **as referências remotas não puderam ser
atualizadas**. Nenhum reset, pull destrutivo ou descarte foi executado.
A alteração preexistente em `interp_bridge.c`, seu `.bak` e o arquivo não
rastreado `tatus --short --ignored` foram preservados e não entraram no commit.

## Implementação

Cinco slots manuais (`slot_01.state` a `slot_05.state`) e um rápido
(`quicksave.state`) usam `%LOCALAPPDATA%/TMNTIVRecomp/saves/` no Windows, tanto
no ponto de entrada normal quanto no retail. A inicialização falha com mensagem
se não conseguir preparar esse diretório. Os saves não dependem do diretório de
instalação. Os buffers antigos de caminhos de 96 caracteres foram ampliados.

F5 salva, F9 carrega e F11 abre a lista, congelando a simulação. Também é
possível pausar com P e abrir a lista com F11; no controle, Select+R abre a lista.
No mapeamento padrão da lista: setas selecionam, S salva, X carrega, Esc cancela.
Sobrescrever um slot manual ocupado exige uma segunda pressão de salvar;
fechar a lista ou mudar de slot cancela a confirmação. O slot rápido é separado.

Configuração em `[KeyMap]` de `config.ini`:

```ini
QuickSave = F5
QuickLoad = F9
SaveStateMenu = F11
```

As listas antigas `Load`/`Save` são ignoradas neste perfil: F5/F9 já carregavam
slots e os atalhos diretos de gravação pulavam a confirmação. As novas entradas
permitem remapeamento; `None` desassocia o comando. Os demais clientes do
framework conservam o número padrão de slots e os atalhos anteriores.

A ROM USA verificada possui byte de tamanho de SRAM `$7FD8=0`; não oferece save
tradicional em SRAM. A opção fictícia de SRAM foi retirada do launcher. Esta
implementação é um **save state**, não uma gravação de progresso nativa da ROM.

O caminho comum `RtlSaveSnapshotToMemory`/`RtlLoadSnapshotFromMemory` continua
sendo usado por arquivo e rewind. Não foi criado outro emulador/serializador.
O envelope existente verifica integridade CRC32 e identidade `tmnt4-beam-save-v1`.
O payload RTLS v10 recebe extensão v1 com hash SHA-256 da ROM e impressão digital
de fontes, compilador, ABI e opções de build. Versões/identidades incompatíveis,
truncamento e extensões inválidas são recusados antes de escrever na máquina.
A compatibilidade é conservadora: mudanças de runtime/compilador/configuração
de build podem invalidar saves; não há migração de formatos históricos.

A captura ocorre nos limites de quadro usados pelo host. A extensão reutiliza
`RtlSaveExecutionState` para `CpuState`, contexto do interpretador, relógios e
fila de eventos APU, sincronização do beam, refresh e resíduos DMA/PPU. Inclui
explicitamente PC suspenso e WAI do Beam Frame Driver. A pilha das interrupções
é a pilha real na WRAM, preservada junto com os registradores e flags.

Os testes encontraram uma lacuna adicional: o snapshot antigo omitia portas de
VRAM/CGRAM, latches, contadores da PPU e diários de rasterização. A nova extensão
da PPU preserva esses campos e os dois diários sem ponteiros do processo.
Isso corrigiu uma divergência de VRAM reproduzida ao carregar repetidamente
durante transferências gráficas. CPU, NMI, temporizadores, controladores de ambos
os jogadores, WRAM, VRAM, CGRAM, OAM, APU/SPC/DSP e DMA continuam usando os
serializadores existentes, com os resíduos necessários acrescentados.

A gravação escreve um temporário exclusivo no mesmo diretório, verifica o total
escrito, chama `FlushFileBuffers`, fecha e só então substitui o destino por
`MoveFileExW` com replace/write-through. Falha antes da substituição conserva o
arquivo anterior. Uma morte do processo pode deixar um temporário órfão; ele
não é tratado como save e não impede uma gravação posterior. Thumbnails são
cosméticos e podem estar ausentes ou desatualizados após interrupção.

## Resultados observados

| Verificação | Resultado |
|---|---|
| Primeira fase, 1 jogador | Aprovado no harness; fase e atores conferidos em captura |
| Partida ativa, 1 jogador | Aprovado; combate e inimigos visíveis |
| Dois jogadores | Aprovado; dois personagens e duas barras de vida conferidos |
| Encerrar processo, iniciar outro e carregar | Aprovado nos quatro cenários abaixo |
| Continuidade de vidas/HUD, pontuação, posições, inimigos e animação | Estado integral e imagem idênticos durante 180 quadros após a carga |
| Checkpoint dentro da NMI | Aprovado, `PC=$008306`, `S=$1DDC`, `$003C=$01` |
| Repetições da NMI suspensa | 8 recargas adicionais por processo, 180 quadros idênticos por recarga |
| Repetições em limites sucessivos | 120 capturas/avanços/restaurações por processo e cenário |
| ROM/schema/identidade incompatíveis | Recusa sem alteração da máquina; checksum recalculado para testar preflight |
| Corrupção/truncamento | Recusa sem alteração da máquina |
| Interrupção de escrita | Arquivo anterior preservado; gravação posterior recuperada |
| Unicode, caminho maior que 96 caracteres, nomes dos seis slots | Testes aprovados; não implica suporte a caminhos acima de MAX_PATH |
| Confirmação manual e isolamento do quick slot | Testes do menu aprovados, perfil de 5 slots e padrão de 12 slots |
| F5/F9 e remapeamento com configuração antiga | Aprovado |
| Núcleo 65816 | 89/89 verificações |
| Ponte interpretador/AOT | 438/438, incluindo S21: NMI aninhada libera espera, ambos os RTIs e pilha/PC |
| Reinício do Windows | Não executado |
| Executável instalado | Não executado |

Os quatro cenários são: primeira fase, combate 1P, combate 2P e NMI suspensa.
Em cada cenário, um processo grava o checkpoint e a referência; outro inicializa
uma máquina nova e carrega antes de executar o boot. Os 180 hashes seguintes
comparam **todo o snapshot e todos os pixels visíveis**, excluindo apenas a fila
de PCM destinada ao dispositivo do host. Registradores/estado interno do DSP e
SPC permanecem na comparação. As capturas finais dos dois processos são
idênticas byte a byte e diferem da imagem do checkpoint: houve avanço real.
Não foram usados pokes de RAM. Não foi necessário identificar individualmente
os endereços de vidas/pontuação: toda a WRAM e o HUD renderizado são comparados.

O teste de arquivo atômico intercepta as chamadas Windows da implementação real:
escrita parcial, flush/close/rename com erro, destino bloqueado, processo filho
encerrado durante a escrita e recuperação. O teste transacional existente do
framework também passou, incluindo validação prévia de prefixos malformados.
O pacote `pytest` não está instalado: a suíte Python não executou. O harness
transacional C foi extraído das mesmas funções de produção e compilado/executado
diretamente com MSVC, assim como os testes de menu, núcleo e ponte.

## Build, evidências e pendências

O CMake Release foi tentado, mas o Windows negou listagem/leitura de `src/gen`.
O CMake então relata que o diretório está vazio. Não foi substituído por um setup
host nem foi alegada uma compilação limpa jogável.

Os dez arquivos C novos/alterados foram compilados com MSVC x64 `/O2 /MD
/std:c11`, sem avisos na compilação final. O probe foi linkado incrementalmente
com esses objetos e os demais objetos Release locais existentes, incluindo o
código gerado já compilado. Isso permite testar a restauração no runtime real,
mas **não equivale a regenerar e reconstruir todo o produto a partir das fontes**.

Evidências privadas ficam em `C:/TMNT4Recomp/save-validation/`: `compile.log`,
`build-probe.log`, `components.log`, `menu-test.log`, `menu-legacy.log`,
`keys.log`, `core-tests.log` e subpastas `one-stage`, `one-active`, `two-active`,
`nmi` com logs, hashes, estados e imagens. Esses estados/imagens contêm dados
derivados do jogo e não entram nos commits. Reprodução está em
`tests/SAVE-TESTS.md`.

Antes de distribuir: atualizar referências remotas quando houver conexão;
repetir build Release completo com acesso aos fontes gerados; testar F5/F9 e a
lista no executável instalado, com dados no LocalAppData real; encerrar/reabrir;
reiniciar o Windows e carregar; confirmar controles físicos e áudio. Não foi
executada campanha completa nem homologação em outra máquina. O instalador
estável permanece preservado; a homologação do candidato experimental está pendente.

## Arquivos alterados

- Projeto: `CMakeLists.txt`, `.gitignore`, `src/main.c`, `src/game_rtl.c`,
  `src/game_save.c`, `src/game_save.h`, pin do submódulo, README e este relatório.
- Testes do projeto: `tests/save_runtime_probe.c`, `tests/atomic_state_test.c`,
  `tests/SAVE-TESTS.md`.
- Framework: `runner/runner.cmake`, `runner/src/atomic_state_file.{c,h}`,
  `beam_frame_driver.{c,h}`, `common_rtl.c`, `save_state_slots.h`,
  `snes/ppu.{c,h}`, `snes_savestate_menu.c`, `desktop/config.h`,
  `desktop/mmx_config.c`, `desktop/host_main.c`.
- Testes do framework: `tests/host/savestate_menu_test.c` e
  `tests/snapshot_guard/test_transaction.py`.

Nenhuma ROM, código gerado do jogo, save, imagem capturada ou dado original do
jogo foi adicionado ao controle de versão. A ROM foi somente lida e verificada.

## Atualização — instalador experimental solicitado pelo usuário

Gerado `dist/TMNT-IV-Turtles-in-Time-PC-Setup-v1.0.1-savegame-test.1.exe`
(5.698.784 bytes), sem substituir o instalador estável v1.0.0.

SHA-256: `232A861BD200254BA51213B2BBA72B4D46EE40637AC3EF00E40F72E3EC3FAD12`.

Os arquivos alterados foram recompilados com MSVC x64 `/O2 /MD /DNDEBUG` e
linkados com os objetos Release existentes do código gerado. O ponto de entrada
é o retail normal, não o probe. O manifesto UTF-8 foi incorporado ao executável.
O executável anterior de desenvolvimento foi preservado em
`C:/TMNT4Recomp/save-validation/installer-build/previous-desktop.exe`.

Os mesmos objetos de runtime passaram novamente no teste de carga em novo
processo com dois jogadores: 180 quadros de estado/imagem iguais e 120 ciclos
de restauração. O executável retail passou nas sete verificações de ROM. O
empacotador verificou dependências, recursos e ausência de ROM/dados proibidos;
o hash do executável empacotado coincide com o recém-linkado.

`installer/build.ps1` e `installer/tmnt.iss` agora aceitam uma versão de pacote,
mantendo `1.0.0` como padrão. O candidato foi empacotado com `-SkipBuild` após
o build incremental acima. Não constitui compilação limpa a partir de
`src/gen`, que continua inacessível. Instalação, operação de F5/F9 no programa
instalado e carga após reiniciar o Windows ficam para a validação solicitada.
