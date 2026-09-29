#!/usr/bin/env python3
"""Build the app's icon asset catalog from the product profile's pinned key art.

    python3 tools/ios/app_icons.py --platform ios|tvos --art ART [--banner ART]
        [--icon-fit cover|contain] --out Assets.xcassets

The catalog is plain files (Contents.json and PNGs), made here on the Linux
build host; Apple's actool compiles it to Assets.car on the Mac
(build-ios-app.sh). The key art is 16:9 artwork with a centred wordmark.

  ios   one 1024x1024 "AppIcon" (actool makes every size from it): the
        artwork's centre, as wide as the square, on a blurred, darkened
        stretch of the same art that fills the rest.
  tvos  the "App Icon & Top Shelf Image" brand assets: a two-layer home-screen
        icon (400x240 @1x/@2x: the art behind, a clear front layer),
        the App Store icon (1280x768) and the top-shelf images (1920x720 and
        2320x720, @1x/@2x), each the art cropped to its aspect.
        --banner gives the top-shelf images their own art. --icon-fit
        contain keeps the icon art's whole width (a wide wordmark) and fills
        above and below with a blurred, darkened cover of the same art.
"""

import argparse
import json
from pathlib import Path
import shutil
import sys

from PIL import Image, ImageEnhance, ImageFilter


INFO = {"author": "xcode", "version": 1}
BRAND = "App Icon & Top Shelf Image"


def write_json(path, value):
    path.mkdir(parents=True, exist_ok=True)
    (path / "Contents.json").write_text(json.dumps(value, indent=2) + "\n")


def cover(art, width, height):
    """The art scaled to cover width x height, cropped to its centre."""
    scale = max(width / art.width, height / art.height)
    scaled = art.resize((round(art.width * scale), round(art.height * scale)), Image.LANCZOS)
    left = (scaled.width - width) // 2
    top = (scaled.height - height) // 2
    return scaled.crop((left, top, left + width, top + height))


def contain(art, width, height):
    """The art scaled to width x height's width when it is wider than that
    aspect (else covering it), on a blurred and darkened cover of itself."""
    if art.width * height <= width * art.height:
        return cover(art, width, height)
    background = cover(art, width, height).filter(ImageFilter.GaussianBlur(width / 40))
    background = ImageEnhance.Brightness(background).enhance(0.6)
    band = art.resize((width, round(art.height * width / art.width)), Image.LANCZOS)
    background.paste(band, (0, (height - band.height) // 2))
    return background


def square_icon(art, size):
    """The art's centre, as wide as the square, faded into a blurred and
    darkened fill of the same art above and below."""
    side = min(art.width, round(art.height * 1.285))
    background = cover(art, side, side).filter(ImageFilter.GaussianBlur(side / 35))
    background = ImageEnhance.Brightness(background).enhance(0.8)
    left = (art.width - side) // 2
    band = art.crop((left, 0, left + side, art.height))
    fade = max(1, art.height // 7)
    mask = Image.new("L", band.size, 255)
    for y in range(band.height):
        alpha = min(255, int(255 * min(y, band.height - 1 - y) / fade))
        mask.paste(alpha, (0, y, band.width, y + 1))
    background.paste(band, (0, (side - art.height) // 2), mask)
    return background.resize((size, size), Image.LANCZOS)


def ios_catalog(art, out):
    icon = out / "AppIcon.appiconset"
    write_json(icon, {"images": [{"filename": "icon-1024.png", "idiom": "universal", "platform": "ios",
                                  "size": "1024x1024"}], "info": INFO})
    square_icon(art, 1024).save(icon / "icon-1024.png")


def imageset(path, name, width, height, render, scales, clear=False):
    images = []
    for scale in scales:
        filename = "%s@%dx.png" % (name, scale)
        size = (width * scale, height * scale)
        image = Image.new("RGBA", size, (0, 0, 0, 0)) if clear else render(*size)
        path.mkdir(parents=True, exist_ok=True)
        image.save(path / filename)
        images.append({"filename": filename, "idiom": "tv", "scale": "%dx" % scale})
    write_json(path, {"images": images, "info": INFO})


def imagestack(path, width, height, render, scales):
    layers = [("Front", True), ("Back", False)]
    write_json(path, {"info": INFO, "layers": [{"filename": "%s.imagestacklayer" % name} for name, _ in layers]})
    for name, clear in layers:
        layer = path / ("%s.imagestacklayer" % name)
        write_json(layer, {"info": INFO})
        imageset(layer / "Content.imageset", name.lower(), width, height, render, scales, clear)


def tvos_catalog(art, out, banner=None, icon_fit="cover"):
    brand = out / ("%s.brandassets" % BRAND)
    assets = [
        ("App Icon - App Store.imagestack", "primary-app-icon", "1280x768"),
        ("App Icon.imagestack", "primary-app-icon", "400x240"),
        ("Top Shelf Image Wide.imageset", "top-shelf-image-wide", "2320x720"),
        ("Top Shelf Image.imageset", "top-shelf-image", "1920x720"),
    ]
    write_json(brand, {"assets": [{"filename": f, "idiom": "tv", "role": r, "size": s} for f, r, s in assets],
                       "info": INFO})
    fit = contain if icon_fit == "contain" else cover
    icon = lambda width, height: fit(art, width, height)
    shelf = lambda width, height: cover(banner or art, width, height)
    imagestack(brand / "App Icon - App Store.imagestack", 1280, 768, icon, (1,))
    imagestack(brand / "App Icon.imagestack", 400, 240, icon, (1, 2))
    imageset(brand / "Top Shelf Image Wide.imageset", "top-shelf-wide", 2320, 720, shelf, (1, 2))
    imageset(brand / "Top Shelf Image.imageset", "top-shelf", 1920, 720, shelf, (1, 2))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--platform", choices=("ios", "tvos"), required=True)
    parser.add_argument("--art", type=Path, required=True, help="the pinned key art")
    parser.add_argument("--banner", type=Path, help="the pinned top-shelf art (tvos; default --art)")
    parser.add_argument("--icon-fit", choices=("cover", "contain"), default="cover",
                        help="tvos icon: crop the art to the icon (cover) or keep its whole width (contain)")
    parser.add_argument("--out", type=Path, required=True, help="the .xcassets directory to (re)create")
    args = parser.parse_args(argv)
    art = Image.open(args.art).convert("RGB")
    banner = Image.open(args.banner).convert("RGB") if args.banner else None
    if args.out.exists():
        shutil.rmtree(args.out)
    write_json(args.out, {"info": INFO})
    if args.platform == "ios":
        ios_catalog(art, args.out)
    else:
        tvos_catalog(art, args.out, banner, args.icon_fit)
    print(args.out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
