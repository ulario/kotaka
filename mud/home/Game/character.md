# Character

Objects in the game world can be promoted to characters by initializing
their character module.

An object with an active character lwo is considered a character.

Please note that the living module must also be activated for the object
to be considered alive.

## Module

Game objects have the character library, which has a single variable of
object type, that contains an lwo, inside which is all information
pertinent to a character.

The character library manages the initialization and erasure of the
character lwo, whereas the lwo itself manages aspects of being a
character.

## Variables

The character lwo contains:

* attack
* defense
* max mp
