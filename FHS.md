# Kotaka FHS

This stipulates the layout of the repository.

All entries are to be kept in alphabetical order.

Note: Kotaka is built upon the kernel library, which enforces directory
constraints on compilable LPC source files.

* lib - Any inheritable
* obj - Any clonable
* lwo - Any light-weight object

## Main

This is the top level layout for the entire repository.

=bin=
    Symlink to the DGD binary

=doc=
    Miscellaneous design notes that haven't been categorized yet

=docs=
    Documentation for github

=mud=
    Root directory for LPC environment

=script=
    Shell scripts

=state=
    Snapshots, swapfile, and editor tempfile.

## Mud

This is the layout as seen by the LPC layer, and the main filesystem
environment seen by DGD.

=doc=
    LPC visible documentation

=etc=
    Configuration information.

=home=
    Privileged areas.

    Note: The kernel library enforces creator based security and declares
    all files under a particular /home/<user> directory as owned by that
    user.  Please consult the klib documentation for more information.

=include=

=include/kernel=
    Include files belonging to the kernel library.

    Read only.

=kernel=
    Files belonging to the kernel library.

    Read only.

=lib=
    Public

=var=
    Variable data, like world saves

    Note: Audit this to see if it needs kept secret.
