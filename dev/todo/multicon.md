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
