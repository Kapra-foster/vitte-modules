/* C:\Users\vince\Documents\GitHub\vitte-modules\ray-runtime\include\vitte_runtime.h
 * ============================================================================
 * vitte_runtime.h — Vitte Runtime Public C ABI (Tokio-like runtime)
 *
 * Objectifs:
 *   - Exposer une API C stable pour piloter le runtime Vitte:
 *       * init/shutdown
 *       * config (workers, blocking pool, features)
 *       * spawn / join (tasks)
 *       * timers (sleep/timeout)
 *       * I/O handles (tcp/file) via sous-APIs (optionnel)
 *       * plugin host (optionnel)
 *   - API "struct-only" pour les échanges (pas de strings libres, pas d'alloc).
 *
 * Conventions ABI:
 *   - Handles opaques (u64) + tags
 *   - Buffers caller-owned (slice ptr+len)
 *   - Status: vitte_status_t (0 OK, <0 erreurs)
 *   - Versioning: api_version + struct_size fields
 *
 * Dépendances:
 *   - vitte_platform.h (status, slices, platform types)
 *   - vitte_abi.h (si tu as déjà un header central, tu peux remplacer)
 * ============================================================================
 */

#pragma once
#ifndef VITTE_RUNTIME_H
#define VITTE_RUNTIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "vitte_platform.h"
/* Optionnel: si tu as vitte_abi.h, tu peux l'inclure ici */
#include "vitte_abi.h"

/* ----------------------------------------------------------------------------
 * ABI versioning
 * ----------------------------------------------------------------------------
 */
#define VITTE_RUNTIME_API_VERSION 1u

typedef uint64_t vitte_handle_u64;

/* ----------------------------------------------------------------------------
 * Opaque handles
 * ----------------------------------------------------------------------------
 */
typedef vitte_handle_u64 vitte_runtime_handle;
typedef vitte_handle_u64 vitte_task_handle;
typedef vitte_handle_u64 vitte_timer_handle;

/* ----------------------------------------------------------------------------
 * Runtime feature flags
 * ----------------------------------------------------------------------------
 */
typedef uint64_t vitte_rt_features;

#define VITTE_RT_FEAT_ASYNC_IO (1ull << 0)
#define VITTE_RT_FEAT_TIMERS (1ull << 1)
#define VITTE_RT_FEAT_NET (1ull << 2)
#define VITTE_RT_FEAT_FS (1ull << 3)
#define VITTE_RT_FEAT_PROCESS (1ull << 4)
#define VITTE_RT_FEAT_SIGNAL (1ull << 5)
#define VITTE_RT_FEAT_PLUGINS (1ull << 6)

#define VITTE_RT_FEAT_DEFAULT                                          \
  (VITTE_RT_FEAT_ASYNC_IO | VITTE_RT_FEAT_TIMERS | VITTE_RT_FEAT_NET | \
   VITTE_RT_FEAT_FS)

/* ----------------------------------------------------------------------------
 * Runtime config
 * ----------------------------------------------------------------------------
 */
typedef struct vitte_runtime_config {
  uint32_t api_version; /* must be VITTE_RUNTIME_API_VERSION */
  uint32_t struct_size; /* sizeof(vitte_runtime_config) */

  uint32_t workers;          /* 0 => auto */
  uint32_t blocking_threads; /* 0 => auto/disabled */

  uint32_t stack_size;     /* 0 => default */
  uint32_t queue_capacity; /* 0 => default */

  vitte_rt_features features; /* bitmask */

  uint64_t reserved0;
  uint64_t reserved1;
} vitte_runtime_config;

VITTE_PLAT_INLINE vitte_runtime_config vitte_runtime_config_default(void) {
  vitte_runtime_config c;
  c.api_version = VITTE_RUNTIME_API_VERSION;
  c.struct_size = (uint32_t)sizeof(vitte_runtime_config);
  c.workers = 0;
  c.blocking_threads = 0;
  c.stack_size = 0;
  c.queue_capacity = 0;
  c.features = VITTE_RT_FEAT_DEFAULT;
  c.reserved0 = 0;
  c.reserved1 = 0;
  return c;
}

/* ----------------------------------------------------------------------------
 * Task entry ABI
 * ----------------------------------------------------------------------------
 */
