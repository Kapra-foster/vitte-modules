/* C:\Users\vince\Documents\GitHub\vitte-modules\ray-runtime\include\vitte_platform.h
 * ============================================================================
 * vitte_platform.h — Vitte Runtime Platform Abstraction (ABI-facing)
 *
 * Objectifs:
 *   - Définir des types et fonctions "plateforme" stables côté C ABI:
 *       * OS/arch/features
 *       * clocks / time
 *       * threads / futex-ish
 *       * poll/epoll/kqueue/iocp abstraction
 *       * fd/handle abstraction
 *       * process/env/args
 *   - Ne pas exposer d'impl. Les impls vivent dans runtime/src/platform/*
 *
 * Conventions ABI:
 *   - Types à taille fixe (u32/u64, etc.)
 *   - Handles opaques (u64) + tags
 *   - Fonctions retournent vitte_status_t (0 = OK, !=0 error)
 *   - Pas d'alloc implicite (l'appelant fournit buffers)
 *
 * Notes:
 *   - Ce header est conçu pour s'aligner avec vitte_abi.h / vitte_runtime.h.
 * ============================================================================
 */

#pragma once
#ifndef VITTE_PLATFORM_H
#define VITTE_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------
 */
#include <stddef.h>
#include <stdint.h>

/* ----------------------------------------------------------------------------
 * Visibility / inline
 * ----------------------------------------------------------------------------
 */
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(VITTE_PLATFORM_BUILD)
#define VITTE_PLAT_API __declspec(dllexport)
#else
#define VITTE_PLAT_API __declspec(dllimport)
#endif
#define VITTE_PLAT_INLINE __forceinline
#else
#if defined(__GNUC__) || defined(__clang__)
#define VITTE_PLAT_API __attribute__((visibility("default")))
#define VITTE_PLAT_INLINE __attribute__((always_inline)) inline
#else
#define VITTE_PLAT_API
#define VITTE_PLAT_INLINE inline
#endif
#endif

/* ----------------------------------------------------------------------------
 * Status codes
 * ----------------------------------------------------------------------------
 */
typedef int32_t vitte_status_t;

/* Generic */
#define VITTE_OK ((vitte_status_t)0)
#define VITTE_EINVAL ((vitte_status_t) - 22)
#define VITTE_ENOSYS ((vitte_status_t) - 38)
#define VITTE_EPERM ((vitte_status_t) - 1)
#define VITTE_ENOMEM ((vitte_status_t) - 12)
#define VITTE_EBUSY ((vitte_status_t) - 16)
#define VITTE_ETIMEDOUT ((vitte_status_t) - 110)
#define VITTE_ECANCELED ((vitte_status_t) - 125)
#define VITTE_EIO ((vitte_status_t) - 5)
#define VITTE_EAGAIN ((vitte_status_t) - 11)
#define VITTE_EOVERFLOW ((vitte_status_t) - 75)

/* Networking / IO common */
#define VITTE_ECONNRESET ((vitte_status_t) - 104)
#define VITTE_ECONNREFUSED ((vitte_status_t) - 111)
#define VITTE_EADDRINUSE ((vitte_status_t) - 98)
#define VITTE_ENOTCONN ((vitte_status_t) - 107)

/* ----------------------------------------------------------------------------
 * Small ABI helpers
 * ----------------------------------------------------------------------------
 */
typedef struct vitte_slice_u8 {
  const uint8_t* ptr;
  uint64_t len;
} vitte_slice_u8;

typedef struct vitte_mut_slice_u8 {
  uint8_t* ptr;
  uint64_t len;
} vitte_mut_slice_u8;

typedef struct vitte_str_view {
  const char* ptr;
  uint64_t len; /* not null-terminated */
} vitte_str_view;

/* ----------------------------------------------------------------------------
 * Platform identity
 * ----------------------------------------------------------------------------
 */
typedef enum vitte_os_kind {
  VITTE_OS_UNKNOWN = 0,
  VITTE_OS_WINDOWS = 1,
  VITTE_OS_LINUX = 2,
  VITTE_OS_MACOS = 3,
  VITTE_OS_FREEBSD = 4,
  VITTE_OS_NETBSD = 5,
  VITTE_OS_OPENBSD = 6,
  VITTE_OS_ANDROID = 7,
  VITTE_OS_IOS = 8
} vitte_os_kind;

typedef enum vitte_arch_kind {
  VITTE_ARCH_UNKNOWN = 0,
  VITTE_ARCH_X86_64 = 1,
  VITTE_ARCH_AARCH64 = 2,
  VITTE_ARCH_X86 = 3,
  VITTE_ARCH_ARM = 4,
  VITTE_ARCH_RISCV64 = 5
} vitte_arch_kind;

