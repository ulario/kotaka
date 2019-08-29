# TODO

This is the master, active TODO list for kotaka development.

Anything here is on the front burner to get done in the foreseeable
future.

Tasks should be listed in order of desired completion.

## Secretlog

This is a new library meant to consolidate the logic for writing logs to
~Secret to protect them from snooping.

As with all API changes, both the old and new APIs need to be supported
at the same time, so we need to migrate the old stuff before we drop it.

To preserve ordering, we must have the old stuff flushed completely
before we start using the new system.

Secretlog will be formally introduced in 0.60, with all existing use
cases set to flush their current methods before enabling secretlog to
preserve ordering.  Version 0.60

Affected users:

ChannelD, IntermudD, and ~Text/sys/logd

## Mudlib upgrading

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