typedef void (*vitte_task_fn)(void* user);

/* Returned by task join. */
typedef struct vitte_task_result {
  uint32_t tag;    /* 0=ok, 1=panic, 2=canceled */
  uint32_t code;   /* optional */
  uint64_t value0; /* user-defined small return (optional) */
  uint64_t value1;
} vitte_task_result;

#define VITTE_TASK_OK 0u
#define VITTE_TASK_PANIC 1u
#define VITTE_TASK_CANCELED 2u

/* ----------------------------------------------------------------------------
 * Runtime lifecycle
 * ----------------------------------------------------------------------------
 */
VITTE_PLAT_API vitte_status_t vitte_runtime_create(
    const vitte_runtime_config* cfg, vitte_runtime_handle* out_rt);
VITTE_PLAT_API vitte_status_t vitte_runtime_destroy(vitte_runtime_handle rt);

/* Blocks until all tasks drained (optional) */
VITTE_PLAT_API vitte_status_t vitte_runtime_shutdown(vitte_runtime_handle rt);

/* ----------------------------------------------------------------------------
 * Tasking
 * ----------------------------------------------------------------------------
 */
typedef struct vitte_spawn_opts {
  uint32_t api_version;
  uint32_t struct_size;

  uint32_t flags;    /* reserved */
  uint32_t priority; /* reserved */
  uint64_t budget;   /* cooperative budget (ticks) */

  uint64_t reserved0;
} vitte_spawn_opts;

#define VITTE_SPAWN_DETACHED (1u << 0)

VITTE_PLAT_INLINE vitte_spawn_opts vitte_spawn_opts_default(void) {
  vitte_spawn_opts o;
  o.api_version = VITTE_RUNTIME_API_VERSION;
  o.struct_size = (uint32_t)sizeof(vitte_spawn_opts);
  o.flags = 0;
  o.priority = 0;
  o.budget = 0;
  o.reserved0 = 0;
  return o;
}

/* Spawn a task. If DETACHED, out_task may be NULL. */
VITTE_PLAT_API vitte_status_t vitte_task_spawn(vitte_runtime_handle rt,
                                               const vitte_spawn_opts* opts,
                                               vitte_task_fn fn, void* user,
                                               vitte_task_handle* out_task);

VITTE_PLAT_API vitte_status_t vitte_task_cancel(vitte_runtime_handle rt,
                                                vitte_task_handle task);

/* Join task and get result. */
VITTE_PLAT_API vitte_status_t vitte_task_join(vitte_runtime_handle rt,
                                              vitte_task_handle task,
                                              vitte_task_result* out_res);

/* Cooperative yield from within a task. */
VITTE_PLAT_API vitte_status_t vitte_task_yield(vitte_runtime_handle rt);

/* ----------------------------------------------------------------------------
 * Timers
 * ----------------------------------------------------------------------------
 */
typedef struct vitte_sleep_req {
  uint32_t api_version;
  uint32_t struct_size;
  uint64_t duration_ns;
  uint64_t reserved0;
} vitte_sleep_req;

VITTE_PLAT_INLINE vitte_sleep_req vitte_sleep_req_make(uint64_t duration_ns) {
  vitte_sleep_req r;
  r.api_version = VITTE_RUNTIME_API_VERSION;
  r.struct_size = (uint32_t)sizeof(vitte_sleep_req);
  r.duration_ns = duration_ns;
  r.reserved0 = 0;
  return r;
}

/* Async sleep: returns a timer handle that can be awaited/joined. */
VITTE_PLAT_API vitte_status_t vitte_timer_sleep(vitte_runtime_handle rt,
                                                const vitte_sleep_req* req,
                                                vitte_timer_handle* out_timer);

/* Wait a timer (blocking current task) */
VITTE_PLAT_API vitte_status_t vitte_timer_wait(vitte_runtime_handle rt,
                                               vitte_timer_handle timer);

/* Cancel timer */
VITTE_PLAT_API vitte_status_t vitte_timer_cancel(vitte_runtime_handle rt,
                                                 vitte_timer_handle timer);

/* ----------------------------------------------------------------------------
 * Basic IO abstractions (optional surface)
 * ----------------------------------------------------------------------------
 */
typedef vitte_handle_u64 vitte_io_handle;