typedef enum vitte_endian {
  VITTE_ENDIAN_UNKNOWN = 0,
  VITTE_ENDIAN_LITTLE = 1,
  VITTE_ENDIAN_BIG = 2
} vitte_endian;

typedef struct vitte_platform_info {
  uint32_t api_version; /* host ABI version (platform API) */
  uint32_t rt_version;  /* runtime version (semver encoded or build id hash) */

  vitte_os_kind os;
  vitte_arch_kind arch;
  vitte_endian endian;

  uint32_t page_size;
  uint32_t cpu_count;

  /* Feature bits (generic) */
  uint64_t features0;
  uint64_t features1;
} vitte_platform_info;

/* Feature bits: features0 */
#define VITTE_FEAT0_TRUECOLOR (1ull << 0)
#define VITTE_FEAT0_HAS_RDTSC (1ull << 1)
#define VITTE_FEAT0_HAS_MONOTONIC (1ull << 2)
#define VITTE_FEAT0_HAS_POLL (1ull << 3)
#define VITTE_FEAT0_HAS_EPOLL (1ull << 4)
#define VITTE_FEAT0_HAS_KQUEUE (1ull << 5)
#define VITTE_FEAT0_HAS_IOCP (1ull << 6)
#define VITTE_FEAT0_HAS_FUTEX (1ull << 7)
#define VITTE_FEAT0_HAS_EVENTFD (1ull << 8)
#define VITTE_FEAT0_HAS_TIMERFD (1ull << 9)

/* ----------------------------------------------------------------------------
 * Time / clocks
 * ----------------------------------------------------------------------------
 */
typedef enum vitte_clock_kind {
  VITTE_CLOCK_REALTIME = 0,
  VITTE_CLOCK_MONOTONIC = 1,
  VITTE_CLOCK_PROCESS = 2,
  VITTE_CLOCK_THREAD = 3
} vitte_clock_kind;

typedef struct vitte_time_spec {
  int64_t sec;
  int32_t nsec;
  int32_t _pad;
} vitte_time_spec;

typedef struct vitte_instant {
  uint64_t ticks; /* opaque ticks from monotonic clock */
  uint64_t freq;  /* ticks per second if needed */
} vitte_instant;

VITTE_PLAT_API vitte_status_t
vitte_platform_get_info(vitte_platform_info* out_info);

/* Read clocks */
VITTE_PLAT_API vitte_status_t vitte_clock_now(vitte_clock_kind clk,
                                              vitte_time_spec* out_ts);
VITTE_PLAT_API vitte_status_t vitte_instant_now(vitte_instant* out_inst);
VITTE_PLAT_API vitte_status_t
vitte_instant_elapsed_ns(const vitte_instant* start, uint64_t* out_ns);

/* Sleep */
VITTE_PLAT_API vitte_status_t vitte_sleep_ns(uint64_t ns);

/* ----------------------------------------------------------------------------
 * Threading primitives (minimal)
 * ----------------------------------------------------------------------------
 */
typedef uint64_t vitte_thread_handle;

typedef struct vitte_thread_start {
  void (*entry)(void* user);
  void* user;
  uint32_t stack_size; /* 0 => default */
  uint32_t flags;      /* reserved */
} vitte_thread_start;

VITTE_PLAT_API vitte_status_t vitte_thread_spawn(
    const vitte_thread_start* start, vitte_thread_handle* out_th);
VITTE_PLAT_API vitte_status_t vitte_thread_join(vitte_thread_handle th);
VITTE_PLAT_API vitte_status_t vitte_thread_detach(vitte_thread_handle th);
VITTE_PLAT_API vitte_status_t vitte_thread_yield(void);
VITTE_PLAT_API vitte_status_t vitte_thread_current_id(uint64_t* out_tid);

/* ----------------------------------------------------------------------------
 * Futex / wait-wake (best-effort abstraction)
 * ----------------------------------------------------------------------------
 */
typedef enum vitte_wait_kind {
  VITTE_WAIT_FUTEX = 1,
  VITTE_WAIT_EVENT = 2
} vitte_wait_kind;

/* Wait on *addr == expected, wake N waiters. */
VITTE_PLAT_API vitte_status_t vitte_wait_u32(const uint32_t* addr,
                                             uint32_t expected,
                                             uint64_t timeout_ns);
VITTE_PLAT_API vitte_status_t vitte_wake_u32(const uint32_t* addr,
                                             uint32_t count);

/* ----------------------------------------------------------------------------
 * File descriptor / handle abstraction
 * ----------------------------------------------------------------------------
 */
typedef enum vitte_handle_kind {
  VITTE_HK_INVALID = 0,
  VITTE_HK_FD = 1,    /* POSIX fd in low 32 bits */
  VITTE_HK_HANDLE = 2 /* Win32 HANDLE in full 64 bits */
} vitte_handle_kind;

