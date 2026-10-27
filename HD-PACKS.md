# Pacotes gráficos externos

Os pacotes são privados por padrão e residem em
`%LOCALAPPDATA%\TMNTIVRecomp\hd-packs\`. O painel F10 controla o pacote; F4
recarrega. `HDPackDir`, `HDPackEnabled` e `HDDiagnostics` também podem ser
configurados no `config.ini`. Quando o pacote está desligado, o modo HD volta
imediatamente ao framebuffer do jogo.

`tools/hd_pack_tool.py --example hd-packs/author-example` gera um PNG e um
manifesto CC0 de um emblema geométrico criado por esta ferramenta. Ele não
usa imagens do jogo. O exemplo demonstra a importação, mas o ID sintético dele
não corresponde a um tile do TMNT IV.

## Manifesto versão 1

```json
{
  "version": 1,
  "author": "Artista",
  "license": "Licença ou autorização documentada",
  "entries": [
    { "id": "64 caracteres hexadecimais minúsculos", "file": "tile.png" },
    { "id": "outro id observado", "file": "frame-2.png", "oam": 17,
      "rect": [32, 24, 96, 72] }
  ]
}
```

PNG RGBA quadrado de 8, 16, 32 ou 64 pixels. O loader normaliza cada imagem a
uma célula RGBA de 64×64, com nearest, numa atlas de 1024×1024 e até 255
entradas. Alpha zero mostra o pixel original; o contorno é sempre limitado à
cobertura nativa. Sem mapeamento válido, a PPU fornece o original.

O ID SHA-256 usa a tag de versão, profundidade (2/4 bpp), camada PPU, dados
planos completos do tile e 16 cores RGB15 da paleta, serializados em little
endian. Não usa endereço VRAM. O endereço é reutilizável e o mesmo tile pode
aparecer em vários lugares. A camada faz parte do ID; slot OAM e retângulo
opcionais limitam contexto. O retângulo testa a origem da observação do tile.
IDs de quadros diferentes de uma animação são recursos independentes: mapear
cada ID deixa a troca seguir a temporização original do jogo. O pacote nunca
avança uma animação por relógio próprio.

Conteúdo JSON é limitado a 256 KiB; há no máximo 255 imagens de 1 MiB cada,
PNG limitado a 64×64 e nomes planos sem caminhos. Recarregar constrói atlas e
mapa novos antes de substituir o pacote atual. Um manifesto ou PNG inválido
preserva a cópia em uso. A validação exige campos `author` e `license`, mas
não verifica se a declaração de direitos é verdadeira. Só instale arte que
você criou ou cuja licença/autorização permita esse uso e distribuição.

## Diagnóstico e limites de cobertura

Ative `HDDiagnostics=1` para registrar `ids.jsonl` no diretório do pacote.
Ele inclui ID, camada, profundidade, slot OAM quando aplicável e coordenada de
observação. O arquivo contém metadados, não pixels, VRAM ou imagens da ROM.
`python tools/hd_pack_tool.py --inspect <ids.jsonl>` agrupa observações para
facilitar o mapeamento.

A primeira integração substitui somente pixels vencedores em Mode 0/1,
tiles 8×8, sem mosaic, color math, color clipping, espaço widescreen ou
camadas adicionais injetadas. Mode 7, modos 2–6, mosaic, math/clip, desenho
legado, widescreen e overlays de host usam a imagem original. O framebuffer,
HUD, contorno transparente, prioridades e colisões permanecem na composição
original. Pacotes podem combinar IDs de quadros nativos, mas não ampliam um
personagem inteiro como uma arte única; não se deve chamar isso de personagem
remasterizado.

A amostragem usa atlas e mapa de tags OpenGL. O único tráfego adicional é uma
textura de metadados R32UI de 256×224 (229.376 bytes por atualização quando o
pacote está visível). Nenhum pixel retorna da GPU durante a execução normal.
Screenshot/readback fica reservado aos testes ou ao comando explícito de
captura. Paletas/brightness RGB15 são convertidos antes do overlay; modos de
cor que não preservem a matemática SNES recebem fallback.
