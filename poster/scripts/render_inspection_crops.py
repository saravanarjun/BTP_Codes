#!/usr/bin/env python3
import fitz
import os

doc = fitz.open('poster/poster.pdf')
page = doc[0]

# Render full poster at 150 DPI
pix = page.get_pixmap(dpi=150)
output_full = 'poster/poster_preview_150dpi.png'
pix.save(output_full)
print(f"[+] Full poster rendered to {output_full} ({pix.width}x{pix.height} px)")

# Render zoomed crops for quality audit
# Page rect is (0, 0, 3024, 1512)
crops = {
    'header': fitz.Rect(0, 0, 3024, 250),
    'col1': fitz.Rect(0, 200, 1020, 1512),
    'col2': fitz.Rect(1000, 200, 2020, 1512),
    'col3': fitz.Rect(2000, 200, 3024, 1512),
}

for name, rect in crops.items():
    crop_pix = page.get_pixmap(dpi=150, clip=rect)
    crop_path = f'poster/crop_{name}.png'
    crop_pix.save(crop_path)
    print(f"[+] Saved crop: {crop_path} ({crop_pix.width}x{crop_pix.height} px)")
