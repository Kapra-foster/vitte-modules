# C:\Users\vince\Documents\GitHub\vitte-modules\ray-runtime\bench\mod.muf
# ============================================================================
# ray-runtime — bench (Muffin manifest)
# - Agrège les benches du runtime (executor, mpsc, io_copy, tcp_throughput)
# - Sortie: un binaire "ray-bench" (ou plusieurs bins si tu préfères)
# ============================================================================

name = "ray-runtime-bench"
kind = "package"
version = "0.1.0"

edition = "2025"
license = "MIT"
authors = ["vince"]

# ----------------------------------------------------------------------------
# Entrées
# ----------------------------------------------------------------------------

[paths]
src = "."

# ----------------------------------------------------------------------------
# Dépendances (modules internes runtime)
# ----------------------------------------------------------------------------

[deps]
"runtime.core"     = { path = "../src/core" }
"runtime.abi"      = { path = "../src/abi" }
"runtime.platform" = { path = "../src/platform" }
"runtime.mem"      = { path = "../src/mem" }
"runtime.sync"     = { path = "../src/sync" }
"runtime.task"     = { path = "../src/task" }
"runtime.async"    = { path = "../src/async" }
"runtime.executor" = { path = "../src/executor" }
"runtime.reactor"  = { path = "../src/reactor" }
"runtime.io"       = { path = "../src/io" }
"runtime.fs"       = { path = "../src/fs" }
"runtime.net"      = { path = "../src/net" }
"runtime.time"     = { path = "../src/time" }
"runtime.process"  = { path = "../src/process" }
"runtime.signal"   = { path = "../src/signal" }
"runtime.ffi"      = { path = "../src/ffi" }
"runtime.plugin"   = { path = "../src/plugin" }

# ----------------------------------------------------------------------------
# Binaries
# ----------------------------------------------------------------------------
# Option A (simple): un binaire par bench (plus facile à exécuter en CI)
# Tu peux aussi faire Option B: un seul binaire "ray-bench" qui dispatch.
# ----------------------------------------------------------------------------

[[bin]]
name = "ray-bench-executor"
main = "b_executor.vitte"

[[bin]]
name = "ray-bench-mpsc"
main = "b_mpsc.vitte"

[[bin]]
name = "ray-bench-io-copy"
main = "b_io_copy.vitte"

[[bin]]
name = "ray-bench-tcp"
main = "b_tcp_throughput.vitte"

# ----------------------------------------------------------------------------
# Profiles (indicatif)
# ----------------------------------------------------------------------------

[profile.dev]
opt = 1
debug = true

[profile.release]
opt = 3
lto = true
debug = false

# ----------------------------------------------------------------------------
# Bench runner (optionnel si ton tool le supporte)
# ----------------------------------------------------------------------------

[bench]
default = "ray-bench-executor"
