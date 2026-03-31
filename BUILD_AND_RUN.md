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
2. Podłącz sygnał do `IN`, odsłuch na `OUT`.
3. Przełączaj `MODEL A`, `MODEL B`, `ROUTING`.
4. Sprawdź `A->B`, `B->A`, `PARALLEL`.
5. Podłącz CV do `A CUT CV` i `A RES CV`; right-click na gałce i zmieniaj depth.
6. Zweryfikuj LED wskaźniki modulacji i ring overlay na gałkach.

## Clean
```bash
make clean
```
