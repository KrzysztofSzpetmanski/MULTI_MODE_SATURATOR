# Test Checklist

## DSP / Engine
- [ ] `SVF`: LP/HP/BP/NOTCH działają i nie wybuchają przy szybkiej modulacji cutoff.
- [ ] `TransistorLadder`: tapy 1/2/3/4 pole działają.
- [ ] `Comb`: FF/FB/LP-FB działają i nie destabilizują się przy feedback ~0.9.
- [ ] `Biquad`: LP/HP/BP/NOTCH/PEAK działają.
- [ ] Routing: `DUAL`, `SERIAL`, `PARALLEL` poprawny.

## VCV UI
- [ ] Zmiana modelu A/B w locie działa bez crash.
- [ ] Submenu depth (right-click) aktualizuje modulację.
- [ ] Ring wskaźnika modulacji jest widoczny po podłączeniu CV.
- [ ] LED-y modulacji reagują na podłączone CV i depth.
- [ ] Build number jest widoczny na panelu.

## Release
- [ ] `make -j4` przechodzi.
- [ ] `make dist` tworzy `.vcvplugin` z wersją `2.0.xxx`.
- [ ] `make deploy-both` przechodzi (przy zamontowanym `/Volumes/music`).
