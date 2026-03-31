# Multi Mode Saturator (Skeleton)

To repo **dopiero startuje** jako nowy projekt VCV Rack: **MULTI_MODE_SATURATOR**.

Aktualny stan kodu to **świadomy szkielet bazowy** sklonowany z wcześniejszego projektu filtrowego, aby zachować:
- sprawdzoną strukturę katalogów,
- workflow build/deploy,
- podział DSP core / engine / platform.

## Co jest teraz
- działający szkielet architektury,
- komplet plików produkcyjnych i informacyjnych,
- punkt startowy do wymiany modeli DSP na saturatory.

## Co będzie robione dalej
- implementacja modeli saturacji (kilka trybów),
- mapowanie parametrów specyficznych dla każdego trybu,
- testowanie brzmienia i stabilności w VCV,
- późniejszy port do Daisy Seed.

## Ważne
Ten etap to **nie gotowy saturator**, tylko fundament, na którym budujemy docelowy moduł.
