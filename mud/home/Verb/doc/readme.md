# Verb

Until we get some formalization of the documentation process, and
establish our principles in the first place, this will have to do.

Verb, to be very blunt, is the module under /home that houses all
commands.

## Security

Right now, for the general part, kotaka is designed to be paranoid.

Kotaka is layered on top of the kernel library, which handles basic
security for us and we just slave ourselves on top of it.

A guest is class 0 and is anyone that isn't registered or logged in.

A player is class 1 and is anyone that has an account registered through Account.

A wizard is class 2 and is anyone that has been granted wizard status through the klib.

An admin is class 3 and is anyone that has been granted full access to /.

The mud owner is class 4 and is tied to the admin user.

Verb by itself has no special privileges, but is recognized as a gated
user by external code and is considered to be trusted to act on behalf of
users.  The caveat being that Verb programs are themselves responsible
for enforcing security and are expected to do their own authorization
checks, and the only thing Verb is entitled to trust is the
authentication and privilege granting infrastructure, part of which is
handled by Account.
