# Architecture

## Warstwy
1. `src/dsp/common`
- Narzędzia DSP (math, smoothing, saturacja, delay, dc block).

2. `src/dsp/filters`
- Niezależne modele filtrów, sample-by-sample, bez zależności VCV/Daisy.

3. `src/dsp/engine`
- `DualFilterEngine`: slot A/B, bypass per slot, routing serial/parallel.

4. `src/platform/vcv`
- Moduł testowy do odsłuchu i strojenia.

5. `src/platform/daisy`
- Adapter pokazujący użycie engine w callbacku audio.

## Główne założenia
- Brak dynamicznej alokacji w ścieżce audio.
- DSP core przenośny między VCV i Daisy.
- Minimalne abstrahowanie w hot-path (dispatch po `switch`, bez virtuali w process).

## Rozszerzenia zaplanowane
- Lepsze modele `DiodeLadder` i `LPG`.
- Refinement tuning/resonance ladderów.
- Opcjonalny oversampling dla cięższych nieliniowości.
