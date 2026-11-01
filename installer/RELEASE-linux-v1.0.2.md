# TMNT IV PC Recomp v1.0.2 — Linux x86-64 experimental

Primeiro pacote Linux nativo com o executável já compilado, launcher,
modo Original/HD e save states. Não exige Wine nem recompilação no computador
do jogador. A ROM não está incluída: selecione sua própria ROM USA no launcher.

Extraia `TMNT-IV-Turtles-in-Time-PC-v1.0.2-linux-x86_64.tar.gz` e execute
`./run.sh` na pasta extraída. Configurações e `saves/` ficam nessa pasta.

Requisitos: Linux x86-64 com glibc 2.35+, libstdc++ (GLIBCXX_3.4.29+) e
OpenGL. Ubuntu 22.04 é a base de compatibilidade; outras distribuições e
Steam Deck ainda precisam de testes. Não presuma compatibilidade entre
save states Windows e Linux.

Validação automatizada: integridade do pacote e manifesto, executável
ELF x86-64, bibliotecas e abertura/captura do launcher em Ubuntu 22.04.
A campanha, áudio e controles ainda precisam de validação manual em Linux.

O arquivo `.sha256` acompanha o pacote. Instruções técnicas: `docs/LINUX.md`.
