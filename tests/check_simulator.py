#!/usr/bin/env python3
"""Every reading the simulator prints follows the message schemas of ARMOR-COMMON (solar_battery, solar_inverter), so that the screen is never developed against made-up shapes.

    python tests/check_simulator.py path/to/solar_sim

Needs ARMOR-COMMON next to this repository; without it the check is skipped, unless ARMOR_REQUIRE_SIBLINGS says that a skip is a failure.
Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
from pathlib import Path

SCHEMAS = Path(__file__).resolve().parents[2] / "ARMOR-COMMON" / "src" / "armor_common" / "schemas"
KNOWN = {"$schema", "$id", "title", "description", "type", "required", "properties", "additionalProperties", "items", "enum", "const",
         "minimum", "maximum", "minItems", "maxItems", "minLength", "maxLength", "pattern", "maxProperties", "propertyNames"}
TYPES = {"object": dict, "array": list, "string": str, "boolean": bool, "integer": int, "number": (int, float), "null": type(None)}


def problems(value: object, schema: dict, where: str) -> list[str]:
    """What is wrong with a value, for the keywords these schemas use; a keyword this check does not know is itself a problem."""
    found = [f"{where}: the schema uses {key}, which this check does not read" for key in schema if key not in KNOWN]
    kinds = schema.get("type")
    if kinds is not None:
        names = kinds if isinstance(kinds, list) else [kinds]
        fits = any(isinstance(value, TYPES[name]) and not (name in ("integer", "number") and isinstance(value, bool)) for name in names)
        if not fits:
            return found + [f"{where}: {value!r} is not {kinds}"]
    if "const" in schema and value != schema["const"]:
        found.append(f"{where}: {value!r} is not {schema['const']!r}")
    if "enum" in schema and value not in schema["enum"]:
        found.append(f"{where}: {value!r} is not one of {schema['enum']}")
    if isinstance(value, (int, float)) and not isinstance(value, bool):
        if value < schema.get("minimum", value) or value > schema.get("maximum", value):
            found.append(f"{where}: {value} is out of range")
    if isinstance(value, str):
        if len(value) < schema.get("minLength", 0) or len(value) > schema.get("maxLength", len(value)):
            found.append(f"{where}: the length of {value!r} is out of range")
        if "pattern" in schema and re.search(schema["pattern"], value) is None:
            found.append(f"{where}: {value!r} does not match {schema['pattern']}")
    if isinstance(value, list):
        if len(value) < schema.get("minItems", 0) or len(value) > schema.get("maxItems", len(value)):
            found.append(f"{where}: the number of items is out of range")
        for index, item in enumerate(value):
            found += problems(item, schema.get("items", {}), f"{where}[{index}]")
    if isinstance(value, dict):
        found += [f"{where}: {name} is missing" for name in schema.get("required", []) if name not in value]
        known = schema.get("properties", {})
        extra = schema.get("additionalProperties", True)
        if len(value) > schema.get("maxProperties", len(value)):
            found.append(f"{where}: too many members")
        for name in value:
            found += problems(name, schema.get("propertyNames", {}), f"{where} name")
        for name, item in value.items():
            if name in known:
                found += problems(item, known[name], f"{where}.{name}")
            elif extra is False:
                found.append(f"{where}: {name} is not in the contract")
            elif isinstance(extra, dict):
                found += problems(item, extra, f"{where}.{name}")
    return found


def main() -> int:
    if not SCHEMAS.is_dir():
        if os.environ.get("ARMOR_REQUIRE_SIBLINGS", "") not in ("", "0"):
            print("SIMULATOR_CONTRACT=FAIL ARMOR-COMMON is not next to this repository", file=sys.stderr)
            return 1
        print("SIMULATOR_CONTRACT=SKIPPED ARMOR-COMMON is not next to this repository")
        return 0
    schemas = {kind: json.loads((SCHEMAS / f"solar_{kind}.schema.json").read_text(encoding="utf-8")) for kind in ("battery", "inverter")}
    printed = subprocess.run([sys.argv[1]], capture_output=True, text=True, check=True, timeout=60).stdout
    found: list[str] = []
    readings = 0
    for number, line in enumerate(printed.splitlines(), 1):
        for device in json.loads(line)["devices"]:
            readings += 1
            found += problems(device["reading"], schemas[device["kind"]], f"answer {number} {device['device']}")
    if not readings:
        found.append("the simulator printed no reading")
    if found:
        print("SIMULATOR_CONTRACT=FAIL", *found[:20], sep="\n  ", file=sys.stderr)
        return 1
    print(f"SIMULATOR_CONTRACT=PASS {readings} readings")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
