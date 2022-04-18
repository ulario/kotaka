# TODO

Master TODO list for kotaka.

## Feature freeze

We need to put in a feature freeze and postpone any features that aren't
production ready yet.

Reason: we're due for a new release and an audit of all changes needs to
be completed for patching purposes.

## Structure refactoring

We need to consolidate our code for structures.

Owing to the need to maintain upgradeability in the future, we are
strictly moving to using clones and not LWOs to handle them.

See STRUCT.md for further information.

## 0.62

The following must be done before we release 0.62:

* module registration, stage 1
* ctor/dtor removal, stage 1, formal deprecation
* bigstruct removal, stage 1, formal deprecation
* Utility removal, stage 1, formal deprecation
* Kotaka consolidation, stage 1, formal notice
* CatalogD removal, stage 1, formal deprecation
* Multiple archetype removal, stage 1, formal deprecation

Please see files in [docs/deprecations] for further details on deprecated features.

### ModuleD: module registration

* ModuleD will register modules explicitly
* On creation, initialize module list based on initd existence
  * This will find nothing if we're doing our first boot

Module status:

0: module is offline
1: module is online
-1: module is shutting down

### Remove CatalogD, Stage 1

CatalogD is hereby formally deprecated

### Ctor/Dtor removal, Stage 1

This stage of ctor/dtor removal is a formal deprecation of constructors and destructors.

### Kotaka consolidation, Stage 1

HelpD and ChannelD will move to the Kotaka module in the following verison

### Bigstruct deprecation, Stage 1 (on hold)

Bigstruct deprecation is put on hold until the consolidated/refactored
struct project is completed.  We need to give Bigstruct's users something
to migrate to before we gut it.

## Backburner

The following need to be done in a future version, but aren't being handled in 0.62.

* Removal of Utility
* Consolidate Kotaka
  * Stage 1: Notice (in progress)
  * Stage 2: Warning stage, redirect, start migration
  * Stage 3: Error stage, check migration
  * Stage 4: Purge stage
