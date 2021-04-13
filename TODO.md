# TODO

Master TODO list for kotaka.

## 0.62

The following must be done before we release 0.62:

### ModuleD: module registration

* ModuleD will register modules explicitly
* On creation, Initialize module list based on initd existence
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

## Backburner

The following need to be done in a future version, but aren't being handled in 0.62.

* Removal of constructors and destructors
  * Stage 1: Formal deprecation (in progress)
  * Stage 2: Warning stage
  * Stage 3: Error stage, check sanitization
    * Any attempt to set a ctor/dtor will cause an error
    * Forbid upgrading from this version until all pinfos are clear of ctor/dtor information
  * Stage 4: Purge stage
    * The ctor/dtor fields will be removed from pinfo
* Removal of Bigstruct
* Removal of Utility
* Consolidate Kotaka
  * Stage 1: Formal notice (in progress)
  * Stage 2: Warning stage, redirect, start migration
  * Stage 3: Error stage, check migration
  * Stage 4: Purge stage
* Removal of CatalogD
  * Stage 1: Formal deprecation (in progress)
  * Stage 2: Warning stage, redirect to IDD, enable migration
  * Stage 3: Error stage, check for completion of migrat
  * Stage 4: Purge
