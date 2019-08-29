# Upgrading

To upgrade your mud, simply call INITD->upgrade_system()
Every upgrade has the following sequence of events:

## Advanced notice

Any API changes will be announced at least one version in advance.

For the purposes of this section, a change means anything that will cause
the old API to be unusable.

Unless otherwise noted, until the old API is obsoleted, it will remain
valid to use both APIs at the same time, though warnings may be issued if
the old API is used.

Please consult UPGRADE-API for currently planned API changes.

## Preparation

If anything needs done to prepare for an upgrade, those instructions will
be provided, and should be completed before the actual upgrade.

## Upgrade

Use the upgrade command to start the upgrade process.

This command is responsible for taking care of all of kotaka's native
code.

## post upgrade work

Instructions may be provided for things needing to be done after the
upgrade is completed.

## full rebuild

Effective as of version 0.53, it will be assumed that a full rebuild will
be done after an upgrade is made.  This implies destruction of all
inheritable objects, followed by recompilation of all other objects.  It
is advised to also ensure that objects whose source code has been
removed, or orphans, are also destructed.

A call to ~System/sys/subd->full_rebuild() will accomplish this
conveniently.

### Caveat

It will destruct all inheritables, as well as all orphaned objects
missing their source code, and then it will background compile all other
source files, including those which have not yet been compiled.

For the latter part, please ensure that there are no stray LPC source
files.
