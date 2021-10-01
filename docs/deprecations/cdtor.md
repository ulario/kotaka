# Constructors and destructors

Unfortunately it was found that constructors and destructors were
unreliable for ensuring proper initialization.

Recompiling an object and introducing a new inheritable into the
inheritance tree will cause the inheritable's constructor to be bypassed,
likewise removing an inheritable during recompilation will cause the
destructor to be bypassed.

Therefore, constructors and destructors are deprecated.  The only
reliable way to ensure an upgrade, apart from defensive coding and usage
of flag variables, is to use the patcher in combination with call_touch.

* Stage 1: Formal deprecation

  Release 0.62 will formally deprecate constructors and destructors.

  They will still be called if provided.

* Stage 2: Warning

  Release 0.63 will cause warning diagnostics to be issued if an initd
  attempts to set a constructor or a destructor during compilation.

  They will still be called if provided.

* Stage 3: Error

  Release 0.64 will cause errors to be raised during compilation if the
  initd attempts to set a constructor or destructor.

  They will still be called if provided.

  There will also be an upgrade check to forbid upgrading to 0.65 until
  all program_info records are purged of constructor or destructor
  information.

* Stage 4: Purge

  Once purge of ctor/dtor information is verified by 0.64, 0.65 will
  remove the constructor and destructor variables from program_info.
