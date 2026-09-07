# Zenith Granular — Fase 2 (SamplerEngine)

Este é o esqueleto compilável do plugin, agora com o `SamplerEngine`:
carrega um `.wav`, toca por nota MIDI, com ADSR e pitch em tempo real.

## O que este código faz
- Regista o plugin como **instrumento** (aparece no Channel Rack do FL Studio,
  não no Mixer).
- Botão **"Load Sample..."** na UI abre o explorador de ficheiros e carrega
  qualquer `.wav`.
- Toca o sample por nota MIDI (a nota C4/60 toca no pitch original; outras
  notas transpõem automaticamente).
- 6 knobs ligados em tempo real e automatizáveis pelo DAW: Attack, Decay,
  Sustain, Release, Pitch, Fine Tune.

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

## Próximo passo (Fase 3)
Implementar o `GranularEngine`: dividir o sample em grãos, com grain size,
density, position e pitch controláveis.
