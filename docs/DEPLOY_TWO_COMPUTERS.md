# Deploy On Two Computers

Workflow dla dwóch maszyn z jednym repo.

## Założenia
- Repo: `MULITI_MODE_FILTER`
- Obie maszyny mają: `git`, `make`, Rack SDK, Rack 2.

## Build i deploy
```bash
cd /Users/lazuli/Documents/PROGRAMMING/VCV_PROGRAMMING/MULITI_MODE_FILTER
make -j4
make deploy-both
```

Domyślne deploy path:
- lokalnie: `~/Library/Application Support/Rack2/plugins-mac-arm64/MultiModeFilterLab`
- big-mac: `/Volumes/music/Library/Application Support/Rack2/plugins-mac-arm64/MultiModeFilterLab`

Jeśli `/Volumes/music` nie jest dostępne, użyj mount w katalogu użytkownika i nadpisz path:
```bash
mkdir -p "$HOME/Volumes/music"
# mount_smbfs <twoj_smb_url> "$HOME/Volumes/music"
make BIG_MAC_MOUNT_DIR="$HOME/Volumes/music" deploy-both
```

## Workflow git
Komputer A:
```bash
git checkout -b codex/<nazwa-zmiany>
# zmiany
make -j4
git add .
git commit -m "<opis>"
git push -u origin codex/<nazwa-zmiany>
```

Po merge do `main`:
```bash
git checkout main
git pull
```

Komputer B:
```bash
git checkout main
git pull
make -j4
make deploy-both
```
