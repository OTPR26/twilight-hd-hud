# Collection parchment

Original texture created for the Collection screen. The source PNG retains
transparent worn edges. The packaged BTI contains pixel data only; labels,
separator and ornament are separate native UI elements.

Build with `python3 tools/encode_collection_parchment.py` (Pillow required).

## Creation prompt

Use case: precise-object-edit. Asset type: transparent game-menu parchment panel
texture, not a mockup. Create a single flat rectangular sheet of aged warm beige
parchment viewed perfectly straight-on, wide 2.4:1 shape on genuinely transparent
background, filling almost the entire canvas with a narrow transparent margin.
Use muted tan and gray-brown mottling, faint fibers, uneven age stains, and
stronger darkened weathering along the perimeter, with no curled corners. Keep
the central text area quiet and readable with low-contrast organic wear. Keep
all edges close to a rectangle, softly feathered with tiny rough imperfections,
no large tears. No text, no symbols, no border, no flourishes, no ornaments, no
gold decorations, no objects, no hands, no scenery, no perspective, no dark
shadow, no watermark. Output a wide landscape image with actual alpha
transparency outside the paper.
