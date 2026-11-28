// ARMOR-NODE-SOLAR-HMI - whether a downloaded image may be installed. Next to the image its server keeps `<image>.sha256` and `<image>.sig2`, the signature of the manifest of the image
// (project, board, version and hash) with the key of the A.R.M.O.R. releases (core/fw_trust.hpp). The image must be of this program, for this board, newer than the one that runs, hash to
// the published hash and carry that signature. Pure: tested on a computer.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstddef>
#include <string>
#include <string_view>

#include "fw_trust.hpp"
#include "semver.hpp"

namespace armor::solarhmi {

// The name of the build a screen runs: the images of the two chip revisions and the two panels are not interchangeable.
inline std::string board_name(bool revision3, bool old_panel) {
  return std::string("jc1060-") + (revision3 ? "rev3" : "rev1") + (old_panel ? "-oldpanel" : "-newpanel");
}

// The 64 hexadecimal digits at the start of a `.sha256` file, in lower case; "" when it does not start with them.
inline std::string published_hash(std::string_view text) {
  if (text.size() < 64) return "";
  std::string out;
  for (std::size_t i = 0; i < 64; ++i) {
    const char c = text[i];
    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) out += c;
    else if (c >= 'A' && c <= 'F') out += static_cast<char>(c - 'A' + 'a');
    else return "";
  }
  if (text.size() > 64 && text[64] != ' ' && text[64] != '\t' && text[64] != '\r' && text[64] != '\n' && text[64] != '*') return "";
  return out;
}

struct ImageFacts {
  std::string project, running_project;
  std::string board;
  std::string version, running_version;
  std::string hash, published;     // of what was written, and what the server published
  std::string signature;           // the text of `<image>.sig2`
};

// "" when the image may be installed; otherwise the first reason that stops it.
inline std::string refusal(const ImageFacts& facts) {
  if (facts.published.empty()) return "no_published_hash";
  if (facts.hash != facts.published) return "hash_differs";
  if (facts.project != facts.running_project) return "other_program";
  if (!semver::is_newer(facts.version, facts.running_version)) return "not_newer";
  if (!fw_trust::manifest_signature_ok(facts.project, facts.board, facts.version, facts.hash, facts.signature)) return "bad_signature";
  return "";
}

}  // namespace armor::solarhmi
