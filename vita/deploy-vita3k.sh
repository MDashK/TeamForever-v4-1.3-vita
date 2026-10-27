#!/bin/bash
# Installs the built VPKs + game data into Vita3K's emulated ux0: (run from Git Bash on Windows).
#   ./deploy-vita3k.sh            -> apps + data
#   ./deploy-vita3k.sh apps       -> only re-install the apps (eboot/sce_sys)
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"          # E:/_sonic1f_sonic2a_vita
ENGINE="$ROOT/TeamForever-v4-1.3-foreve"
UX0="${VITA3K_UX0:-$APPDATA/Vita3K/Vita3K/ux0}"

install_app() { # vpk titleid
    local dst="$UX0/app/$2"
    rm -rf "$dst" && mkdir -p "$dst"
    unzip -q -o "$1" -d "$dst"
    echo "installed $2"
}

install_app "$ENGINE/build-vita/Sonic1Forever.vpk"  SNCF00001
install_app "$ENGINE/build-vita/Sonic2Absolute.vpk" SNCA00002
[ "$1" = "apps" ] && exit 0

# Sonic 1 Forever: Data.rsdk + mods/Sonic 1 Forever (already a mod)
S1="$UX0/data/Sonic1Forever"
mkdir -p "$S1"
cp -u "$ROOT/s1_data/Data.rsdk" "$S1/"
cp -ru "$ROOT/s1_data/mods" "$S1/"

# Sonic 2 Absolute: the loose Data/ + Scripts/ only override Data.rsdk when they come from a mod,
# so they're installed as mods/Sonic 2 Absolute
S2="$UX0/data/Sonic2Absolute"
MOD2="$S2/mods/Sonic 2 Absolute"
mkdir -p "$MOD2"
cp -u "$ROOT/s2_data/Data.rsdk" "$S2/"
cp -ru "$ROOT/s2_data/Data" "$ROOT/s2_data/Scripts" "$MOD2/"
cp -u "$ROOT/S2Absolute-main/Mods/Sonic 2 Absolute/mod.ini" "$MOD2/"
printf '[mods]\nSonic 2 Absolute=true\n' > "$S2/mods/modconfig.ini"
echo "data deployed to $UX0/data"
