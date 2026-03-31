# Project Handover

## Start point dla nowej osoby / nowego czatu
1. Sprawdź status projektu w `docs/STATUS.md`.
2. Przeczytaj `ARCHITECTURE.md` i `README.md`.
3. Zrób build `make -j4` i szybki smoke test wg `TEST_CHECKLIST.md`.

## Gdzie jest logika
- DSP: `src/dsp/*`
- Engine/routing: `src/dsp/engine/*`
- VCV module: `src/platform/vcv/*`
- Daisy adapter: `src/platform/daisy/*`

## Workflow wersji
- Każdy build podnosi `plugin.json` z `2.0.xxx` -> `2.0.(xxx+1)`.
- Ten sam skrypt aktualizuje `kBuildNumber` w `DualFilterModule.h`.

## Aktualna wersja robocza
- `plugin.json`: `2.0.007`
- panel: `BUILD 7`
- ostatni pakiet: `dist/MultiModeFilterLab-2.0.007-mac-arm64.vcvplugin`

## Najważniejsze decyzje techniczne
- Brak dynamicznej alokacji w audio path.
- Prosty static-dispatch po modelu (bez virtual call w hot path).
- VCV jest warstwą testową; DSP core bez zależności na GUI/hardware.