typedef struct vitte_io_buf {
  uint8_t* ptr;
  uint64_t len;
} vitte_io_buf;

typedef struct vitte_io_rw_result {
  uint64_t n;        /* bytes read/written */
  vitte_status_t st; /* status */
} vitte_io_rw_result;

VITTE_PLAT_API vitte_status_t vitte_io_close(vitte_runtime_handle rt,
                                             vitte_io_handle h);

/* ----------------------------------------------------------------------------
 * TCP (minimal) — optional; you can move to vitte_net.h
 * ----------------------------------------------------------------------------
 */
typedef struct vitte_tcp_bind_req {
  uint32_t api_version;
  uint32_t struct_size;
  vitte_sockaddr addr;
  uint32_t backlog;
  uint32_t flags;
} vitte_tcp_bind_req;

typedef struct vitte_tcp_connect_req {
  uint32_t api_version;
  uint32_t struct_size;
  vitte_sockaddr addr;
  uint32_t flags;
  uint32_t _pad;
} vitte_tcp_connect_req;

typedef struct vitte_tcp_accept_result {
  vitte_io_handle stream;
  vitte_sockaddr peer;
  vitte_status_t st;
  int32_t _pad;
} vitte_tcp_accept_result;

VITTE_PLAT_API vitte_status_t vitte_tcp_bind(vitte_runtime_handle rt,
                                             const vitte_tcp_bind_req* req,
                                             vitte_io_handle* out_listener);
VITTE_PLAT_API vitte_tcp_accept_result
vitte_tcp_accept(vitte_runtime_handle rt, vitte_io_handle listener);
VITTE_PLAT_API vitte_status_t
vitte_tcp_connect(vitte_runtime_handle rt, const vitte_tcp_connect_req* req,
                  vitte_io_handle* out_stream);

VITTE_PLAT_API vitte_io_rw_result vitte_tcp_read(vitte_runtime_handle rt,
                                                 vitte_io_handle stream,
                                                 vitte_io_buf buf);
VITTE_PLAT_API vitte_io_rw_result vitte_tcp_write(vitte_runtime_handle rt,
                                                  vitte_io_handle stream,
                                                  vitte_io_buf buf);

VITTE_PLAT_API vitte_status_t vitte_tcp_set_nodelay(vitte_runtime_handle rt,
                                                    vitte_io_handle stream,
                                                    int32_t enabled);

/* ----------------------------------------------------------------------------
 * Plugins (optional) — host side; you can split to vitte_plugin.h
 * ----------------------------------------------------------------------------
 */
typedef vitte_handle_u64 vitte_plugin_handle;

typedef struct vitte_plugin_load_req {
  uint32_t api_version;
  uint32_t struct_size;
  vitte_str_view path_utf8; /* dylib path or package id */
  uint64_t flags;
} vitte_plugin_load_req;

typedef struct vitte_plugin_info {
  uint32_t api_version;
  uint32_t struct_size;
  uint32_t kind; /* proc_macro/lint/formatter/tool/backend */
  uint32_t abi_version;
  vitte_str_view name;
  vitte_str_view version;
} vitte_plugin_info;

VITTE_PLAT_API vitte_status_t
vitte_plugin_load(vitte_runtime_handle rt, const vitte_plugin_load_req* req,
                  vitte_plugin_handle* out_plug);
VITTE_PLAT_API vitte_status_t vitte_plugin_unload(vitte_runtime_handle rt,
                                                  vitte_plugin_handle plug);
VITTE_PLAT_API vitte_status_t
vitte_plugin_get_info(vitte_runtime_handle rt, vitte_plugin_handle plug,
                      vitte_plugin_info* out_info);

/* ----------------------------------------------------------------------------
 * Diagnostics helpers
 * ----------------------------------------------------------------------------
 */
typedef struct vitte_rt_counters {
  uint64_t tasks_spawned;
  uint64_t tasks_completed;
  uint64_t timers_created;
  uint64_t io_handles;
  uint64_t reserved0;
  uint64_t reserved1;
} vitte_rt_counters;

VITTE_PLAT_API vitte_status_t vitte_runtime_counters(
    vitte_runtime_handle rt, vitte_rt_counters* out_counters);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* VITTE_RUNTIME_H */
