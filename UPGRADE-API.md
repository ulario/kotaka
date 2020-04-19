# Upgrade API

This file will hold notes of pending changes in APIs going forward.

## Amendment of patching

To be completed in version 0.61

Patching will be simplified.

In version 0.61, any program, inheritable or otherwise, may have a
patcher associated with it by the corresponding initd during its
compilation.  ObjectD will query the initd for the patcher during
compilation and record it in the corresponding program's pinfo.

Upon recompilation of an object, if the object or any of its inherits,
direct or otherwise, has a registered patcher, the object and all its
clones will be tagged with call_touch and all such patchers will be
called before the object is handled.

Recompiling an object will abort any pending patches for it and its
clones, regardless of if there are any patchers for the new version of
the object.

Please note that patchers are executed in the context of the new version
of the object, so care should probably be taken to refrain from removing
any variables that will be required during patching until patching is completed.

## Deprecation of SortD

To be completed in version 0.61

I decided awhile ago that I should prefer libraries to daemons
when it comes to shared code.  It's more robust since the callers
have a reference to the inheritable instead of just a pointer. 
Also there can be a bit of a chicken and egg problem if you try
to call a daemon that hasn't been compiled yet.

Pursuant to this SortD is hereby deprecated.  All of its code has
been moved to /lib/sort and existing calls to SortD have been
converted to local calls to /lib/sort.  As it is SortD has
already for awhile been turned into simply a stub that inherits
/lib/sort.

(completed)
