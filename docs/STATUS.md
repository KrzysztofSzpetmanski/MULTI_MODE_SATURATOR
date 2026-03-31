# MULITI_MODE_FILTER - Status

## Data
- Date: 2026-03-31
- Branch: `main`
- Repo: `https://github.com/KrzysztofSzpetmanski/MULITI_MODE_FILTER`
- Version: `2.0.007`
- Build panel: `BUILD 7`

## Co jest gotowe
- DSP common + filtry:
  - `SVF` (TPT), `TransistorLadder`, `Comb`, `Biquad`
  - `DiodeLadder` scaffold
  - `LPG` scaffold
- Runtime:
  - `DualFilterEngine` (slot A/B, bypass per slot, routing `A->B`, `B->A`, `PARALLEL`)
- VCV test module:
  - wybór modeli A/B
  - routing
  - parametry wspólne i modelowe
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
- `make dist`: OK (`dist/MultiModeFilterLab-2.0.007-mac-arm64.vcvplugin`)
- `make install`: OK (lokalna instalacja Rack2)
- `make deploy-both`:
  - local deploy: OK
  - big-mac deploy: FAIL (brak zamontowanego `/Volumes/music` / błąd uprawnień-mount)

## Otwarty dług techniczny
- Lepsze mapowanie model-specyficznych parametrów niż `p1..p4`.
- Oversampling switch dla mocniejszych nieliniowości.
- Dodatkowe testy regresji dla stability przy szybkiej modulacji.
