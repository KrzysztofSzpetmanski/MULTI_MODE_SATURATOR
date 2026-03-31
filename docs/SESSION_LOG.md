# Session Log

## 2026-03-31
- Utworzono pełny szkielet DSP/engine/platform (VCV + Daisy adapter).
- Dodano workflow produkcyjny: `Makefile`, auto-bump `2.0.xxx`, deploy targets, docs operacyjne.
- Dodano UI workflow z innych działających modułów:
  - etykiety panelowe rysowane w widget,
  - CV depth submenu na gałkach,
  - ring wskaźnika modulacji,
  - LED wskaźniki modulacji.
- Build i pakowanie zweryfikowane.
- Lokalny deploy działa.
- Deploy na drugi komputer wymaga działającego mount SMB (aktualnie niedostępny).
