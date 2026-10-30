#!/usr/bin/env python3
# DFToolBox - Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Generate df-directory.txt from a Dreamfall installation.

The .pak directory is a double-array trie without a check array:
looking up a known name is cheap and reliable, but walking it
backwards gives many possible names per file. This script collects
evidence for the real names from the game itself:

  string   full pathnames found in the game data and executable
  font     font pages listed in the art/gui/font/*.dat descriptors
  sibling  same name as a confirmed file, different extension
           (e.g. the .lip files belonging to voice-over .mp3s)
  basename basename found in the game data, directory confirmed
  hex      generated scripts with hex suffixes in a confirmed directory
  vocabulary the only possible name that consists entirely of words
           used in confirmed names, this is a well informed guess

Every candidate is verified against the trie. Files without any
evidence are left out and get guessed by dftoolbox at runtime.

Usage: generate-filelist.py DREAMFALL_DIR > df-directory.txt
"""

import argparse
import collections
import glob
import os
import re
import struct
import sys

NAMETABLE = "\0abcdefghijklmnopqrstuvwxyz/??-_'.0123456789"
CHAR2CODE = {c: i for i, c in enumerate(NAMETABLE) if c not in "\0?"}
NAMETABLE_CHARS = len(NAMETABLE)


class Pak:
    def __init__(self, filename):
        self.filename = filename
        with open(filename, "rb") as f:
            header = f.read(24)
            if header[:12] != b"tlj_pack0001":
                raise RuntimeError("not a TLJ pak: " + filename)
            file_count, _, byte_count = struct.unpack_from("<3i", header, 12)
            data = f.read(file_count * 20 + byte_count)
        self.entries = [struct.unpack_from("<5i", data, i * 20) for i in range(file_count)]
        raw = data[file_count * 20:]
        self.nametable = "".join(NAMETABLE[b] if b < NAMETABLE_CHARS else "?" for b in raw)
        self.parents = None

    def segment(self, i):
        start = self.entries[i][4]
        return self.nametable[start:self.nametable.index("\0", start)]

    def is_file(self, i):
        return self.entries[i][1] != 0

    def files(self):
        return [i for i in range(len(self.entries)) if self.is_file(i)]

    def lookup(self, pathname):
        """Exact lookup of a lowercase pathname, returns the entry or -1"""
        n = len(self.entries)
        if not pathname or pathname[0] not in CHAR2CODE:
            return -1
        i = CHAR2CODE[pathname[0]]
        pos = 1
        for _ in range(64):
            if i < 0 or i >= n:
                return -1
            offset, filesize, base, path_length, _ = self.entries[i]
            if not (filesize or base or path_length or self.entries[i][4]):
                return -1
            seg = self.segment(i)
            if pathname[pos:pos + len(seg)] != seg:
                return -1
            pos += len(seg)
            if filesize:
                return i if pos == len(pathname) and path_length == pos else -1
            if pos >= len(pathname) or path_length != pos - 1 or pathname[pos] not in CHAR2CODE:
                return -1
            i = base + CHAR2CODE[pathname[pos]]
            pos += 1
        return -1

    def guesses(self, i, limit=4096, dir_suffixes=None):
        """All pathnames that the trie allows for entry i, if
        dir_suffixes is given only pathnames whose directory ends with
        one of them are returned"""
        if self.parents is None:
            self.parents = collections.defaultdict(list)
            for d, e in enumerate(self.entries):
                if e[1] == 0 and any(e[1:]):
                    for c in range(1, NAMETABLE_CHARS):
                        self.parents[e[2] + c].append((d, c))
        path_length = self.entries[i][3]
        result = []
        stack = [(i, self.segment(i))]
        while stack and len(result) < limit:
            source, name = stack.pop()
            if len(name) + 1 == path_length:
                if 0 < source < NAMETABLE_CHARS:
                    result.append(NAMETABLE[source] + name)
                continue
            for d, c in self.parents[source]:
                if self.entries[d][3] + len(name) + 2 == path_length:
                    candidate = self.segment(d) + NAMETABLE[c] + name
                    if dir_suffixes is not None:
                        # the directories completed so far must be known
                        first = candidate.find("/")
                        last = candidate.rfind("/")
                        if first != last and candidate[first + 1:last] not in dir_suffixes:
                            continue
                    stack.append((d, candidate))
        return [g for g in result if "?" not in g]

    def read(self, i, size=None):
        offset, filesize = self.entries[i][:2]
        with open(self.filename, "rb") as f:
            f.seek(offset)
            return f.read(filesize if size is None else min(size, filesize))


STRING_RX = re.compile(rb"[A-Za-z0-9_\-'./\\#]{4,}")
SKIP_MAGIC = (b"DDS ", b"RIFF", b"ID3", b"\xff\xfb", b"\xff\xfa", b"\xff\xf3")


def harvest_strings(data, out):
    for m in STRING_RX.finditer(data):
        s = m.group().decode("latin1").replace("\\", "/")
        for part in s.split("#"):
            part = part.strip("./")
            if len(part) >= 4:
                out.add(part)


def collect_strings(paks, gamedir):
    strings = set()
    for pak in paks:
        with open(pak.filename, "rb") as f:
            for i in pak.files():
                offset, size = pak.entries[i][:2]
                f.seek(offset)
                head = f.read(4)
                if head.startswith(SKIP_MAGIC):
                    continue
                f.seek(offset)
                harvest_strings(f.read(size), strings)
    for filename in glob.glob(os.path.join(gamedir, "*.exe")) + glob.glob(os.path.join(gamedir, "*.dll")):
        with open(filename, "rb") as f:
            harvest_strings(f.read(), strings)
    return strings


def font_names(paks):
    """Font descriptors start with the font name in a 128 byte field,
    followed by the point size and the number of texture pages"""
    names = set()
    for pak in paks:
        for i in pak.files():
            head = pak.read(i, 136)
            if head.startswith(b"art/gui/font/") and len(head) == 136:
                name = head[:head.index(b"\0")].decode("latin1")
                size, pages = struct.unpack_from("<2i", head, 128)
                if 0 < size < 1000 and 0 <= pages < 1000:
                    names.add("%s_%d.dat" % (name, size))
                    for page in range(pages):
                        names.add("%s_%d_%d.tga" % (name, size, page))
    return names


def split(pathname):
    d, _, b = pathname.rpartition("/")
    return d, b


def stem(pathname):
    d, b = split(pathname)
    return d + "/" + b.rsplit(".", 1)[0] if "." in b else pathname


HEX_RX = re.compile(r"_[0-9a-f]+\.[a-z0-9]+$")
TOKEN_RX = re.compile(r"[^a-z0-9]+")


def tokens(basename):
    return [t for t in TOKEN_RX.split(basename) if t]


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("gamedir", help="Dreamfall installation directory")
    parser.add_argument("--stats", action="store_true", help="print statistics to stderr")
    args = parser.parse_args()

    paks = [Pak(f) for f in sorted(glob.glob(os.path.join(args.gamedir, "bin", "res", "*.pak")))
            if os.path.basename(f) != "resource.pak"]
    if not paks:
        sys.exit("error: no .pak files found in " + os.path.join(args.gamedir, "bin", "res"))

    def log(*msg):
        if args.stats:
            print(*msg, file=sys.stderr, flush=True)

    log("harvesting strings")
    strings = collect_strings(paks, args.gamedir)
    log("  %d strings" % len(strings))

    # lowercase -> spelling used in the game data
    spelling = {}
    for s in strings:
        spelling.setdefault(s.lower(), s)
    basenames = {}
    for low, s in spelling.items():
        basenames.setdefault(split(low)[1], split(s)[1])

    # resolved[pak index][entry] = (pathname, evidence)
    resolved = [dict() for _ in paks]

    def accept(names, evidence):
        count = 0
        for name in names:
            low = name.lower()
            for k, pak in enumerate(paks):
                j = pak.lookup(low)
                if j >= 0 and j not in resolved[k]:
                    resolved[k][j] = (name, evidence)
                    count += 1
        return count

    log("resolving")
    # only strings that look like full paths can match directly
    log("  string: %d" % accept((s for s in spelling.values() if "/" in s and "." in s), "string"))
    log("  font: %d" % accept(font_names(paks), "font"))

    # voice-over lip sync data is named after the sound file
    log("  sibling: %d" % accept([stem(n) + ".lip" for r in resolved for n, _ in r.values()
                                   if n.lower().endswith((".mp3", ".wav"))], "sibling"))

    # match the remaining files against the names the trie allows for them
    while True:
        names = [n for r in resolved for n, _ in r.values()]
        dirs = set(split(n.lower())[0] for n in names)
        dir_suffixes = set()
        for d in dirs:
            parts = d.split("/")
            for j in range(len(parts)):
                dir_suffixes.add("/".join(parts[j:]))
        stems = set(stem(n.lower()) for n in names)

        counts = collections.Counter()
        for k, pak in enumerate(paks):
            for i in pak.files():
                if i in resolved[k]:
                    continue
                gs = [g for g in pak.guesses(i, dir_suffixes=dir_suffixes) if split(g)[0] in dirs]
                for evidence, test in (("sibling",  lambda g: stem(g) in stems),
                                       ("basename", lambda g: split(g)[1] in basenames),
                                       ("hex",      lambda g: HEX_RX.search(g))):
                    match = [g for g in gs if test(g)]
                    if len(match) == 1:
                        d, b = split(match[0])
                        resolved[k][i] = (d + "/" + basenames.get(b, b), evidence)
                        counts[evidence] += 1
                        break
        log("  %s" % dict(counts))
        if not counts:
            break

    # last resort: pick the only name that is made of known words
    vocabulary = set()
    for r in resolved:
        for n, _ in r.values():
            vocabulary.update(tokens(split(n.lower())[1]))

    count = 0
    for k, pak in enumerate(paks):
        for i in pak.files():
            if i in resolved[k]:
                continue
            match = [g for g in pak.guesses(i, dir_suffixes=dir_suffixes)
                     if split(g)[0] in dirs and all(t in vocabulary for t in tokens(split(g)[1]))]
            if len(match) == 1:
                resolved[k][i] = (match[0], "vocabulary")
                count += 1
    log("  vocabulary: %d" % count)

    total = sum(len(p.files()) for p in paks)
    found = sum(len(r) for r in resolved)
    log("resolved %d of %d files (%.1f%%)" % (found, total, 100.0 * found / total))
    log("evidence: %s" % dict(collections.Counter(e for r in resolved for _, e in r.values())))

    for name in sorted(set(n for r in resolved for n, _ in r.values())):
        print(name)


if __name__ == "__main__":
    main()
