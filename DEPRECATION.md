# Deprecation

Deprecation is a formal process by which a feature is removed that has
proven to be ill considered or problematic after it has already been released.

## Stages

Deprecation takes place in stages.

* Formalization

  This is where formal notice is given in the release of the pending deprecation.

  The code reserves the right to commence migration of any data affected.

  No further proceedings are permitted on a feature deprecation until it
  has been formalized for at least one release.

* Warning

  Any attempts to use the deprecated feature will cause warning messages
  to be posted to the system log.

  A deprecated feature must spend at least one release in warning stage.

  Warning stage is also the proper time to initiate data migration if
  necessary.

* Error

  After a feature has spent at least one release in warning stage,
  further usage of the feature may be forbidden with errors.

  Any required data migration in connection with the feature being
  deprecated must be completed in the error stage.  Any further upgrades
  must be blocked until migration can be verified as completed.

* Purge

  After the error stage is passed, the feature may be completely removed.
