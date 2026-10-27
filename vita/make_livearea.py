"""Builds a vita sce_sys folder (icon0, pic0, LiveArea bg0/startup) from an icon and a 960x544 title screenshot.

The vita wants 8-bit palettized PNGs for these, so everything is quantized to 256 colours.

usage: python make_livearea.py <icon image> <title screenshot> <out sce_sys dir> [crop x y]
"""
import os
import sys

from PIL import Image, ImageFilter

TEMPLATE = """<?xml version="1.0" encoding="utf-8"?>

<livearea style="a1" format-ver="01.00" content-rev="1">
  <livearea-background>
    <image>bg0.png</image>
  </livearea-background>

  <gate>
    <startup-image>startup.png</startup-image>
  </gate>
</livearea>
"""


def save_indexed(img, path):
    img.convert("RGB").quantize(colors=256, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE).save(path, optimize=True)


def cover(img, w, h):
    """scale to fill w x h, cropping the overflow (centered)"""
    scale = max(w / img.width, h / img.height)
    img = img.resize((round(img.width * scale), round(img.height * scale)), Image.LANCZOS)
    x = (img.width - w) // 2
    y = (img.height - h) // 2
    return img.crop((x, y, x + w, y + h))


def main():
    icon_path, shot_path, out_dir = sys.argv[1:4]
    crop = (int(sys.argv[4]), int(sys.argv[5])) if len(sys.argv) > 5 else (0, 0)

    icon = Image.open(icon_path).convert("RGB")
    shot = Image.open(shot_path).convert("RGB")
    shot = shot.crop((crop[0], crop[1], crop[0] + 960, crop[1] + 544))

    os.makedirs(os.path.join(out_dir, "livearea", "contents"), exist_ok=True)

    save_indexed(icon.resize((128, 128), Image.LANCZOS), os.path.join(out_dir, "icon0.png"))
    save_indexed(shot, os.path.join(out_dir, "pic0.png"))

    # LiveArea background: softened title screen so the gate/icon stay readable
    bg = cover(shot, 840, 500).filter(ImageFilter.GaussianBlur(3))
    save_indexed(bg, os.path.join(out_dir, "livearea", "contents", "bg0.png"))

    # gate image: the title screen itself
    save_indexed(cover(shot, 280, 158), os.path.join(out_dir, "livearea", "contents", "startup.png"))

    with open(os.path.join(out_dir, "livearea", "contents", "template.xml"), "w", encoding="utf-8") as f:
        f.write(TEMPLATE)
    print("wrote", out_dir)


if __name__ == "__main__":
    main()
