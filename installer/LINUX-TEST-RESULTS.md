# Linux v1.0.2 — validação do pacote experimental

Data: 2026-11-01.

Artefato: `TMNT-IV-Turtles-in-Time-PC-v1.0.2-linux-x86_64.tar.gz`.
Tamanho: 4.668.720 bytes.
SHA-256: `a25a571291be3035b0d487333d389624e63e9c3a0beeb1b6479b73a4ceba58fc`.

## Build e conteúdo

- ROM USA verificada localmente pelo SHA-256 de `rom_identity.txt`.
- Recompilador nativo compilado; geração local de quatro unidades C concluída.
- Clang 22.1.8 no Windows, target `x86_64-linux-gnu`, sysroot Ubuntu da
  toolchain portátil v1.0.14; SDL3 3.4.10 ligado estaticamente.
- CMake Release com `SNESRECOMP_SETUP_HOST=OFF` e composição LLE.
- Executável ELF64 x86-64 PIE com RUNPATH `$ORIGIN`.
- TAR.GZ preserva permissão 0755 no executável e em `run.sh`.
- Pacote contém executável, recursos, licenças, instruções e manifesto;
  exclui ROM, código gerado, fontes privados e dados do usuário.

## Teste em Linux

[Execução aprovada no GitHub Actions](https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp/actions/runs/41267548038),
segunda tentativa, job Ubuntu 22.04, duração de 26 segundos.

- Download do mesmo arquivo armazenado na release em rascunho.
- `sha256sum --check`: aprovado.
- Todos os tamanhos e hashes do `manifest.json`: aprovados.
- Bibliotecas verificadas por `ldd`: nenhuma ausente.
- Launcher aberto em Xvfb, renderização Mesa por software, áudio dummy.
- Janela visível de 1100 × 880 identificada por PID e capturada.
- Captura inspecionada: capa, fontes, controles e seletor de ROM visíveis.
  O seletor integrado aparece quando zenity/kdialog não estão instalados.
- Configurações ancoradas corretamente à pasta extraída, conforme o log.

A ROM permaneceu na máquina local. O teste Linux não iniciou uma partida.
Campanha, desempenho durante gameplay, áudio real, gamepads e salvar/carregar
ainda precisam de validação manual em Linux. Por isso o pacote é experimental.

As evidências (captura, log, relatório ELF, janela e bibliotecas) estão no
artefato `linux-playable-package-verification` da execução acima.
