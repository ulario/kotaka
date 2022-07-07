# PatchD

We need to eventually rework the patch queue.

There are components of PatchD that are important:

* The pending flag database.

  This keeps track of which objects have pending patches.

  When a master object is recompiled, it and all of its clones need to be
  tagged for patching, and the patchers need to be called for it before
  it is accessed.

  A set "patch pending" flag indicates that a recompiled object has not
  yet had its patchers called.

  Recompiling with patchers: set flag, arm touch trigger.  If already
  pending, overwrite.

  Recompiling without patchers: clear flag, don't arm touch trigger.  If
  already pending, turns into a no-op.

  Every time we check for patchability we always refer to the associated
  pinfo.

  This is important to make sure that an object's patchers are only
  called once per recompilation, to avoid repeated patcher invocation,
  just in case the object is touch-tagged for other reasons.

* The pending object queue.

  This keeps track of all armed objects that we have touched, to make
  sure that patching is actually triggered in a finite amount of time.

## Upgrading PatchD

After the new structure feature is implemented and we have dedicated
objects for deques, mappings, and arrays, we need to migrate PatchD's
existing data over to them.

No more improvised data structures.
