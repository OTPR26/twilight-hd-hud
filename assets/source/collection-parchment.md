# Collection parchment

Original texture created for the Collection screen. The source PNG retains
transparent worn edges. The packaged BTI contains pixel data only; labels,
separator and ornament are separate native UI elements.

Build with `python3 tools/encode_collection_parchment.py` (Pillow required).

## Creation prompt

Use case: stylized-concept. Asset type: transparent game-menu parchment panel
texture, not a mockup. Create a single flat rectangular sheet of pale cream
parchment viewed perfectly straight-on, wide 2.4:1 shape on genuinely transparent
background, filling almost the entire canvas with a narrow transparent margin.
Very restrained light fibers and cloudy warm ivory texture, slightly darker
faded beige edges with subtle irregular worn fibers, no curled corners. The
central 90 percent must be nearly uniform pale ivory to support small dark
text in a game UI. Keep all edges close to a rectangle, softly feathered with
tiny rough imperfections, no large tears. No text, no symbols, no border, no
gold decorations, no objects, no hands, no scenery, no perspective, no dark
shadow, no watermark. Output a wide landscape image with actual alpha
transparency outside the paper.
