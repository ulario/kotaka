# TODO

Master TODO list for kotaka.

## Deprecate implicit module registration

In the future, the existence of a module's InitD will no longer be the
official indication of the module's being online.

Henceforth a module's official status is determined by ModuleD's records.

Existence of the initds will however be used to restore ModuleD's initd
database if ModuleD is destructed and later reloaded.

### 0.61

Usage of the module's initd's existence as an indication
of the module's status will be formally deprecated.

Going forward, ModuleD should be explicitly queried
regarding a module's state.

ModuleD's interface for booting and shutting down modules
should be used exclusively.

A module can be nuked even if it's shut down to enforce a
complete purge of its owned objects.

During the 0.61 upgrade, ModuleD will be instructed to reset its list to
conform to the initds currently existing.

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
burden, constructors and destructors are once again deprecated and are
foreseen to remain so permanently.

From now on, objects should use their creator functions to initialize any
libraries, and libraries themselves should be prepared to be called by
their inheritors on create and should prepare for objects that neglect to
do so.  It is advisable to declare a patcher in the library's initd to
ensure that any belated creation tasks are called on upgrade.

### 0.61

Formal deprecation and no support will be offered for continued usage.

Warnings will be issued if an initd attempts to register a constructor or
a destructor, however ObjectD will continue to query for them when
programs are compiled as a check on compliance.

### 0.62

ObjectD will cease support for registering constructors and destructors.

ObjectD will audit compliance by querying the initd and returning an
error if it attempts to register a constructor or destructor.

A check will be added to 0.62 requiring all program_info's to be pruned
of constructors and destructors before an upgrade will be allowed to 0.63.

### 0.63

0.62 will have checked the program database to ensure removal of
constructors and destructors before permitting the upgrade to 0.63.

Program info lwo's will be stripped of all information regarding
constructors and destructors.

## Removal of Bigstruct

Bigstruct has outlived its usefulness, and shall be removed.

### 0.61

Usage of Bigstruct in all forms is deprecated, and clients should switch
to the linked list and mapping facilities provided.

### 0.62

The Bigstruct module will be removed.

## Removal of Utility

Seems that Utility is no longer a needed module.

### 0.61

The module is simply going to be removed.

## Consolidate Kotaka

We're at the point of ravioli.  There are certain things that are going
to comprise the unprivileged core of Kotaka, so they should be
consolidated into the Kotaka module.

Channel and Help in particular.

## Miscellaneous

Audit the mud's usage of ansi color codes

Implement proper emote handling for users, ChannelD, and IntermudD
