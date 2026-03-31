# MULITI_MODE_FILTER - Status

## Data
- Date: 2026-03-31
- Branch: `main`
- Repo: `https://github.com/KrzysztofSzpetmanski/MULITI_MODE_FILTER`
- Version: `2.0.010`
- Build panel: `BUILD 10`

## Co jest gotowe
- DSP common + filtry:
  - `SVF` (TPT), `TransistorLadder`, `Comb`, `Biquad`
  - `DiodeLadder` scaffold
  - `LPG` scaffold
- Runtime:
  - `DualFilterEngine` (slot A/B, bypass per slot, routing `DUAL`, `SERIAL`, `PARALLEL`)
- VCV test module:
  - wybór modeli A/B jako listy rozwijane
  - wybór mode A/B jako listy rozwijane (dynamiczne per model)
  - routing jako lista rozwijana `DUAL/SERIAL/PARALLEL`
  - `SERIAL/PARALLEL`: oba wejścia akceptowane bez zgadywania (`IN A`, `IN B`, lub oba), oba wyjścia klonowane
  - osobne `IN A/B` i `OUT A/B`
  - CV wejścia obu modeli na dole panelu
  - CV modulacja cutoff/resonance dla A/B
  - visual modulation ring na gałkach
  - submenu głębokości CV (right-click)
  - LED wskaźniki modulacji
- Daisy adapter:
  - `src/platform/daisy/DaisyAdapter.*`

## Co jest teraz priorytetem
1. Strojenie charakteru modeli pod odsłuch.
2. Dopracowanie `LPG` i `DiodeLadder` z prostego scaffold do wersji muzycznej.
3. Opcjonalny stereo wrapper w module VCV (na razie moduł testowy jest mono).

## Ostatnia walidacja (2026-03-31)
- `make -j4`: OK
- `make dist`: OK (`dist/MultiModeFilterLab-2.0.010-mac-arm64.vcvplugin`)
- `make install`: OK (lokalna instalacja Rack2)
- `make deploy-both`:
  - local deploy: OK
  - big-mac deploy: OK (`/Volumes/music` zamontowane)

## Otwarty dług techniczny
- Lepsze mapowanie model-specyficznych parametrów niż `p1..p4`.
- Oversampling switch dla mocniejszych nieliniowości.
- Dodatkowe testy regresji dla stability przy szybkiej modulacji.
