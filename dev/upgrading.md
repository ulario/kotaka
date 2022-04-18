# General upgrading plans

Ok, so the whole "force everything to recompile" is intrusive into a
module.

Instead we should give each module a chance to veto an upgrade if needed,
and then we need to instruct the module to actually upgrade itself as
instructed.

As the System module we shouldn't be force upgrading a module ourselves.

# Required stages

* check

  The first required stage is to query each module, including System
  internally, to make sure that it's ready to upgrade.

  Examples of things that should stop this are planned migrations
  initiated in a previous version that we need to verify as completed.
  If we're not done cleaning up after a previous upgrade we need to veto
  anything going forward that would be a problem.
