# Account

This is a very critical subsystem, and it provides all account management functions in Kotaka.

Basic security piggybacks on top of the klib, but it is the Account
module that actually provides authentication and authorization services
for the rest of the mudlib.

## Critical daemons

BanD

	This keeps track of all account and site bans.

	Critically, System itself is set up to automatically deny
	connections from sitebanned IPs by acting as a UserD proxy to
	prevent unprivileged code from exposure to blacklisted IPs.

	Cascaded UserDs are not even allowed to handle connections that
	fail the BanD blacklist check.

AccountD

	This provides authentication services for users.  Users are not
	recognized as privileged unless authenticated, and only after
	that are they granted capabilities via the kernel library.
