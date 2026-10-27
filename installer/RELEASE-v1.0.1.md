# v1.0.1 — Save Game

O usuário confirmou o teste do executável fornecido. O instalador v1.0.1
empacota exatamente esse executável, sem recompilá-lo após a aprovação.

- Cinco slots manuais e Quick Save separado em LocalAppData.
- F5 salva, F9 carrega, F11 abre a lista de saves.
- Gravação segura, verificação de compatibilidade e contexto completo de
  execução, incluindo NMI suspensa e resíduos da PPU.
- O instalador não contém ROM; exige a ROM USA original do usuário.

Instalador: `downloads/v1.0.1/TMNT-IV-Turtles-in-Time-PC-Setup-v1.0.1.exe`.

SHA-256:
`8D2B96A2AE700E1A74962B972535D6A4641DDAB5DD2704CA81AB0DAC612E5F4C`.

Executável aprovado, SHA-256:
`012918343BAF3205A6EF50D66F1D2274BB849C6CD25AE3AE3787C09B73894053`.

O build é incremental: arquivos alterados recompilados em Release e objetos
existentes do código gerado. O bloqueio de acesso a `src/gen` impediu uma
recompilação limpa. A identificação interna do executável mantém a do candidato
`1.0.1-savegame-test.1`; a versão do pacote é 1.0.1. O executável foi mantido
idêntico ao aprovado para não introduzir mudanças após o teste.

A confirmação do usuário não foi interpretada como comprovação de todos os
cenários de hardware, reinício do Windows ou reinstalação. Os resultados
automatizados estão em `SAVE-GAME-REPORT.md`.

A Etapa 2 continua parcial: localização PT/EN, exclusão confirmada, classificação
visual de saves inválidos e integração ao launcher permanecem pendentes,
conforme `SAVE-UI-AUDIT.md`. A versão 1.0.0 e seu instalador foram preservados.

O instalador e o checksum ficam no repositório porque o conector disponível não
oferece upload de anexos em Releases. Nenhum save, thumbnail, ROM ou dump privado
foi incluído.
