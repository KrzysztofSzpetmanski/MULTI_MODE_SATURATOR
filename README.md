# Multi Mode Filter Lab (VCV + Daisy-ready)

Modularny system filtrów z architekturą warstwową:
- DSP core niezależny od platformy,
- runtime `DualFilterEngine` (sloty A/B + routing),
- warstwa testowa VCV Rack,
- adapter pod Daisy Seed.

## Co działa teraz
- Filtry działające: `SVF`, `TransistorLadder`, `Comb`, `Biquad`.
- Stuby pod dalszy rozwój: `DiodeLadder`, `LPG`.
- Routing: `DUAL`, `SERIAL`, `PARALLEL`.
- `SERIAL` i `PARALLEL`:
  - biorą sygnał z `IN A` lub `IN B` (albo merge obu),
  - dają sklonowany wynik na `OUT A` i `OUT B`.
- Bypass nie ma osobnych przełączników:
  - wyłączenie slotu robi się przez `MODEL = OFF` w dropdownie.
- Testowy moduł VCV: `Dual Filter Lab`.
- Layout I/O:
  - osobne `IN A`, `IN B` po lewej,
  - osobne `OUT A`, `OUT B` po prawej,
  - CV dla obu modeli na dole panelu.
- CV modulacja cutoff/resonance z:
  - ring indicator na gałce,
  - submenu głębokości CV (right-click),
  - LED wskaźnikiem aktywnej modulacji.

## Wersjonowanie
- Wersja pluginu: `2.0.xxx`.
- `xxx` = auto-increment build przy `make`/`make dist`.
- Numer buildu jest też wyświetlany na panelu modułu.

## Build
Szczegóły: `BUILD_AND_RUN.md`

Szybko:
```bash
cd /Users/lazuli/Documents/PROGRAMMING/VCV_PROGRAMMING/MULITI_MODE_FILTER
make -j4
make dist
make install
```

## Dokumentacja
- `SETUP.md`
- `BUILD_AND_RUN.md`
- `ARCHITECTURE.md`
- `KNOWN_ISSUES.md`
- `TEST_CHECKLIST.md`
- `NEW_CHAT_PROMPT.md`
- `docs/STATUS.md`
- `docs/SESSION_LOG.md`
- `docs/DEPLOY_TWO_COMPUTERS.md`
- `docs/GITHUB_SETUP.md`
- `docs/PROJECT_HANDOVER.md`
