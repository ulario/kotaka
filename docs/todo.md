# Todo

This is the master, active TODO list for kotaka development.

Anything here is on the front burner to get done in the foreseeable
future.

Tasks should be listed in order of desired completion.

## Ongoing

The following TODO items are in progress.

### Mudlib upgrading

From now on, to avoid accidents of versions being skipped by premature
recompiles or upgrades, all necessary checks will be done in a
verification phase before the upgrade commences.

First, the host admin downloads the new version.

Second, the current version checks to see if it is ready to upgrade.
Usually this means all pending data upgrades from the previous version
are completed.

Third, the presently compiled will inspect the new version's upgrade
information to see if the current version is an acceptable upgrade
source.

After these checks are completed the upgrade will commence.

This should be documented in upgrading.md.

## Long term

These are on the list but are not yet in progress.

### Consolidate Kotaka

We're at the point of ravioli.  There are certain things that are going
to comprise the unprivileged core of Kotaka, so they should be
consolidated into the Kotaka module.

Channel and Help in particular.
