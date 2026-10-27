# TMNT IV: correção e validação — 27/10/2026

Executável Release: `build/TmntIvTurtlesInTimeSNESRecomp.exe`. Execute dessa pasta, mantendo DLLs e arquivos de suporte existentes. A ROM continua externa, sem alteração; SHA-256 confirmado como `5b82cdd6f2da56f43680d6a5021faebe2e06036d30602c1a7917aa414cf8b5f4`.

A causa do travamento era o modelo de execução atômica de interrupções do driver. A NMI em $008174 grava $01 em $003C e seleciona seu caminho conforme o valor anterior. O caminho principal executa lógica do jogo e pode entrar em $008324, que grava $80 e espera. Uma NMI posterior encontra o sinalizador ocupado, executa o caminho curto de serviço e limpa $003C, permitindo que a NMI anterior continue. Exigir RTI antes de devolver controle ao agendador impedia essa próxima NMI e atingia o limite de instruções. Continuar com o contexto parcialmente alterado causava as falhas secundárias observadas.

Em `snesrecomp/runner/src/beam_frame_driver.c`, interrupções agora usam execução retomável até o limite do campo ou estado quiescente. PC e pilha reais permanecem válidos entre campos; RTI retorna ao fluxo interrompido, inclusive a uma NMI mais antiga. A entrada salva P antes de definir I=1 e D=0. A contagem diagnóstica de interrupções foi preservada. O contrato antigo de `interp_bridge_run_interrupt()` permanece disponível aos demais hosts.

Os diagnósticos temporários TMNT foram removidos do driver e da ponte ativa. O backup `.bak` preexistente foi preservado. Não houve alteração artificial da ROM, do sinalizador $003C, das tabelas do jogo ou dos tempos para contornar a espera.

Backups anteriores à edição: `analysis/backups/`. Patch final: `analysis/framework-fix.patch`. As alterações preexistentes em `recomp/bank00.cfg` foram preservadas e registradas em `analysis/baseline-project.patch`. Não foram criados commits, para manter o trabalho local existente separado da revisão pelo usuário.

Validação executada:

- Compilação e link Release via CMake/Ninja/MSVC concluídos.
- Núcleo 65816: 89/89 verificações, incluindo larguras, pilha, decimal, WAI e interrupções. Registro: `interp816_test.log`.
- Ponte interpretador/AOT: 438/438 verificações. Nova regressão S21 reproduz espera dentro de NMI, suspende no prazo de um campo, entrega NMI aninhada e verifica liberação da espera, ambos os RTIs e restauração da pilha e PC principal. Registro: `bridge_test.log`.
- Testes por roteiros de entrada, sem pokes de memória: copyright, Konami, título, menu, seleção, cena inicial e combate da primeira fase. Capturas BMP e dumps de estado disponíveis em `analysis/`. `title.fb.bmp`, `select.fb.bmp`, `stage.fb.bmp`, `resumed.fb.bmp` e `fps_after.fb.bmp` mostram o avanço.
- Roteiros exercitaram Start, direita+ataque e salto; capturas mostram avanço horizontal do cenário e inimigos. Start adicional foi enviado durante o jogo; pausa/retomada ainda merece confirmação manual específica.
- Teste normal: 3.847 simulações e apresentações em 64,342 s, média global 59,790 fps incluindo transições/capturas. Registro: `realtime.log`.
- Perfil de gameplay no binário final: 714 quadros registrados; 60,026 fps calculados entre timestamps de apresentações, mediana 16,665 ms, p95 17,064 ms, maior intervalo 19,316 ms. CPU do jogo: média 3,759 ms/quadro. Dados: `gameplay-timing.csv` e `validation.json`.
- Renderer Direct3D 11, VSync ativo, monitor 60 Hz; simulação NTSC preservada em 60,098812 Hz. Nenhuma modificação de velocidade foi necessária. A taxa medida não constitui garantia de ausência de quedas em todo o jogo ou outros PCs.
- Nenhuma ocorrência de limite do interpretador, BRK inesperado ou falha TMNT nos testes do jogo corrigido. Alguns testes sintéticos do núcleo/ponte provocam BRK/limite intencionalmente.
- Áudio WASAPI abriu em estéreo, 32 kHz, com callbacks e tráfego SPC registrados. Controles PS4 e Xbox 360 foram detectados; comandos de teste vieram dos roteiros, sem operação física dos controles.

Limitações: não foi feita comparação diferencial com emulador independente, escuta do áudio, campanha completa, teste de dois jogadores, save/load ou confirmação física dos controles. A suite Python pytest não rodou porque pytest não está instalado; os dois harnesses C foram compilados e executados diretamente com MSVC. O teste da ponte usou somente um adaptador local de setenv para _putenv_s, sem alterar a implementação do framework por essa necessidade de teste.

Para conferência manual: abra o executável, inicie uma partida, confirme ataque/salto e pausa no controle escolhido, ouça música e efeitos, e jogue fases adicionais. No teclado configurado, Return=Start, setas=movimento, A=Y e Z=B; confira a fonte de entrada selecionada no launcher. Tab ativa turbo e deve permanecer desligado para jogar em velocidade normal. VSync=1 e DisableFrameDelay=0 já estão configurados.
