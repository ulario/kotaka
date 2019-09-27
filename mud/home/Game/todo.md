# Game TODO

Right now we have too much focus on system infrastructure.  We need
something FUN.  Something that will let players actually have a reason to actually play.

## Economy

We need to have some sort of in game economy.  Resources?

## Social

Ways for players to interact

## Combat

We need the ability to fight.

For now, we have a very simple combat system.

Objects can be characters by activation of their character module, which spawns the appropriate lwo to track their stats.

Furthermore, they can, if characters, activate as living beings by subsequent activation of their alive module, which spawns a separate lwo to keep track of any data required for them to be alive.

### Character

A character has the following attributes:

* Attack
* Defense
* Max HP

A character without an alive module is considered dead.

### Living

A living character has the following additional attributes:

* Current HP

### Life

A character can become alive by initialization of their alive module.

A character is considered alive so long as their HP remains positive.

### Damage

Any time a fighter is attacked, they take damage equal to the attack power of the attacker minus their defense power.

This damage is deducted from their current HP.

### Death

If a fighter's HP reaches zero, they die.

Death by loss of HP or any other means will result in the shutdown of their alive module.

Death, with the shutdown of their alive module, results in the forfeiture of any initiative