typedef struct vitte_handle {
  uint64_t raw;
  vitte_handle_kind kind;
  uint32_t flags;
} vitte_handle;

#define VITTE_HANDLE_FLAG_CLOEXEC (1u << 0)
#define VITTE_HANDLE_FLAG_NONBLOCK (1u << 1)

VITTE_PLAT_API vitte_handle vitte_handle_invalid(void);
VITTE_PLAT_API int32_t vitte_handle_is_valid(vitte_handle h);

/* ----------------------------------------------------------------------------
 * Polling / event loop abstraction
 * ----------------------------------------------------------------------------
 */
typedef uint64_t vitte_poller_handle;

typedef enum vitte_io_interest {
  VITTE_IO_READ = 1u << 0,
  VITTE_IO_WRITE = 1u << 1,
  VITTE_IO_HUP = 1u << 2,
  VITTE_IO_ERR = 1u << 3
} vitte_io_interest;

typedef struct vitte_io_event {
  uint64_t token; /* user token */
  uint32_t flags; /* vitte_io_interest bits */
  uint32_t _pad;
} vitte_io_event;

typedef struct vitte_io_reg {
  vitte_handle handle;
  uint64_t token;
  uint32_t interests; /* vitte_io_interest */
  uint32_t flags;     /* reserved */
} vitte_io_reg;

VITTE_PLAT_API vitte_status_t
vitte_poller_create(vitte_poller_handle* out_poller);
VITTE_PLAT_API vitte_status_t vitte_poller_destroy(vitte_poller_handle poller);

VITTE_PLAT_API vitte_status_t vitte_poller_register(vitte_poller_handle poller,
                                                    const vitte_io_reg* reg);
VITTE_PLAT_API vitte_status_t
vitte_poller_reregister(vitte_poller_handle poller, const vitte_io_reg* reg);
VITTE_PLAT_API vitte_status_t
vitte_poller_deregister(vitte_poller_handle poller, vitte_handle handle);

/* Wait for events; writes up to out_events.len events, returns count in
 * out_count */
VITTE_PLAT_API vitte_status_t vitte_poller_wait(
    vitte_poller_handle poller,
    vitte_mut_slice_u8 out_events_bytes, /* array of vitte_io_event */
    uint64_t timeout_ns, uint64_t* out_count);

/* ----------------------------------------------------------------------------
 * Networking helpers (addr parsing)
 * ----------------------------------------------------------------------------
 */
typedef struct vitte_sockaddr {
  uint8_t bytes[128];
  uint32_t len;
  uint32_t family;
} vitte_sockaddr;

VITTE_PLAT_API vitte_status_t
vitte_sockaddr_loopback_ipv4(uint16_t port, vitte_sockaddr* out_addr);
VITTE_PLAT_API vitte_status_t vitte_sockaddr_parse(vitte_str_view s,
                                                   vitte_sockaddr* out_addr);

/* ----------------------------------------------------------------------------
 * Process / env / args
 * ----------------------------------------------------------------------------
 */
typedef struct vitte_env_kv {
  vitte_str_view key;
  vitte_str_view val;
} vitte_env_kv;

VITTE_PLAT_API vitte_status_t vitte_get_exe_path(vitte_mut_slice_u8 out_utf8,
                                                 uint64_t* out_len);
VITTE_PLAT_API vitte_status_t vitte_get_cwd(vitte_mut_slice_u8 out_utf8,
                                            uint64_t* out_len);

VITTE_PLAT_API vitte_status_t vitte_env_get(vitte_str_view key,
                                            vitte_mut_slice_u8 out_utf8,
                                            uint64_t* out_len);
VITTE_PLAT_API vitte_status_t vitte_env_set(vitte_str_view key,
                                            vitte_str_view val);
VITTE_PLAT_API vitte_status_t vitte_env_unset(vitte_str_view key);

VITTE_PLAT_API vitte_status_t vitte_args_count(uint32_t* out_count);
VITTE_PLAT_API vitte_status_t vitte_args_get(uint32_t index,
                                             vitte_mut_slice_u8 out_utf8,
                                             uint64_t* out_len);

/* ----------------------------------------------------------------------------
 * Debug / tracing hooks (optional)
 * ----------------------------------------------------------------------------
 */
typedef void (*vitte_log_sink_fn)(uint32_t level, vitte_str_view target,
                                  vitte_str_view msg, void* user);

VITTE_PLAT_API vitte_status_t vitte_log_set_sink(vitte_log_sink_fn fn,
                                                 void* user);
VITTE_PLAT_API vitte_status_t vitte_log_flush(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* VITTE_PLATFORM_H */
