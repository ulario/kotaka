# Multiple archetypes

Multiple archetypes are hereby deprecated.

Current deprecation stage is stage 1, formal deprecation.

* Stage 1: Formal deprecation (pending for 0.62)

  This is to be completed in release 0.62.

* Stage 2: Warning

  In release 0.63, we will start issuing warnings on any attempt to set
  multiple archetypes.  If a call is made to set_archetypes() in the
  future, a warning diagnostic will be issued and the extra archetypes
  will be ignored.

  We will also install a patcher for /home/Thing/lib/thing/archetype to
  automatically migrate data in place.

* Stage 3: Error

  In release 0.64 any attempt to call set_archetypes() will raise a
  runtime error.

  An upgrade check will also be performed, and any attempt to upgrade to
  release 0.65 will be blocked if there are outstanding patches for the
  archetype library.

* Stage 4: Purge

  In releas 0.65 all archetypes will have been converted, and the
  pre-upgrade check in 0.64 will have prevented this upgrade.

  This stage will remove all associated functions and variables, causing
  all related state to be purged.
