# Structures

We need to be able to have structures for things like big mappings, big
arrays, and so on.

I just realized we can't slave them just to LWOs, because those can't be
reliably upgraded.  Clones on the other hand can be searched for and
manually patched as needed.

However, if we use clones we also need to have a way to garbage collect
them later if they are accidentally leaked.

* array
* deque
* mapping

## Code reuse and privilege separation

We need to make sure the System module is self sufficient, so it needs to
be able to have its own clones for its internal data structures.

However, we also need the rest of the mud to have unprivileged versions,
which will need to have a nil creator.

We need to move the functionality to a System library, and have the
clonables inherit this library.
