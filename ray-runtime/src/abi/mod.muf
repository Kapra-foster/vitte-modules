# C:\Users\vince\Documents\GitHub\vitte-modules\ray-runtime\src\abi\mod.muf
# ============================================================================
# ray-runtime — src/abi (Muffin manifest) — COMPLETE
#
# But:
#   - Déclare le module ABI du runtime (struct-only + helpers purs)
#   - Construit une lib (ou module importable) + optionnel: tests abi
#   - Zéro I/O ici, uniquement types/records + fonctions pures
# ============================================================================

name = "ray-runtime-abi"
kind = "module"
version = "0.1.0"

edition = "2025"
license = "MIT"
authors = ["vince"]

[paths]
src = "."

# ----------------------------------------------------------------------------
# Dépendances internes (ajuste selon ton arbo)
# ----------------------------------------------------------------------------

[deps]
"runtime.core" = { path = "../core" }

# ----------------------------------------------------------------------------
# Build outputs
# ----------------------------------------------------------------------------

[build]
# lib = artefact importable par le runtime + plugins (ABI)
type = "lib"
name = "ray_runtime_abi"

# ----------------------------------------------------------------------------
# Sources (ordre explicite pour bootstrap/compilateurs simples)
# ----------------------------------------------------------------------------

[sources]
files = [
  "abi_errors.vitte",
  "abi_layout.vitte",
  "abi_result.vitte",
  "abi_slices.vitte",
  "abi_strings.vitte",
  "abi_types.vitte",
  "abi_versioning.vitte"
]

# ----------------------------------------------------------------------------
# Exports (surface publique)
# ----------------------------------------------------------------------------

[export]
modules = [
  "ray.runtime.abi.abi_errors",
  "ray.runtime.abi.abi_layout",
  "ray.runtime.abi.abi_result",
  "ray.runtime.abi.abi_slices",
  "ray.runtime.abi.abi_strings",
  "ray.runtime.abi.abi_types",
  "ray.runtime.abi.abi_versioning"
]

# ----------------------------------------------------------------------------
# Options
# ----------------------------------------------------------------------------

[options]
warnings = "all"
opt = 2
debug = true

# ----------------------------------------------------------------------------
# Tests (optionnel)
# ----------------------------------------------------------------------------
# Si tu as un runner, tu peux brancher des tests smoke/layout.
# Sinon laisse vide.

[tests]
files = [
  # "t_abi_layout_smoke.vitte",
  # "t_abi_versioning_smoke.vitte"
]

[profile.release]
opt = 3
lto = true
debug = false
