# Upgrade API

This file will hold notes of pending changes in APIs going forward.

## ModuleD will be official registry for modules

In the future, the existence of a module's InitD will no longer
be the official indication of the module's being online.

### 0.61

Usage of the module's initd's existence as an indication
of the module's status will be formally deprecated.

Going forward, ModuleD should be explicitly queried
regarding a module's state.

ModuleD's interface for booting and shutting down modules
should be used exclusively.

A module can be nuked even if it's shut down to enforce a
complete purge of its owned objects.

### 0.62

ModuleD's record on the status of a particular module
will no longer automatically reflect the status of the
module's initd.

Modules will only be sent signals by ModuleD if they are
officially online according to ModuleD's records.

A module registered as online, but whose initd is not
compiled, will be an anomalous state that will generate a
runtime error.  Such a detected status will cause the
defunct module's missing initd to be deprived of signals.

## Deprecation of constructors and destructors

Due to being more complicated than they are worth and due to the data
burden, constructors and destructors are once again deprecated and are foreseen to remain so permanently.

### 0.61

Formal deprecation and no support will be offered for continued usage.

Warnings will be issued if an initd attempts to register a constructor or
a destructor, however ObjectD will continue to query for them when
programs are compiled.

### 0.62

ObjectD will cease support for registering constructors and destructors.

ObjectD will query initd's for them, but will return errors if any are returned.

### 0.63

All programs should be recompiled by the end of the 0.62 upgrade, so no
constructors or destructors should be registered.

Before the 0.63 upgrade is allowed to commence, all program_info objects
will be checked to make sure that constructors and destructors have been
removed.

### 0.64

Program info lwo's will be stripped of all information regarding constructors and destructors

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
