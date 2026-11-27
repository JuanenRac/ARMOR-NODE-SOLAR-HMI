#!/usr/bin/env bash
# ARMOR-NODE-SOLAR-HMI - builds one variant of the firmware inside the ESP-IDF container and makes one file that is written at address 0.
# Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#
#   bash tools/build_firmware.sh NAME [VARIANT_FILE ...]      for example: rev1-newpanel   or   rev3-newpanel variants/rev3
#
# SOLAR_SETTINGS, when set, holds extra lines "CONFIG_SOLAR_NAME=value" separated by ";" to try another pin or timing without touching the code.
set -euo pipefail
name="$1"
shift
defaults="sdkconfig.defaults"
for file in "$@"; do defaults="$defaults;$file"; done
build="build/$name"
mkdir -p "$build" out
if [ -n "${SOLAR_SETTINGS:-}" ]; then
  : > "$build/extra"
  IFS=';' read -ra lines <<< "$SOLAR_SETTINGS"
  for line in "${lines[@]}"; do
    [[ "$line" =~ ^CONFIG_SOLAR_[A-Z0-9_]+=(y|n|-?[0-9]{1,6})$ ]] || { echo "not a setting of the screen: $line" >&2; exit 1; }
    echo "$line" >> "$build/extra"
  done
  defaults="$defaults;$build/extra"
fi
idf.py -B "$build" -D SDKCONFIG="$build/sdkconfig" -D "SDKCONFIG_DEFAULTS=$defaults" build
version="$(python -c 'import json; print(json.load(open("armor.project.json", encoding="utf-8"))["version"])')"
out="$PWD/out/armor-solar-hmi-$version-$name-complete.bin"
(cd "$build" && python -m esptool --chip esp32p4 merge_bin -o "$out" @flash_args)
cp "$build/armor_solar_hmi.bin" "out/armor-solar-hmi-$version-$name-app.bin"
grep -E '^CONFIG_(SOLAR_|ESP32P4_SELECTS|ESP32P4_REV_MIN_FULL)' "$build/sdkconfig" > "out/armor-solar-hmi-$version-$name-settings.txt"
(cd out && sha256sum ./*.bin > SHA256SUMS)
echo "FIRMWARE=BUILT $name"
