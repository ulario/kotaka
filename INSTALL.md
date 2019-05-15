# INSTALL

To use kotaka:

## Get DGD

You can find dgd at git://github.com/dworkin/dgd

## Get the kernel library

You can find the kernel library in one of two locations:

git://github.com/dworkin/kernellib
git://github.com/shentino/kernellib

The shentino branch is preferred, but the dworkin branch should
still work.

Use the shentino branch if you want to keep track of callouts as
a resource, or have runtime auditing of filequota

## Put the mudlib where you want it.

The top level directory contains everything you need to run
kotaka, except for the kernel library and dgd.

## Hook in the kernel library

The kernel library has a /kernel directory and an /include/kernel
directory, both of which are required to use kotaka.

By default, symlinks to the kernel library and its include files
are already in place that assume both kotaka and the kernel library
are neighbors.

## Configure the kotaka.dgd file appropriately

Make sure the directory setting points to the absolute path of
where you installed kotaka.

## Set up the monitor script (optional)

This script can be used to keep dgd going and will automatically
restart it on termination if desired.

If you'd rather start dgd manually, that is ok.

## Configure and enable Intermud (optional)

You will need to examine ~Intermud/intermudd.c for correctness
and to define MUDNAME.

Once the file has been edited properly, check ~Game/initd.c and
amend the load() function to boot the Intermud module.

## Connect

By default, kotaka.dgd tells dgd to listen to the following ports:

* 50000 Telnet login
* 50001 Live status updates
* 50010 Kernel library binary port emergency login

## Administrative setup

Security and privileges for Kotaka are based on the underlying
kernel library.

Connect to port 50000, and select "register a new account"

Then, pick the "admin" username.

## You are now ready.

If you need more information, just read the in game help files.
