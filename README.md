# Zenith Granular — Fase 4 (FX: Filtro, Saturação, Delay, Reverb)

Cadeia de efeitos completa, cada um no seu próprio ficheiro (FilterEngine,
SaturationEngine, DelayEngine, ReverbEngine), aplicada depois do
sampler/granular, antes da saída — mesmo princípio de organização usado por
romplers profissionais (voz isolada → FX partilhado no fim).

## O que este código faz
- Regista o plugin como **instrumento** (Channel Rack do FL Studio).
- Botão **"Load Sample..."** ou **arrastar e largar** um ficheiro de áudio.
- Formatos suportados: WAV, AIFF, FLAC, OGG, MP3, M4A, WMA.
- Sampler: Attack, Decay, Sustain, Release, Pitch, Fine Tune.
- Granulador: Grain Size, Density, Position, Position Random, Grain Pitch,
  Pitch Random, Pan Spread, Granular Mix.
- Filtro: Low-Pass/High-Pass/Band-Pass, Cutoff, Resonance.
- Saturação: Soft Clip/Tape/Tube, Drive, Mix.
- Delay: Time (ms), Feedback, Mix. (sync ao BPM fica para depois)
- Reverb: Size, Damping, Mix.
- O sample carregado é lembrado entre sessões do projeto `.flp`.

## Próximo passo (Fase 5/6)
Modulação (LFOs, envelope de modulação, macros) e sistema de presets.

## Como compilar (Windows, gratuito)
1. Instala o [CMake](https://cmake.org/download/) e o
   [Visual Studio Community](https://visualstudio.microsoft.com/) (gratuito,
   com a carga de trabalho "Desenvolvimento para desktop com C++").
2. Abre o **cmd** (Prompt de Comando) dentro desta pasta e corre exatamente
   estes dois comandos, um de cada vez (o `--config Release` no segundo é
   obrigatório — sem ele o Visual Studio compila em Debug e o ficheiro
   fica noutra pasta):
   ```
   cmake -B build
   cmake --build build --config Release
   ```
3. Na primeira vez, o CMake vai descarregar o JUCE automaticamente
   (precisa de internet só nesse passo).
4. O ficheiro gerado chama-se `ZenithGranular.vst3` e fica em:
   `build\ZenithGranular_artefacts\Release\VST3\ZenithGranular.vst3`
5. Como o CMakeLists já tem `COPY_PLUGIN_AFTER_BUILD TRUE`, ele também é
   copiado automaticamente para `C:\Program Files\Common Files\VST3`.
   Se não aparecer lá (às vezes o Windows bloqueia por permissão), copia
   à mão da pasta do passo 4 para essa pasta.

**Se não encontrares o ficheiro**, abre o cmd na pasta do projeto e corre
`dir /s /b *.vst3` — isso procura em todas as subpastas e mostra o caminho
exato de qualquer `.vst3` gerado, mesmo que tenha ficado em Debug.


