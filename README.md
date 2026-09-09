# Zenith Granular — Fase 6 (Presets)

Guarda/carrega o estado completo do plugin (todos os knobs + o sample) com
um nome, na pasta de dados do utilizador — funciona em qualquer projeto do
FL Studio, não só no que foi guardado.

## O que este código faz
- Regista o plugin como **instrumento** (Channel Rack do FL Studio).
- Botão **"Load Sample..."** ou **arrastar e largar** um ficheiro de áudio.
- Formatos suportados: WAV, AIFF, FLAC, OGG, MP3, M4A, WMA.
- **Presets**: dropdown com a lista de presets guardados, botão Save (pede
  um nome) e Delete. Ficam em
  `%APPDATA%\ZenithGranular\Presets\*.zgpreset` no Windows.
- Interface em 4 abas: Sampler, Granular, FX, Macros.
- Sampler: Attack, Decay, Sustain, Release, Pitch, Fine Tune.
- Granulador: Grain Size, Density, Position, Position Random, Grain Pitch,
  Pitch Random, Pan Spread, Granular Mix.
- FX: Filtro (LP/HP/BP + Cutoff/Resonance), Saturação (Soft Clip/Tape/Tube +
  Drive/Mix), Delay (Time/Feedback/Mix), Reverb (Size/Damping/Mix).
- Macros: Space (Reverb+Delay Mix), Texture (Granular Mix+Density), Movement
  (Position Random+Pan Spread), Chaos (Pitch Random+Saturation Drive/Mix).
- O sample carregado é lembrado entre sessões do projeto `.flp` e dentro
  dos presets.

## Presets de fábrica
16 presets vêm prontos na primeira vez que abres o plugin (não precisam de
sample próprio — aplicam-se a qualquer som que carregares), em várias
variações por categoria:
- Piano: Clean, Bright, Soft
- Bass: Deep, Bright, Sub
- 808: Punch, Sub, Distorted
- Keys: Warm, Bright, Dreamy
- Pad: Atmospheric, Dark
- Lo-Fi: Dust, Wobble

## Favoritos
Botão de estrela ao lado da lista de presets marca/desmarca o preset atual
como favorito. O interruptor "Favoritos" filtra a lista para mostrar só
esses.

## Correção importante
O carregamento de presets estava a mostrar o nome escolhido mas sem mexer
nos knobs de verdade. Corrigido — agora usa o mesmo mecanismo já testado
dos Macros.

## Design visual (Fase 7 - início)
- LookAndFeel próprio: knobs circulares com arco teal, fundo escuro, abas
  e botões com a identidade visual do projeto.
- Waveform real do sample carregado, no topo da janela.

## Presets de fábrica
28 presets nativos, protegidos contra remoção acidental (o botão Delete
recusa apagar presets de fábrica, só os teus). Variações por categoria:
- Piano: Clean, Bright, Soft, Dark, Granular (5)
- Bass: Deep, Bright, Sub, Distorted, Granular (5)
- 808: Punch, Sub, Distorted, Clean, Wide (5)
- Keys: Warm, Bright, Dreamy, Granular, Vintage (5)
- Pad: Atmospheric, Dark, Bright, Drone (4)
- Lo-Fi: Dust, Wobble, Crushed, Ambient (4)

## Classificação por estrelas (1-3) e favoritos
Ao lado da lista de presets há 3 estrelas clicáveis (como no Zenology) para
classificares o preset atual. O botão "Favoritos" abre um menu só com os
presets classificados, sem esconder ou alterar a lista principal.

## Correção importante
O carregamento de presets estava a mostrar o nome escolhido mas sem mexer
nos knobs de verdade. Corrigido - agora usa o mesmo mecanismo já testado
dos Macros.

## Próximo passo (Fase 7b)
Continuar o design: animação dos grãos sobre a waveform, e depois a
biblioteca nativa de samples (Fase 6b).
