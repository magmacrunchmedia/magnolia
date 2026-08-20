#!/bin/sh
# Creates a new magnolia game beside this engine checkout.
#
#   tools/new-game.sh george-boole
#
# The Makefile it copies carries the bin2s asset-embedding rules and the deploy
# targets, which are the parts that are non-obvious and the parts that silently
# drift when they are retyped for each new game.
set -e

if [ -z "$1" ]; then
    echo "usage: $0 <game-name>" >&2
    echo "  game-name becomes the app directory and the Homebrew Channel entry," >&2
    echo "  so keep it lowercase with dashes: my-game" >&2
    exit 1
fi

GAME="$1"

case "$GAME" in
    *[!a-z0-9-]*)
        echo "error: '$GAME' has characters outside a-z, 0-9 and dash." >&2
        echo "  It becomes a directory name on an SD card and a symbol prefix," >&2
        echo "  and both are happier without spaces or capitals." >&2
        exit 1
        ;;
esac

ENGINE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
DEST=$(dirname "$ENGINE_DIR")/$GAME

if [ -e "$DEST" ]; then
    echo "error: $DEST already exists -- refusing to overwrite it." >&2
    exit 1
fi

echo "Creating $DEST"
mkdir -p "$DEST"
cp -R "$ENGINE_DIR/template/." "$DEST/"
mkdir -p "$DEST/sprites" "$DEST/audio"

# bin2s needs at least one asset or the build has nothing to embed and the
# generated assets.h is empty, which fails at the first include rather than here
# where the cause is obvious.
cat > "$DEST/sprites/README.md" <<'NOTE'
Drop PNGs here. They are embedded into the binary by bin2s at build time and
reachable as `<name>_png` / `<name>_png_size` from `assets.h`.
NOTE
cat > "$DEST/audio/README.md" <<'NOTE'
Drop raw PCM here (see the game README for the ffmpeg recipe). Embedded by bin2s
and reachable as `<name>_pcm` / `<name>_pcm_size` from `assets.h`.
NOTE

TODAY=$(date +%Y%m%d)
for f in Makefile meta.xml README.md source/main.c source/config.h; do
    sed -e "s/__GAME__/$GAME/g" -e "s/__DATE__/$TODAY/g" "$DEST/$f" > "$DEST/$f.tmp"
    mv "$DEST/$f.tmp" "$DEST/$f"
done

cat > "$DEST/.gitignore" <<'NOTE'
build/
sdcard/
*.dol
*.elf
*.map
.DS_Store
NOTE

echo ""
echo "Done. Next:"
echo "  cd $DEST"
echo "  make            # builds against ../magnolia"
echo "  make dolphin    # stage and push to Dolphin's SD folder"
echo ""
echo "The engine is expected at ../magnolia relative to the game, which is how"
echo "the Makefile's include paths are written."
