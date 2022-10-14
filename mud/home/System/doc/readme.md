# System

Ah yes, the klib's version of admin

System is privileged on a code basis, but is considered the mudlib's
"kernel" as it were, if we were to treat the kernel library itself as the
HAL for DGD.

Obviously lots of interesting stuff happens here.

TODO: Make a habit of keeping documentation up to date WITH the code.

Code may need to lead, but documentation must follow.

## Important daemons

ErrorD

ObjectD

	This is obviously important, and one thing we do that is critical
	is keep track of patchers that are registered during compilation.

PatchD

	In place upgrades are the bread and butter of DGD and we need to
	keep track of which objects haven't been patched on a data level
	after their code has been upgraded.
