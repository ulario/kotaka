# Upgrading

The upgrade process is documented in two parts.  The first part details
the responsibility of the admin running the system.  The second part
details the process followed by the mudlib.

## Admin

Before an upgrade can be applied the admin must do some preparation.

### Download

Use git to download in place the proper version of kotaka.  With a timely
upgrade schedule this will usually be the latest point release, however
upgrades should be done serially by version.  There are version checking
safeguards to enforce proper version sequencing but it's best to have the
correct version downloaded in advance.

Please note that if you have modified the source code you bear the
responsibility of your changes and any impact it may have on the upgrade
process as well as any conflicts with changes introduced by upstream.

### Upgrade

The upgrade process is intended to be invoked manually by appropriately privileged code.

The official hook for commencing the upgrade process is INITD->upgrade_system().

For the admin's convenience, an "upgrade" verb is provided.

## Mudlib

Once the admin has downloaded the new version and activated the upgrade
process, the mudlib will upgrade itself.

### Check current version for readiness

Before an upgrade is allowed to proceed the current version will be
checked for readiness.  Most often this will include completion of
post-upgrade work from the previous upgrade.  Data migration may be
included, as well as in general anything that relies on functionality
scheduled to be removed in a future version, which may include the
version being upgraded to.

### Check installed version for compatibility

Each version of kotaka has a whitelist of compatible versions from which
it can be upgraded to.  The upgrade process checks this whitelist before
proceeding.

### Recompile System

As an invariant that will apply in perpetuity, the first step to
upgrading the system is to recompile the System module

### System upgrades

Any upgrade operations applying to the System module itself will be
completed after it is recompiled.

### Module initd recompile

Each active module's initd will be recompiled.

Any active module whose initd is missing in the installed version will be shut down.

### Module upgrade

After a module's initd has been recompiled, its module upgrade hook will be called.

### Global purge

As a housekeeping measure, each module will be asked to destruct all of
its inheritables, as well as any orphaned objects whose source code has
been removed.

### GLobal rebuild

After the purge, each module is asked to recompile all of its objects,
which may include compiling new source files.
