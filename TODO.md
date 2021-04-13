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

### CatalogD: deprecate

* Make sure no code uses CatalogD
* Make sure IDD completely absorbs all state from CatalogD
* Post system-upgrade, force migrate all remaining data and signal when completed
* Must be COMPLETELY ready to remove by the following version.

## Backburner

The following need to be done in a future version, but aren't being handled in 0.62.

* Deprecation of constructors and destructors
* Removal of Bigstruct
* Removal of Utility
* Consolidate Kotaka
  * Channel
  * Help
