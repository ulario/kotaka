# Miscellaneous deprecations

* AnsiD

  AnsiD was a standalone daemon, and this now violates currently
  established practices for Kotaka.

  The functionality has been moved to a library and an inheriting stub
  has been left in its place.

  We will only need to keep this around for transition.  The rebuild for
  0.62 will remove all references to AnsiD, after which it can be simply
  removed.
