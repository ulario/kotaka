# General upgrading plans

Ok, so the whole "force everything to recompile" is intrusive into a
module.

Instead we should give each module a chance to veto an upgrade if needed,
and then we need to instruct the module to actually upgrade itself as
instructed.

As the System module we shouldn't be force upgrading a module ourselves.