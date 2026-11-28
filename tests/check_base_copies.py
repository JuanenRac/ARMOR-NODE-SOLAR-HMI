#!/usr/bin/env python3
"""The files of core/ that come from the shared firmware base of ARMOR-COMMON are still the same as their source (until this repository is one of the nodes that
`sync_firmware_base.py` writes, this check is what stops the two from drifting apart).

Needs ARMOR-COMMON next to this repository; without it the check is skipped, unless ARMOR_REQUIRE_SIBLINGS says that a skip is a failure.
Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
"""
from __future__ import annotations

import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT.parent / "ARMOR-COMMON" / "firmware_base" / "core"
SHARED = ("json.hpp", "hmac_sha256.hpp", "secret_seal.hpp", "fw_trust.hpp", "ed25519_verify.hpp", "semver.hpp")
NAMES = {"@PROJECT@": "ARMOR-NODE-SOLAR-HMI", "@project-@": "armor-solar-hmi", "@project_@": "armor_solar_hmi", "@kind@": "solar-hmi"}


def main() -> int:
    if not BASE.is_dir():
        if os.environ.get("ARMOR_REQUIRE_SIBLINGS", "") not in ("", "0"):
            print("SHARED_FILES=FAIL ARMOR-COMMON is not next to this repository", file=sys.stderr)
            return 1
        print("SHARED_FILES=SKIPPED ARMOR-COMMON is not next to this repository")
        return 0
    changed = []
    for name in SHARED:
        source = (BASE / name).read_text(encoding="utf-8")
        for mark, value in NAMES.items():
            source = source.replace(mark, value)
        if (ROOT / "core" / name).read_text(encoding="utf-8") != source:
            changed.append(name)
    if changed:
        print(f"SHARED_FILES=FAIL differs from ARMOR-COMMON/firmware_base/core: {', '.join(changed)}", file=sys.stderr)
        return 1
    print(f"SHARED_FILES=PASS {len(SHARED)} files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
