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
39 presets nativos, auto-reparáveis (se algum for apagado por engano numa
versão antiga, volta a aparecer sozinho da próxima vez que abrires o
plugin) e protegidos contra remoção pelo botão Delete. Categorias:
- Piano (5): Clean, Bright, Soft, Dark, Granular
- Bass (5): Deep, Bright, Sub, Distorted, Granular
- 808 (5): Punch, Sub, Distorted, Clean, Wide
- Keys (5): Warm, Bright, Dreamy, Granular, Vintage
- Pad (4): Atmospheric, Dark, Bright, Drone
- Lo-Fi (4): Dust, Wobble, Crushed, Ambient
- Bell (4): Clean, Dark, Granular, Shimmer
- Synth (4): Lead, Pad, Pluck, Bass
- Pluck (3): Bright, Warm, Granular

## Navegar por categoria
Botão "Presets ▾" no topo abre um menu com uma categoria por cima (Piano,
Bass, 808, etc.) e, dentro de cada uma, só os presets dessa categoria —
como pediste, clicar em "Piano" mostra só as variações de piano.

## Classificação por estrelas (1-3) e favoritos
Ao lado da lista de presets há 3 estrelas clicáveis (como no Zenology) para
classificares o preset atual. O botão "Favoritos" abre um menu só com os
presets classificados, sem esconder ou alterar a lista principal.

## Correções importantes
- O carregamento de presets não estava a mexer nos knobs de verdade —
  corrigido (usa o mesmo mecanismo já testado dos Macros).
- Presets de fábrica apagados por engano numa versão antiga agora voltam
  a aparecer sozinhos (sistema auto-reparável, sem depender de um
  "marcador" que impedia a reposição).

## Correções de interface
- Removidos símbolos Unicode que não apareciam bem em alguns PCs (seta do
  botão "Presets", estrela do botão "Favoritos" - agora só texto).
- As 3 estrelas de classificação agora são desenhadas como forma vetorial
  (Path), não dependem da fonte do sistema ter o carácter de estrela.

## Animação dos grãos (Fase 7 - continuação)
Pontos roxos aparecem sobre a waveform, movendo-se de acordo com a posição
real de leitura dos grãos ativos no motor granular - atualizado ~30x por
segundo. Só é visível quando o "Granular Mix" está acima de 0%.

## Otimização de CPU (Fase 8)
- Janela dos grãos (Hann): passou de calcular std::cos() em cada amostra de
  cada grão para uma tabela pré-calculada (1024 pontos) com interpolação —
  isto evitava até 128 chamadas de cosseno por amostra de áudio (16 grãos x
  8 vozes), agora é só uma leitura de tabela.
- Visualização dos grãos: passou de publicar a posição a cada amostra de
  áudio para publicar só uma vez por bloco (a UI só precisa de ~30
  atualizações por segundo, não 44100).
- Parâmetros lidos a cada amostra (Granular Mix, Grain Density) agora usam
  leitura atómica "relaxed" em vez do modo mais lento por omissão — seguro
  aqui porque não precisamos de sincronizar com mais nada.
- Removida uma limpeza de buffer duplicada no processBlock.
- Lado da UI: a lista de posições dos grãos deixou de ser recriada 30x por
  segundo, agora reutiliza a mesma memória.
- Reverb agora só corre o algoritmo quando o "Reverb Mix" está acima de 0%
  (Delay e Saturação já faziam isto).
- Rede de segurança no fim da cadeia: qualquer NaN/Inf (pode surgir de
  feedback extremo no delay/reverb) é substituído por silêncio, e picos
  extremos são limitados, antes de chegar ao host.

## Próximo passo
Fase 6b (biblioteca nativa de samples) ou Fase 9 (testes em diferentes
sample rates/DAWs), conforme preferires.
