# Upgrading as a developer

Upgrading an end user's MUD is a required at times operation.

Minimizing disruption of the end user's code is paramount.

General principles is, that each module's InitD is responsible first
checking and then enforcing the upgrades for all of the code in its
module.

## Principles

1.  System is special

It is the heart of the system and holds rule over the other modules in
the library, therefore it must be treated specially.

2.  Bridge the gap

We should strive to resist the temptation to force someone depending on
us to dump stuff out.  We should migrate when possible.

## Current upgrade procedure:

### Have INITD recompile itself.

INITD is the heart of the System module and also has a fixed API
that it must adhere to with the kernel library.

INITD's first move will be to fire a callout and then recompile itself.

The callout will run in the newly recompiled INITD and will
handle the process.

To ensure a clean recompile we destruct all System inheritables first.

### Have the upgraded INITD upgrade System.

The upgraded INITD will then do what it needs to upgrade the
System module and prepare for upgrades of the other modules.

Once INITD has finished upgrading the System module, ModuleD will
send an upgrade signal to the other modules.

### Modules upgrade themselves.

Each module is responsible for upgrading itself.  This should probably be done:

* Each module should recompile itself.
* Each module should then upgrade itself.
* If the module cannot upgrade itself, it should emit a diagnostic.

Upgrade signals are delivered in an undefined order and any
module must at any time be prepared to have another module in a
state before or after upgrading.  It should also be prepared to
be accessed both before and after it is upgraded.

Any modules that change the api exposed to other modules must be
prepared to accept access under the previous API.  The System
module will also be required to comply with this.

### Global rebuild.

All modules are given a chance to purge their libraries and unneeded objects.

After this, all modules are given a chance to compile or recompile.

## Module upgrades

The module upgrading process is in two parts.

All hooks apply to the module's InitD.

### ObjectD hooks.

void upgrading()

	This is called when the initd has been successfully
	recompiled.  It is called in the task that did the
	compile, and therefore will be executed using the old
	code.

void upgrade()

	This is called as a prefix to the next task to execute
	and is guaranteed to execute before anything else.  It
	will be called using the initd's new code.

### ModuleD hooks.

void upgrade_module()

	Command from ModuleD instructing it to upgrade itself.
	ModuleD will recompile the initd first.

void upgrade_purge()

	Sent as part of the global upgrade process, this
	instructs the module to clean up in preparation for the
	build phase.  The module is expected to destruct any
	inheritable or otherwise decommissioned objects,
	particularly if such inheritables are inherited by other
	modules who may well need to rebuild their own objects.

void upgrade_build()

	Sent as part of the global upgrade process, this
	instructs the module to recompile all of its objects,
	which may well include those that inherit from other
	modules.
