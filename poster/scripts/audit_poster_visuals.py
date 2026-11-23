#!/usr/bin/env python3
import fitz

doc = fitz.open('poster/poster.pdf')
page = doc[0]

print("=== POSTER VISUAL & STRUCTURAL AUDIT ===")
print(f"Total Page Dimensions: {page.rect.width} x {page.rect.height} pt ({page.rect.width/72:.2f} x {page.rect.height/72:.2f} in)")

blocks = page.get_text("blocks")
print(f"Total text blocks detected: {len(blocks)}")

# Measure bounding box of all content
min_x = min(b[0] for b in blocks)
min_y = min(b[1] for b in blocks)
max_x = max(b[2] for b in blocks)
max_y = max(b[3] for b in blocks)

print(f"Content Bounding Box:")
print(f"  X: {min_x:.1f} to {max_x:.1f} pt (Margins: Left={min_x:.1f} pt, Right={page.rect.width - max_x:.1f} pt)")
print(f"  Y: {min_y:.1f} to {max_y:.1f} pt (Margins: Top={min_y:.1f} pt, Bottom={page.rect.height - max_y:.1f} pt)")

# Inspect column-wise vertical extent
col1_blocks = [b for b in blocks if b[0] < 1000 and b[1] > 200]
col2_blocks = [b for b in blocks if 1000 <= b[0] < 2000 and b[1] > 200]
col3_blocks = [b for b in blocks if b[0] >= 2000 and b[1] > 200]

print(f"\nColumn 1 Vertical Span: {min(b[1] for b in col1_blocks):.1f} to {max(b[3] for b in col1_blocks):.1f} pt (Height: {max(b[3] for b in col1_blocks) - min(b[1] for b in col1_blocks):.1f} pt)")
print(f"Column 2 Vertical Span: {min(b[1] for b in col2_blocks):.1f} to {max(b[3] for b in col2_blocks):.1f} pt (Height: {max(b[3] for b in col2_blocks) - min(b[1] for b in col2_blocks):.1f} pt)")
print(f"Column 3 Vertical Span: {min(b[1] for b in col3_blocks):.1f} to {max(b[3] for b in col3_blocks):.1f} pt (Height: {max(b[3] for b in col3_blocks) - min(b[1] for b in col3_blocks):.1f} pt)")

# Check balance between columns
heights = [
    max(b[3] for b in col1_blocks) - min(b[1] for b in col1_blocks),
    max(b[3] for b in col2_blocks) - min(b[1] for b in col2_blocks),
    max(b[3] for b in col3_blocks) - min(b[1] for b in col3_blocks)
]
print(f"\nColumn Balance Variance: max-min = {max(heights) - min(heights):.1f} pt (Ideal < 150 pt)")
if max(heights) - min(heights) < 150:
    print("[PASS] Columns are exceptionally well balanced!")
else:
    print("[INFO] Column balance acceptable, but check visual layout.")
