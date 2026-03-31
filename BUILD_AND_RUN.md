# Build And Run

## Build
```bash
cd /Users/lazuli/Documents/PROGRAMMING/VCV_PROGRAMMING/MULITI_MODE_FILTER
make -j4
```

## Package
```bash
make dist
```

Artefakty:
- `plugin.dylib`
- `dist/MultiModeFilterLab/`
- `dist/MultiModeFilterLab-2.0.xxx-<platform>.vcvplugin`

## Install lokalnie do Rack2
```bash
make install
```

## Deploy na dwa komputery
```bash
make deploy-both
```

## Smoke test
1. Otwórz VCV Rack 2 i dodaj `Dual Filter Lab`.
2. Podłącz sygnały do `IN A` i `IN B`, odsłuchuj `OUT A` i `OUT B`.
3. Przełączaj listy `MODEL A`, `MODEL B`, `MODE A`, `MODE B`, `ROUTING`.
4. Sprawdź `DUAL`, `SERIAL`, `PARALLEL`.
5. Podłącz CV do `A CUT CV` i `A RES CV`; right-click na gałce i zmieniaj depth.
6. Zweryfikuj LED wskaźniki modulacji i ring overlay na gałkach.

## Clean
```bash
make clean
```
