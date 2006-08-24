This is a drawing designer, or is intended to become one.
The name p-designer obviously comes from the tag, \p.

Intended roadmap:

1. Create datastructures to store drawing in
2. Write something to parse \p strings into those datastructures
3. Render some kind of basic outline of the figure in a way
   (is easy in Win32 and probably also wx)
4. Generate \p strings from the datastructures
5. Add draggable handles to the preview display
6. Add something to add further segments to a drawing
7. ???

Other stuff / wishlist:
* It should be possible to somehow split a segment into two, eg. click
  somewhere on a bezier and split it into two there. Same for lines etc.
* How about moves? ('m' command)
  - Maybe render as a dotted or dashed line between points
* Zoom ability
* Feature to convert between scales (number after \p)
* Stretch?
* Integrate into Aegisub video preview
* Maybe import outlines of one or more characters from a font to modify?
  (At least is possible with Win32 and Freetype2.)
* Convert segments between different types
* Binary operations with other shapes

Things to check:
* What happens when a drawing doesn't start with an 'm' command?
