This directory is to contain the source for a basic motion tracker.
The goal should be to make it not specific to Aegisub, but generally
re-usable in other projects as well.

It should probably be as modular as reasonably possible:
- Publish functions for creating some basic data structures, like:
  o Trackpoints
  o Image scale spaces (?)
  o Other?
- Functions for basic manipulations:
  o Generate track points from an image, with a choice of algorithm
  o Track one point from one image to the next, also algorithm choice
  o More?
- Packaged all-in-one operations
  o Something (callback based?) that generates a series of track points
    tracked along a series of images

I guess it might become a bit like FFTW is for fourier transforms, if
this succeeds.

Code should, of course, be well commented.

Should not depend on wxWidgets (cf. "not specific to Aegisub") and
should overall be as independent as possible.
