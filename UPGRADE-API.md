# Upgrade API

This file will hold notes of pending changes in APIs going forward.

## ModuleD will be official registry for modules

In the future, the existence of a module's InitD will no longer
be the official indication of the module's being online.

### Step 1

Usage of the module's initd's existence as an indication
of the module's status will be formally deprecated.

Going forward, ModuleD should be explicitly queried
regarding a module's state.

ModuleD's interface for booting and shutting down modules
should be used exclusively.

A module can be nuked even if it's shut down to enforce a
complete purge of its owned objects.

### Step 2

ModuleD's record on the status of a particular module
will no longer automatically reflect the status of the
module's initd.

Modules will only be sent signals by ModuleD if they are
officially online according to ModuleD's records.

A module registered as online, but whose initd is not
compiled, will be an anomalous state that will generate a
runtime error.  Such a detected status will cause the
defunct module's missing initd to be deprived of signals.

## Amendment of constructors and destructors

Constructors and destructors are still useful, they are therefore
being reinstated.

However, due to the ability of a program to arbitrarily add and
remove inheritables on recompilation, a caveat is announced.

Inheritables are warned that their constructor might not be
properly called if an object inherits them after a clone has been
created.  It is advisable to have a patcher registered to check
for this.

Inheritables are also warned that they may be removed at any time
from an object before the object is destructed, and thus, that
the destructor might not be called.

If an inheritable needs to be prepared for use after construction
due to post-clone recompilation, have a patcher registered to
intercept the object that was recompiled.

## Amendment of patching

For the same reason as for constructors and destructors,
inheritors being able to rewrite the inheritance tree of their
objects at any time they see fit also makes guarantees for
patching impossible.

Therefore, going forward, we will simplify the regime, and the
responsibilities of affected programs will be amended.

A patcher can be registered with a program when the program is compiled.
The patcher will be queried from the InitD responsible for the program in
question at the time the program is compiled.

When a non inheritable program is compiled, it is checked for patchers in
either itself or any other program it inherits, and they are called as
soon as the program is compiled.  All the object's clones will also be
touched to ensure they are patched before being accessed again.

It's possible to use a patcher to intercept objects that
newly inherit an inheritable after being originally created.

## Deprecation of SortD

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
