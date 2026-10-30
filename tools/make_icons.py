#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

"""Generates the hicolor icons and the Windows .ico files from
icons/dftoolbox.png and icons/dfmodtool.png. Run from the repository
root, the results are committed."""

from pathlib import Path

from PIL import Image

HICOLOR_SIZES = [16, 32, 48, 64, 128, 256]
ICO_SIZES = [16, 24, 32, 48, 64, 256]


def scaled(source, size):
    return source.resize((size, size), Image.Resampling.LANCZOS)


def main():
    icons = Path("icons")

    dftoolbox = Image.open(icons / "dftoolbox.png").convert("RGBA")
    for size in HICOLOR_SIZES:
        directory = icons / "hicolor" / f"{size}x{size}" / "apps"
        directory.mkdir(parents=True, exist_ok=True)
        scaled(dftoolbox, size).save(directory / "dftoolbox.png")

    for name in ["dftoolbox", "dfmodtool"]:
        source = Image.open(icons / f"{name}.png").convert("RGBA")
        scaled(source, 256).save(icons / f"{name}.ico",
                                 sizes=[(size, size) for size in ICO_SIZES])


if __name__ == "__main__":
    main()
