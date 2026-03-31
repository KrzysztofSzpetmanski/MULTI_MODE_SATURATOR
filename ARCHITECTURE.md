# Architecture (Start Skeleton)

## Cel
Budujemy nowy moduł: **Multi Mode Saturator**.

## Obecny stan
Obecny kod to szkielet pochodzący z projektu dual-filter, użyty jako baza infrastrukturalna.

## Warstwy (docelowo)
- `src/dsp/common` - utility DSP (smoothers, math, saturatory pomocnicze).
- `src/dsp/models` / `src/dsp/filters` - modele saturacji (do migracji nazewnictwa w kolejnych krokach).
- `src/dsp/engine` - runtime, sloty modeli, routing i mapowanie parametrów.
- `src/platform/vcv` - moduł testowy Rack.
- `src/platform/daisy` - adapter pod `AudioCallback`.

## Założenie projektowe
Nie robimy „jednego super saturatora”, tylko zestaw wyspecjalizowanych modeli.

## Status
Architektura jest gotowa jako **szkielet**, modelowanie saturatora dopiero się rozpoczyna.
