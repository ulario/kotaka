# Multiple connections

An important feature we want to add is the ability for the same user to
connect multiple times.

Reason: Allow one player, especially game staff, to be logged in on more
than one character at a time if necessary.

The "user" concept will need to now stand for one physical user, and we
will have a new "session" concept to be tied directly to the connection
in question.

The session will maintain the ustate stack and thus handle in character
interactions with the character currently being played, whereas the user
will now be reserved to stand for the user when they are logged in at least once.

## Tasks for the user object

* Subscription to channels?
* Anything regarding authentication or authorization, such as with the Account module.

## Tasks for the session object

* Ustate stack
* Liaison with the connection object
* Promotion of a guest connection to a user connection
* Will be associated with the IP address of the connection

## Migration

When this feature is implemented, the Text module should probably be
responsible for migrating all logged in users from the old user object to
the new session object.

We should be able to do so iteratively and atomically.

We need to be ready to direct new connections straight to the session
object.

We need to be able to upgrade/migrate existing connections from the
legacy user object to the new session object.

If we attempt to log in with a session object for a user that is already
connected on a user object, we need to migrate the old legacy user to a
new session object.

The Text module will be responsible for handling the overall migration
process.

We must start the migration process when this feature goes gold, and we
must prevent further upgrades until the migration process is finished.

We cannot get rid of the legacy user object until after the migration is
complete.

The user object as it stands must first be able to distinguish whether it
is being used as a legacy user object or a new user object.
