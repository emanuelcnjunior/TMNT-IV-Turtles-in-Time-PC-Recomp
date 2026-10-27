# Windows release 1.0.2

O jogador baixa apenas `TMNT-IV-Turtles-in-Time-PC-Setup-v1.0.2.exe`.
Windows 10 1903 ou posterior, x64, com driver gráfico OpenGL compatível com o jogo.
O instalador não baixa ROMs nem exige ferramentas de desenvolvimento.
O instalador informa que o jogador deve fornecer sua própria ROM americana.
O executável, os atalhos e o instalador usam um emblema original de tartaruga
com máscara, disponível em `installer/tmnt.ico`. O script `make-icon.py`
regenera o ícone em múltiplas resoluções usando Python e Pillow.

## Uso

Durante a instalação, selecione sua ROM original USA na etapa obrigatória de ROM, mesmo em reinstalações. O instalador valida o SHA-256 e importa a ROM para a pasta de dados do usuário. O nome de origem pode
ser qualquer um. O programa aceita exclusivamente 1048576 bytes e SHA-256
`5b82cdd6f2da56f43680d6a5021faebe2e06036d30602c1a7917aa414cf8b5f4`
(CRC32 de referência `5940BD99`). Cabeçalhos de copiador, patches e outras regiões
são rejeitados. As mensagens do seletor e de erro incluem português e inglês.

A cópia fica em `%LOCALAPPDATA%\TMNTIVRecomp\Teenage Mutant Ninja Turtles IV - Turtles in Time (USA).sfc`.
O SHA-256 cobre o arquivo inteiro e é verificado novamente a cada abertura.
A importação grava os mesmos bytes validados em arquivo temporário na pasta de
destino, faz flush e substitui o destino atomicamente. Uma falha não substitui a
ROM anterior por um arquivo parcial. A origem nunca é modificada.
Configuração e saves usam a pasta de dados do usuário. O fluxo retail abre o
seletor nativo quando necessário e abre o launcher após validar a ROM;
o menu de abertura permite configurar controles, vídeo e áudio antes de jogar.

A desinstalação preserva essa pasta. Para apagar explicitamente todos os dados,
feche o jogo e exclua **apenas** `%LOCALAPPDATA%\TMNTIVRecomp` pelo Explorer.
Isso apaga a cópia da ROM, configurações e saves. Reinstalar encontra a ROM preservada.

## Build para mantenedores

Ferramentas somente na máquina de build: Git e submódulos, Visual Studio 2022 C++
x64 (Windows SDK e componentes redistribuíveis), CMake >= 3.20, Ninja, Inno Setup
6.7.3 e as dependências de regeneração do projeto (Python/Rust/Bash).
Obtenha Inno Setup no repositório oficial `jrsoftware/issrc`, release `is-6_7_3`;
confira a assinatura Authenticode antes de executar. O compilador não é distribuído.

1. Faça checkout de um commit/tag conhecido, incluindo os submódulos. Integre e
   registre quaisquer alterações locais do framework antes de publicar; um
   checkout sujo não reproduz uma release a partir do tag sozinho.
2. Em Bash, regenere usando sua própria ROM, fora do repositório:
   `bash tools/regen.sh --rom "/c/caminho/rom-original.sfc"`.
3. No **Developer PowerShell for VS 2022 x64**, execute:

```powershell
./installer/build.ps1 -BuildDir build-release -Iscc 'C:/Program Files (x86)/Inno Setup 6/ISCC.exe' -CrtDir "$env:VCToolsRedistDir/x64/Microsoft.VC143.CRT"
./installer/test-rom.ps1 -Exe build-release/TmntIvTurtlesInTimeSNESRecomp.exe -Rom 'C:/caminho/rom-original.sfc'
./installer/test-install.ps1 -Rom 'C:/caminho/rom-original.sfc'
```

O teste de instalação precisa de uma conta de teste sem dados TMNTIVRecomp
anteriores e de desktop interativo. Instala numa pasta temporária do build,
abre/cancela o seletor, importa, desinstala, reinstala, inicia e fecha o jogo.
Preserva os dados ao final, tal como a desinstalação normal.
O teste de região usa conteúdo sintético com região alterada; complemente com
uma ROM real de outra região, obtida legalmente, na homologação.

O script exige build Release jogável (`SNESRECOMP_SETUP_HOST=OFF`), compila com
`TMNT_RETAIL_WINDOWS=ON`, versão 1.0.2, coleta as dependências PE recursivas e
falha se faltar uma dependência ou recurso obrigatório. Copia o CRT redistribuível
x64 app-local, inclusive quando a máquina de build já possui essas DLLs no Windows.
O UCRT e DLLs do sistema são fornecidos pelo Windows mínimo suportado.

O stage é novo a cada build e usa uma lista explícita de recursos. Nunca copia
rom.cfg, ROMs, src/gen, ferramentas, dumps, patches, diretórios de análise ou
dados extraídos do jogo. A capa existente acompanha o launcher com sua atribuição
de origem. A instalação define tela cheia em `[Graphics] Fullscreen = 1`, sem
alterar as outras preferências; o jogador pode mudar essa opção no menu.
SDL é estático nesta configuração; se passar a compartilhado,
a resolução PE deve localizar sua DLL ou o empacotamento falha.
O `manifest.json` no instalador registra tamanho e SHA-256 de cada arquivo do payload.
As licenças e os avisos dos recursos acompanham o programa.

Saídas: `dist/TMNT-IV-Turtles-in-Time-PC-Setup-v1.0.2.exe` e `.exe.sha256`.
Reprodução significa repetir o processo com fontes, configurações e ferramentas
iguais; não há garantia de EXE byte a byte idêntico (timestamps e assinatura).
Não use `scripts/package_release.sh` para esta distribuição Windows: ele produz
um pacote de desenvolvimento antigo que exige compilação na máquina do jogador.

## Publicar no GitHub

1. Gere o instalador em Release e confira o manifesto, o checksum e a ausência de ROM ou dados extraídos.
2. Atualize `downloads/v1.0.2/`, `installer/RELEASE-v1.0.2.md` e o workflow `publish-approved-release-v1.0.2.yml` com o hash aprovado.
3. Faça commit dessas alterações e envie `main` para `origin`. O GitHub Actions valida os bytes, cria a tag e publica a release v1.0.2 como Latest.

Não envie o stage, a árvore de build ou qualquer ROM. Não há ROM em secrets de CI nem download de ROM na distribuição.
Instalação silenciosa também exige a ROM: /ROM="C:\caminho\jogo.sfc".
