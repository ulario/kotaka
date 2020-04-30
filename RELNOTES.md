# Release notes

This is release 0.61 of Kotaka.

Please pay attention to the following:

## Important

Old pre secretlog buffers have been removed

	The upgrade has finished
	0.60 would not allow an upgrade until the flushing was done

	This has the potential to cause data loss if the upgrade process
	put forth in 0.60 was not properly followed

	This affects LogD, ChannelD, and Text's logd

TODO.md

	In the coming releases there will be important instructions to
	follow.  Presently they are in TODO.md.

## Noteworthy

ChannelD config can now be saved and loaded
Allow global aliases to be saved and restored
Allow kernel verb to save, restore, and reset ACCESSD
Rebuild has been moved to the wiztool, and fullrebuild has been removed
Loadworld is now called restoreworld

## Other

String lists now use 512 byte strings instead of 4096
Remove old testing code for pre secretlog queues
Qsort test moved from Utility to Test
No longer need to check to see if PatchD, ChannelD, LogD, or Text LogD are busy.

	PatchD for updates, and the others for pre secretlog flushes

Remove 0.60 upgrade code from Text LogD (check for flush)
Ditto for ChannelD
Turns out IntermudD sends nil messages for emotes (todo: rework emote handling)
Moved Ansi code to Text module
Allow INITD's system upgrade to be initiated by a verb, interface, or a klib admin
Infinite ticks are ok if we sandbox the upgrade hook we call
Simplify secretlog flushing, also allow manual flushing
Remove callout removal from secretlog, the call_out_unique function takes care of not repeating
With ansi rendering, reset the terminal colors to standard with each new row
Small formatting fix in IntermudD
Explicitly set color for first row (todo: don't assume that default color is always going to be light gray on black)
DGD list shut down
Have accounts deleted by AccountD, not the account object
Don't allow existing accounts to be renamed
Back up old account file when saving or deleting
If we get a nil message in ChannelD stub it out (workaround, revisit when implementing emotes)
More verbose diagnostic if a module's initd is missing during attempting to boot it
Don't have Text boot Ansi anymore
Ditto from Kotaka
Call tests directly instead of having them background
Avoid leaving nils if we have no data to load in BanD
Don't save empty lists in BanD
Highlight headers when printing ban lists
Gather Game thing code into top object
Deprecate having an orphaned saved password in IntermudD, we should always associate the password with the mudname it belongs to.
SortD is deprecated, from now on code if possible should be inherited as a library.
Constructors and destructors are formally deprecated
Document future PatchD procedures.  Gist: if the object specifies or inherits any patchers, flag them for calling after recompilation before the object is accessed.
DumpD rework, gut for later readding of proper staggering of incremental and full snapshots with offsets
Have DumpD complain if it's stalled too long
Channel timestamps should include seconds
Add colors to channel messages
Have ModuleD audit for loaded modules if it's created, in case it was destructed during runtime
Automatic clone hunts are a performance killer, don't do it unless asked
Lock down DumpD so that not just anyone can call it
Have INITD destruct all system objects whose source is removed
Hotboot moved to system wiztool
Dormants moved to system wiztool
We do full snapshots every day by default
Orphans moved to system wiztool
Move rebuild function to system wiztool
Check on header for ban list (todo: revisit during ansi code audit)
Reformat output of pinfo command
Let ChannelD boss IntermudD around on what to listen to on I3
Simplify compile utility, we don't need to be clever
As part of wiztool's rebuild, also destruct all listed libraries just to be safe in case they aren't registered
Have ChannelD responsible for manipulating IntermudD, also fix incorrect api
Reformat statedump wiztool command, the one that in our version allows incrementals
Module registration has been documented in TODO.md
Also document the reconstitution process if ModuleD happens to be destructed
Use dgd-small (todo: revert)
Formally document the constructor/destructor deprecation in TODO.md
Bigstruct and Utility will be deprecated
Simplify module rebuild process, and don't automatically shut down a module here just because its initd is missing.  If it's registered but missing that *should* cause an error
System is not an explicit module (todo: treat it as one but make it special, we just don't register ourselves)
Utility module is empty, delete it (todo: check to make sure that a missing initd during system upgrade causes the module to shut down)
Allow modules to be listed
Don't allow System to automatically purge objects belonging to another module
Allow Bigstruct and String modules to be shut down
Don't destruct a program twice
When shutting down a module, destruct its master objects first on one swoop, inheritables included
It's ok for the kernel to call the standard craetor function after ours does its work, we don't need to take over
An lwo master can still be destructed, provide the proper call
Document ticks budget for Test module a little better
Have mapping struct lwo allow indices to be queried
Move binary search library to System
Simplify callout wiping in SwapD, and reduce the spam and simplify it in general
Utility module is gone, so remove the paths include for it
Don't boot the Utility module from Kotaka
Remove utility paths include from including files
Revert dgd-small, the public release should be standard
Document the need for consolidation
Upgrade process is already documented in UPGRADE-DEV.md, remove the redundant notes from todo.md
Small tidy of BanD
Privatize purge_objects in ModuleD
ModuleD reorganized
INITD prefixing and despamming
0.59 callout already intercepted by now, remove the note
Whoops, we forgot to log the sender in ChannelD
Whoops, we forgot to have SwapD actually swap out (todo: audit SwapD)
Don't dump if the interval is zero
Simplify SwapD
Incremental snapshots are important for logging after all, says I, in my opinion
Lock down ModuleD some more
ACCESSD should only be restored manually or on a cold boot, if someone monkeyed with it offline we shouldn't load it automatically
AliasD and ChannelD should only be automatically restored on cold boot, don't monkey with it if we're warm booting
Allow verbs to manipulate ACCESSD via KernelD
Don't automatically nuke a module if its initd is missing just because we're sending signals.  Missing initd's outside of an upgrade is probably an error
Document in ObjectD
Remove full_rebuild from ObjectD
Forgot to have modules regrant their own global access on boot after moduled stopped doing it for them
Revert usage of dgd-small, this is supposed to be standard.  We forgot to adjust the hotboot script though.
Nil module is already globally readable
Scripts should be parameterized
INITD reorganized
We always know our current version, so remove the variables.  Future versions will list the versions they can be safely upgraded from anyway.
Streamline the system upgrade process and don't use so many callouts
Clarify todo list for future versions
Tweak log messages in tests
We run tests in callotus now, so each one only needs 100M ticks or so
Reorganize thing library (todo: recheck it and only lock down functions that would cause integrity issues if bypassed, but we can allow masking of functionality)
Provide maskable function calls (todo: recheck Thing's thing library)
Don't allow internal calls in Thing to be masked
Changed my mind about prefixing log messages in INITD
Abort function wasn't used
In DumpD we can just modulo the delay after adding the offset (todo, reaudit to make sure it's working)
Simplify DumpD by using the wipe_callouts safun
IntermudD doesn't want or need to save chanlistid or mudlistid, so don't bother recording them, or saving if they change
Migrate Game thing bits to a thing library, we want to have different kinds of things in the future, like hungry wolves.  At least until we have proper scripting.
Todo note about auditing ansi code usage
Swap out if fragmentation is over 50 percent
Only use actual color codes when we're rendering, plain text markup can just use standard bolding.
Default terminal color should be considered undefined
We need to implement emote handling.  Considering, IntermudD isn't the only thing that could use it, standard channels could use emotes too.
Clean up more automated module shutdowns.
Simplify thing code, especially when it comes to inventory management.
Make sure we don't get masked out of important functions in thing library.
