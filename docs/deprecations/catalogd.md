# CatalogD

CatalogD was an early attempt to provide the same functionality as
SkotOS's IDD, but I picked a lousy name.

I also made the mistake of having it be a bit sloppy in how it was
organized, including the usage of anonymous directory objects.

The directory objects are fine for now, but they need recreated to go
with the new IDD.

* Stage 1: Formal deprecation

  Any usage of CatalogD is hereby deprecated as of release 0.62.

  We will cause CatalogD to combine its local data with that of IDD to
  return a full set of data.

  Migration of data in CatalogD will be deferred until the warning stage,
  however any data that has already been migrated will be left as is.

  This is a note to future deprecations and migrations to be more formal
  about the migration process and have it properly documented in the
  future.

* Stage 2: Warning

  In release 0.63 any usage of CatalogD will elicit a warning diagnostic.

  Release 0.63 will cause CatalogD to background migrate all of its data
  to IDD.  We will do so incrementally to avoid overloading the system,
  using a 0-callout to iteratively process each object.

* Stage 3: Error

  In release 0.64 any attempt to use CatalogD except for migration
  purposes will raise an error.

  We will also forbid upgrades to 0.65 until CatalogD's information has
  been completely migrated.

* Stage 4: Purge

  With all data confirmed as migrated by the 0.64 upgrade check we can
  safely destruct CatalogD.

  Stage 0.64's upgrade process will destruct CatalogD.  The LPC file will
  also be deleted from the repository.

  This should also result in all directory clones being deleted by then,
  so we will also destruct the master object concurrent with the removal
  of its source file from the repository.
