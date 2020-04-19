# TODO

Master TODO list for kotaka

## Deprecate implicit module registration

In the future, the existence of a module's InitD will no longer be the
official indication of the module's being online.

Henceforth a module's official status is determined by ModuleD's records.

However, in the event of ModuleD's destruction and recompilation, it will
audit the existence of existing initds to recreate its data.

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
