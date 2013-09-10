/**
 * Seccomp Library
 *
 * Copyright (c) 2012,2013 Red Hat <pmoore@redhat.com>
 * Author: Paul Moore <pmoore@redhat.com>
 */

/*
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of version 2.1 of the GNU Lesser General Public License as
 * published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses>.
 */

#ifndef _SECCOMP_H
#define _SECCOMP_H

#include <elf.h>
#include <inttypes.h>
#include <asm/unistd.h>
#include <linux/audit.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * version information
 */

#define SCMP_VER_MAJOR		2
#define SCMP_VER_MINOR		1
#define SCMP_VER_MICRO		0

/*
 * types
 */

/**
 * Filter context/handle
 */
typedef void * scmp_filter_ctx;

/**
 * Filter attributes
 */
enum scmp_filter_attr {
	_SCMP_FLTATR_MIN = 0,
	SCMP_FLTATR_ACT_DEFAULT = 1,	/**< default filter action */
	SCMP_FLTATR_ACT_BADARCH = 2,	/**< bad architecture action */
	SCMP_FLTATR_CTL_NNP = 3,	/**< set NO_NEW_PRIVS on filter load */
	_SCMP_FLTATR_MAX,
};

/**
 * Comparison operators
 */
enum scmp_compare {
	_SCMP_CMP_MIN = 0,
	SCMP_CMP_NE = 1,		/**< not equal */
	SCMP_CMP_LT = 2,		/**< less than */
	SCMP_CMP_LE = 3,		/**< less than or equal */
	SCMP_CMP_EQ = 4,		/**< equal */
	SCMP_CMP_GE = 5,		/**< greater than or equal */
	SCMP_CMP_GT = 6,		/**< greater than */
	SCMP_CMP_MASKED_EQ = 7,		/**< masked equality */
	_SCMP_CMP_MAX,
};

/**
 * Argument datum
 */
typedef uint64_t scmp_datum_t;

/**
 * Argument / Value comparison definition
 */
struct scmp_arg_cmp {
	unsigned int arg;	/**< argument number, starting at 0 */
	enum scmp_compare op;	/**< the comparison op, e.g. SCMP_CMP_* */
	scmp_datum_t datum_a;
	scmp_datum_t datum_b;
};

/*
 * macros/defines
 */

/**
 * The native architecture token
 */
#define SCMP_ARCH_NATIVE	0

/**
 * The x86 (32-bit) architecture token
 */
#define SCMP_ARCH_X86		AUDIT_ARCH_I386

/**
 * The x86-64 (64-bit) architecture token
 */
#define SCMP_ARCH_X86_64	AUDIT_ARCH_X86_64

/**
 * The x32 (32-bit x86_64) architecture token
 *
 * NOTE: this is different from the value used by the kernel because we need to
 * be able to distinguish between x32 and x86_64
 */
#define SCMP_ARCH_X32		(EM_X86_64|__AUDIT_ARCH_LE)

/**
 * The ARM architecture token
 */
#define SCMP_ARCH_ARM		AUDIT_ARCH_ARM

/**
 * Convert a syscall name into the associated syscall number
 * @param x the syscall name
 */
#define SCMP_SYS(x)		(__NR_##x)

/**
 * Specify an argument comparison struct for use in declaring rules
 * @param arg the argument number, starting at 0
 * @param op the comparison operator, e.g. SCMP_CMP_*
 * @param datum_a dependent on comparison
 * @param datum_b dependent on comparison, optional
 */
#define SCMP_CMP(...)		((struct scmp_arg_cmp){__VA_ARGS__})

/**
 * Specify an argument comparison struct for argument 0
 */
#define SCMP_A0(...)		SCMP_CMP(0, __VA_ARGS__)

/**
 * Specify an argument comparison struct for argument 1
 */
#define SCMP_A1(...)		SCMP_CMP(1, __VA_ARGS__)

/**
 * Specify an argument comparison struct for argument 2
 */
#define SCMP_A2(...)		SCMP_CMP(2, __VA_ARGS__)

/**
 * Specify an argument comparison struct for argument 3
 */
#define SCMP_A3(...)		SCMP_CMP(3, __VA_ARGS__)

/**
 * Specify an argument comparison struct for argument 4
 */
#define SCMP_A4(...)		SCMP_CMP(4, __VA_ARGS__)

/**
 * Specify an argument comparison struct for argument 5
 */
#define SCMP_A5(...)		SCMP_CMP(5, __VA_ARGS__)

/*
 * seccomp actions
 */

/**
 * Kill the process
 */
#define SCMP_ACT_KILL		0x00000000U
/**
 * Throw a SIGSYS signal
 */
#define SCMP_ACT_TRAP		0x00030000U
/**
 * Return the specified error code
 */
#define SCMP_ACT_ERRNO(x)	(0x00050000U | ((x) & 0x0000ffffU))
/**
 * Notify a tracing process with the specified value
 */
#define SCMP_ACT_TRACE(x)	(0x7ff00000U | ((x) & 0x0000ffffU))
/**
 * Allow the syscall to be executed
 */
#define SCMP_ACT_ALLOW		0x7fff0000U

/*
 * functions
 */

/**
 * Initialize the filter state
 * @param def_action the default filter action
 *
 * This function initializes the internal seccomp filter state and should
 * be called before any other functions in this library to ensure the filter
 * state is initialized.  Returns a filter context on success, NULL on failure.
 *
 */
scmp_filter_ctx seccomp_init(uint32_t def_action);

/**
 * Reset the filter state
 * @param ctx the filter context
 * @param def_action the default filter action
 *
 * This function resets the given seccomp filter state and ensures the
 * filter state is reinitialized.  This function does not reset any seccomp
 * filters already loaded into the kernel.  Returns zero on success, negative
 * values on failure.
 *
 */
int seccomp_reset(scmp_filter_ctx ctx, uint32_t def_action);

/**
 * Destroys the filter state and releases any resources
 * @param ctx the filter context
 *
 * This functions destroys the given seccomp filter state and releases any
 * resources, including memory, associated with the filter state.  This
 * function does not reset any seccomp filters already loaded into the kernel.
 * The filter context can no longer be used after calling this function.
 *
 */
void seccomp_release(scmp_filter_ctx ctx);

/**
 * Merge two filters
 * @param ctx_dst the destination filter context
 * @param ctx_src the source filter context
 *
 * This function merges two filter contexts into a single filter context and
 * destroys the second filter context.  The two filter contexts must have the
 * same attribute values and not contain any of the same architectures; if they
 * do, the merge operation will fail.  On success, the source filter context
 * will be destroyed and should no longer be used; it is not necessary to
 * call seccomp_release() on the source filter context.  Returns zero on
 * success, negative values on failure.
 *
 */
int seccomp_merge(scmp_filter_ctx ctx_dst, scmp_filter_ctx ctx_src);

/**
 * Return the native architecture token
 *
 * This function returns the native architecture token value, e.g. SCMP_ARCH_*.
 *
 */
uint32_t seccomp_arch_native(void);

/**
 * Check to see if an existing architecture is present in the filter
 * @param ctx the filter context
 * @param arch_token the architecture token, e.g. SCMP_ARCH_*
 *
 * This function tests to see if a given architecture is included in the filter
 * context.  If the architecture token is SCMP_ARCH_NATIVE then the native
 * architecture will be assumed.  Returns zero if the architecture exists in
 * the filter, -EEXIST if it is not present, and other negative values on
 * failure.
 *
 */
int seccomp_arch_exist(const scmp_filter_ctx ctx, uint32_t arch_token);

/**
 * Adds an architecture to the filter
 * @param ctx the filter context
 * @param arch_token the architecture token, e.g. SCMP_ARCH_*
 *
 * This function adds a new architecture to the given seccomp filter context.
 * Any new rules added after this function successfully returns will be added
 * to this architecture but existing rules will not be added to this
 * architecture.  If the architecture token is SCMP_ARCH_NATIVE then the native
 * architecture will be assumed.  Returns zero on success, negative values on
 * failure.
 *
 */
int seccomp_arch_add(scmp_filter_ctx ctx, uint32_t arch_token);

/**
 * Removes an architecture from the filter
 * @param ctx the filter context
 * @param arch_token the architecture token, e.g. SCMP_ARCH_*
 *
 * This function removes an architecture from the given seccomp filter context.
 * If the architecture token is SCMP_ARCH_NATIVE then the native architecture
 * will be assumed.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_arch_remove(scmp_filter_ctx ctx, uint32_t arch_token);

/**
 * Loads the filter into the kernel
 * @param ctx the filter context
 *
 * This function loads the given seccomp filter context into the kernel.  If
 * the filter was loaded correctly, the kernel will be enforcing the filter
 * when this function returns.  Returns zero on success, negative values on
 * error.
 *
 */
int seccomp_load(const scmp_filter_ctx ctx);

/**
 * Get the value of a filter attribute
 * @param ctx the filter context
 * @param attr the filter attribute name
 * @param value the filter attribute value
 *
 * This function fetches the value of the given attribute name and returns it
 * via @value.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_attr_get(const scmp_filter_ctx ctx,
		     enum scmp_filter_attr attr, uint32_t *value);

/**
 * Set the value of a filter attribute
 * @param ctx the filter context
 * @param attr the filter attribute name
 * @param value the filter attribute value
 *
 * This function sets the value of the given attribute.  Returns zero on
 * success, negative values on failure.
 *
 */
int seccomp_attr_set(scmp_filter_ctx ctx,
		     enum scmp_filter_attr attr, uint32_t value);

/**
 * Resolve a syscall number to a name
 * @param arch_token the architecture token, e.g. SCMP_ARCH_*
 * @param num the syscall number
 *
 * Resolve the given syscall number to the syscall name for the given
 * architecture; it is up to the caller to free the returned string.  Returns
 * the syscall name on success, NULL on failure.
 *
 */
char *seccomp_syscall_resolve_num_arch(uint32_t arch_token, int num);

/**
 * Resolve a syscall name to a number
 * @param arch_token the architecture token, e.g. SCMP_ARCH_*
 * @param name the syscall name
 *
 * Resolve the given syscall name to the syscall number for the given
 * architecture.  Returns the syscall number on success, including negative
 * pseudo syscall numbers (e.g. __PNR_*); returns __NR_SCMP_ERROR on failure.
 *
 */
int seccomp_syscall_resolve_name_arch(uint32_t arch_token, const char *name);

/**
 * Resolve a syscall name to a number
 * @param name the syscall name
 *
 * Resolve the given syscall name to the syscall number.  Returns the syscall
 * number on success, including negative pseudo syscall numbers (e.g. __PNR_*);
 * returns __NR_SCMP_ERROR on failure.
 *
 */
int seccomp_syscall_resolve_name(const char *name);

/**
 * Set the priority of a given syscall
 * @param ctx the filter context
 * @param syscall the syscall number
 * @param priority priority value, higher value == higher priority
 *
 * This function sets the priority of the given syscall; this value is used
 * when generating the seccomp filter code such that higher priority syscalls
 * will incur less filter code overhead than the lower priority syscalls in the
 * filter.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_syscall_priority(scmp_filter_ctx ctx,
			     int syscall, uint8_t priority);

/**
 * Add a new rule to the filter
 * @param ctx the filter context
 * @param action the filter action
 * @param syscall the syscall number
 * @param arg_cnt the number of argument filters in the argument filter chain
 * @param ... scmp_arg_cmp structs (use of SCMP_ARG_CMP() recommended)
 *
 * This function adds a series of new argument/value checks to the seccomp
 * filter for the given syscall; multiple argument/value checks can be
 * specified and they will be chained together (AND'd together) in the filter.
 * If the specified rule needs to be adjusted due to architecture specifics it
 * will be adjusted without notification.  Returns zero on success, negative
 * values on failure.
 *
 */
int seccomp_rule_add(scmp_filter_ctx ctx,
		     uint32_t action, int syscall, unsigned int arg_cnt, ...);


/**
 * Add a new rule to the filter
 * @param ctx the filter context
 * @param action the filter action
 * @param syscall the syscall number
 * @param arg_cnt the number of elements in the arg_array parameter
 * @param arg_array array of scmp_arg_cmp structs
 *
 * This function adds a series of new argument/value checks to the seccomp
 * filter for the given syscall; multiple argument/value checks can be
 * specified and they will be chained together (AND'd together) in the filter.
 * If the specified rule needs to be adjusted due to architecture specifics it
 * will be adjusted without notification.  Returns zero on success, negative
 * values on failure.
 *
 */
int seccomp_rule_add_array(scmp_filter_ctx ctx,
			   uint32_t action, int syscall, unsigned int arg_cnt,
			   const struct scmp_arg_cmp *arg_array);

/**
 * Add a new rule to the filter
 * @param ctx the filter context
 * @param action the filter action
 * @param syscall the syscall number
 * @param arg_cnt the number of argument filters in the argument filter chain
 * @param ... scmp_arg_cmp structs (use of SCMP_ARG_CMP() recommended)
 *
 * This function adds a series of new argument/value checks to the seccomp
 * filter for the given syscall; multiple argument/value checks can be
 * specified and they will be chained together (AND'd together) in the filter.
 * If the specified rule can not be represented on the architecture the
 * function will fail.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_rule_add_exact(scmp_filter_ctx ctx, uint32_t action,
			   int syscall, unsigned int arg_cnt, ...);

/**
 * Add a new rule to the filter
 * @param ctx the filter context
 * @param action the filter action
 * @param syscall the syscall number
 * @param arg_cnt  the number of elements in the arg_array parameter
 * @param arg_array array of scmp_arg_cmp structs
 *
 * This function adds a series of new argument/value checks to the seccomp
 * filter for the given syscall; multiple argument/value checks can be
 * specified and they will be chained together (AND'd together) in the filter.
 * If the specified rule can not be represented on the architecture the
 * function will fail.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_rule_add_exact_array(scmp_filter_ctx ctx,
				 uint32_t action, int syscall,
				 unsigned int arg_cnt,
				 const struct scmp_arg_cmp *arg_array);

/**
 * Generate seccomp Pseudo Filter Code (PFC) and export it to a file
 * @param ctx the filter context
 * @param fd the destination fd
 *
 * This function generates seccomp Pseudo Filter Code (PFC) and writes it to
 * the given fd.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_export_pfc(const scmp_filter_ctx ctx, int fd);

/**
 * Generate seccomp Berkley Packet Filter (BPF) code and export it to a file
 * @param ctx the filter context
 * @param fd the destination fd
 *
 * This function generates seccomp Berkley Packer Filter (BPF) code and writes
 * it to the given fd.  Returns zero on success, negative values on failure.
 *
 */
int seccomp_export_bpf(const scmp_filter_ctx ctx, int fd);

/*
 * pseudo syscall definitions
 */

/* NOTE - pseudo syscall values {-1..-99} are reserved */
#define __NR_SCMP_ERROR		-1

/* socket syscalls */

#define __PNR_socket		-101
#ifndef __NR_socket
#define __NR_socket		__PNR_socket
#endif /* __NR_socket */

#define __PNR_bind		-102
#ifndef __NR_bind
#define __NR_bind		__PNR_bind
#endif /* __NR_bind */

#define __PNR_connect		-103
#ifndef __NR_connect
#define __NR_connect		__PNR_connect
#endif /* __NR_connect */

#define __PNR_listen		-104
#ifndef __NR_listen
#define __NR_listen		__PNR_listen
#endif /* __NR_listen */

#define __PNR_accept		-105
#ifndef __NR_accept
#define __NR_accept		__PNR_accept
#endif /* __NR_accept */

#define __PNR_getsockname	-106
#ifndef __NR_getsockname
#define __NR_getsockname	__PNR_getsockname
#endif /* __NR_getsockname */

#define __PNR_getpeername	-107
#ifndef __NR_getpeername
#define __NR_getpeername	__PNR_getpeername
#endif /* __NR_getpeername */

#define __PNR_socketpair	-108
#ifndef __NR_socketpair
#define __NR_socketpair		__PNR_socketpair
#endif /* __NR_socketpair */

#define __PNR_send		-109
#ifndef __NR_send
#define __NR_send		__PNR_send
#endif /* __NR_send */

#define __PNR_recv		-110
#ifndef __NR_recv
#define __NR_recv		__PNR_recv
#endif /* __NR_recv */

#define __PNR_sendto		-111
#ifndef __NR_sendto
#define __NR_sendto		__PNR_sendto
#endif /* __NR_sendto */

#define __PNR_recvfrom		-112
#ifndef __NR_recvfrom
#define __NR_recvfrom		__PNR_recvfrom
#endif /* __NR_recvfrom */

#define __PNR_shutdown		-113
#ifndef __NR_shutdown
#define __NR_shutdown		__PNR_shutdown
#endif /* __NR_shutdown */

#define __PNR_setsockopt	-114
#ifndef __NR_setsockopt
#define __NR_setsockopt		__PNR_setsockopt
#endif /* __NR_getsockopt */

#define __PNR_getsockopt	-115
#ifndef __NR_getsockopt
#define __NR_getsockopt		__PNR_getsockopt
#endif /* __NR_getsockopt */

#define __PNR_sendmsg		-116
#ifndef __NR_sendmsg
#define __NR_sendmsg		__PNR_sendmsg
#endif /* __NR_sendmsg */

#define __PNR_recvmsg		-117
#ifndef __NR_recvmsg
#define __NR_recvmsg		__PNR_recvmsg
#endif /* __NR_recvmsg */

#define __PNR_accept4		-118
#ifndef __NR_accept4
#define __NR_accept4		__PNR_accept4
#endif /* __NR_accept4 */

#define __PNR_recvmmsg		-119
#ifndef __NR_recvmmsg
#define __NR_recvmmsg		__PNR_recvmmsg
#endif /* __NR_recvmmsg */

#define __PNR_sendmmsg		-120
#ifndef __NR_sendmmsg
#define __NR_sendmmsg		__PNR_sendmmsg
#endif /* __NR_sendmmsg */

/* ipc syscalls */

#define __PNR_semop		-201
#ifndef __NR_semop
#define __NR_semop		__PNR_semop
#endif /* __NR_semop */

#define __PNR_semget		-202
#ifndef __NR_semget
#define __NR_semget		__PNR_semget
#endif /* __NR_semget */

#define __PNR_semctl		-203
#ifndef __NR_semctl
#define __NR_semctl		__PNR_semctl
#endif /* __NR_semctl */

#define __PNR_semtimedop	-204
#ifndef __NR_semtimedop
#define __NR_semtimedop		__PNR_semtimedop
#endif /* __NR_semtime */

#define __PNR_msgsnd		-211
#ifndef __NR_msgsnd
#define __NR_msgsnd		__PNR_msgsnd
#endif /* __NR_msgsnd */

#define __PNR_msgrcv		-212
#ifndef __NR_msgrcv
#define __NR_msgrcv		__PNR_msgrcv
#endif /* __NR_msgrcv */

#define __PNR_msgget		-213
#ifndef __NR_msgget
#define __NR_msgget		__PNR_msgget
#endif /* __NR_msgget */

#define __PNR_msgctl		-214
#ifndef __NR_msgctl
#define __NR_msgctl		__PNR_msgctl
#endif /* __NR_msgctl */

#define __PNR_shmat		-221
#ifndef __NR_shmat
#define __NR_shmat		__PNR_shmat
#endif /* __NR_shmat */

#define __PNR_shmdt		-222
#ifndef __NR_shmdt
#define __NR_shmdt		__PNR_shmdt
#endif /* __NR_shmdt */

#define __PNR_shmget		-223
#ifndef __NR_shmget
#define __NR_shmget		__PNR_shmget
#endif /* __NR_shmget */

#define __PNR_shmctl		-224
#ifndef __NR_shmctl
#define __NR_shmctl		__PNR_shmctl
#endif /* __NR_shmctl */

/* single syscalls */

#define __PNR_arch_prctl	-10001
#ifndef __NR_arch_prctl
#define __NR_arch_prctl		__PNR_arch_prctl
#endif /* __NR_arch_prctl */

#define __PNR_bdflush		-10002
#ifndef __NR_bdflush
#define __NR_bdflush		__PNR_bdflush
#endif /* __NR_bdflush */

#define __PNR_break		-10003
#ifndef __NR_break
#define __NR_break		__PNR_break
#endif /* __NR_break */

#define __PNR_chown32		-10004
#ifndef __NR_chown32
#define __NR_chown32		__PNR_chown32
#endif /* __NR_chown32 */

#define __PNR_epoll_ctl_old	-10005
#ifndef __NR_epoll_ctl_old
#define __NR_epoll_ctl_old	__PNR_epoll_ctl_old
#endif /* __NR_epoll_ctl_old */

#define __PNR_epoll_wait_old	-10006
#ifndef __NR_epoll_wait_old
#define __NR_epoll_wait_old	__PNR_epoll_wait_old
#endif /* __NR_epoll_wait_old */

#define __PNR_fadvise64_64	-10007
#ifndef __NR_fadvise64_64
#define __NR_fadvise64_64	__PNR_fadvise64_64
#endif /* __NR_fadvise64_64 */

#define __PNR_fchown32		-10008
#ifndef __NR_fchown32
#define __NR_fchown32		__PNR_fchown32
#endif /* __NR_fchown32 */

#define __PNR_fcntl64		-10009
#ifndef __NR_fcntl64
#define __NR_fcntl64		__PNR_fcntl64
#endif /* __NR_fcntl64 */

#define __PNR_fstat64		-10010
#ifndef __NR_fstat64
#define __NR_fstat64		__PNR_fstat64
#endif /* __NR_fstat64 */

#define __PNR_fstatat64		-10011
#ifndef __NR_fstatat64
#define __NR_fstatat64		__PNR_fstatat64
#endif /* __NR_fstatat64 */

#define __PNR_fstatfs64		-10012
#ifndef __NR_fstatfs64
#define __NR_fstatfs64		__PNR_fstatfs64
#endif /* __NR_fstatfs64 */

#define __PNR_ftime		-10013
#ifndef __NR_ftime
#define __NR_ftime		__PNR_ftime
#endif /* __NR_ftime */

#define __PNR_ftruncate64	-10014
#ifndef __NR_ftruncate64
#define __NR_ftruncate64	__PNR_ftruncate64
#endif /* __NR_ftruncate64 */

#define __PNR_getegid32		-10015
#ifndef __NR_getegid32
#define __NR_getegid32		__PNR_getegid32
#endif /* __NR_getegid32 */

#define __PNR_geteuid32		-10016
#ifndef __NR_geteuid32
#define __NR_geteuid32		__PNR_geteuid32
#endif /* __NR_geteuid32 */

#define __PNR_getgid32		-10017
#ifndef __NR_getgid32
#define __NR_getgid32		__PNR_getgid32
#endif /* __NR_getgid32 */

#define __PNR_getgroups32	-10018
#ifndef __NR_getgroups32
#define __NR_getgroups32	__PNR_getgroups32
#endif /* __NR_getgroups32 */

#define __PNR_getresgid32	-10019
#ifndef __NR_getresgid32
#define __NR_getresgid32	__PNR_getresgid32
#endif /* __NR_getresgid32 */

#define __PNR_getresuid32	-10020
#ifndef __NR_getresuid32
#define __NR_getresuid32	__PNR_getresuid32
#endif /* __NR_getresuid32 */

#define __PNR_getuid32		-10021
#ifndef __NR_getuid32
#define __NR_getuid32		__PNR_getuid32
#endif /* __NR_getuid32 */

#define __PNR_gtty		-10022
#ifndef __NR_gtty
#define __NR_gtty		__PNR_gtty
#endif /* __NR_gtty */

#define __PNR_idle		-10023
#ifndef __NR_idle
#define __NR_idle		__PNR_idle
#endif /* __NR_idle */

#define __PNR_ipc		-10024
#ifndef __NR_ipc
#define __NR_ipc		__PNR_ipc
#endif /* __NR_ipc */

#define __PNR_lchown32		-10025
#ifndef __NR_lchown32
#define __NR_lchown32		__PNR_lchown32
#endif /* __NR_lchown32 */

#define __PNR__llseek		-10026
#ifndef __NR__llseek
#define __NR__llseek		__PNR__llseek
#endif /* __NR__llseek */

#define __PNR_lock		-10027
#ifndef __NR_lock
#define __NR_lock		__PNR_lock
#endif /* __NR_lock */

#define __PNR_lstat64		-10028
#ifndef __NR_lstat64
#define __NR_lstat64		__PNR_lstat64
#endif /* __NR_lstat64 */

#define __PNR_mmap2		-10029
#ifndef __NR_mmap2
#define __NR_mmap2		__PNR_mmap2
#endif /* __NR_mmap2 */

#define __PNR_mpx		-10030
#ifndef __NR_mpx
#define __NR_mpx		__PNR_mpx
#endif /* __NR_mpx */

#define __PNR_newfstatat	-10031
#ifndef __NR_newfstatat
#define __NR_newfstatat		__PNR_newfstatat
#endif /* __NR_newfstatat */

#define __PNR__newselect	-10032
#ifndef __NR__newselect
#define __NR__newselect		__PNR__newselect
#endif /* __NR__newselect */

#define __PNR_nice		-10033
#ifndef __NR_nice
#define __NR_nice		__PNR_nice
#endif /* __NR_nice */

#define __PNR_oldfstat		-10034
#ifndef __NR_oldfstat
#define __NR_oldfstat		__PNR_oldfstat
#endif /* __NR_oldfstat */

#define __PNR_oldlstat		-10035
#ifndef __NR_oldlstat
#define __NR_oldlstat		__PNR_oldlstat
#endif /* __NR_oldlstat */

#define __PNR_oldolduname	-10036
#ifndef __NR_oldolduname
#define __NR_oldolduname	__PNR_oldolduname
#endif /* __NR_oldolduname */

#define __PNR_oldstat		-10037
#ifndef __NR_oldstat
#define __NR_oldstat		__PNR_oldstat
#endif /* __NR_oldstat */

#define __PNR_olduname		-10038
#ifndef __NR_olduname
#define __NR_olduname		__PNR_olduname
#endif /* __NR_olduname */

#define __PNR_prof		-10039
#ifndef __NR_prof
#define __NR_prof		__PNR_prof
#endif /* __NR_prof */

#define __PNR_profil		-10040
#ifndef __NR_profil
#define __NR_profil		__PNR_profil
#endif /* __NR_profil */

#define __PNR_readdir		-10041
#ifndef __NR_readdir
#define __NR_readdir		__PNR_readdir
#endif /* __NR_readdir */

#define __PNR_security		-10042
#ifndef __NR_security
#define __NR_security		__PNR_security
#endif /* __NR_security */

#define __PNR_sendfile64	-10043
#ifndef __NR_sendfile64
#define __NR_sendfile64		__PNR_sendfile64
#endif /* __NR_sendfile64 */

#define __PNR_setfsgid32	-10044
#ifndef __NR_setfsgid32
#define __NR_setfsgid32		__PNR_setfsgid32
#endif /* __NR_setfsgid32 */

#define __PNR_setfsuid32	-10045
#ifndef __NR_setfsuid32
#define __NR_setfsuid32		__PNR_setfsuid32
#endif /* __NR_setfsuid32 */

#define __PNR_setgid32		-10046
#ifndef __NR_setgid32
#define __NR_setgid32		__PNR_setgid32
#endif /* __NR_setgid32 */

#define __PNR_setgroups32	-10047
#ifndef __NR_setgroups32
#define __NR_setgroups32	__PNR_setgroups32
#endif /* __NR_setgroups32 */

#define __PNR_setregid32	-10048
#ifndef __NR_setregid32
#define __NR_setregid32		__PNR_setregid32
#endif /* __NR_setregid32 */

#define __PNR_setresgid32	-10049
#ifndef __NR_setresgid32
#define __NR_setresgid32	__PNR_setresgid32
#endif /* __NR_setresgid32 */

#define __PNR_setresuid32	-10050
#ifndef __NR_setresuid32
#define __NR_setresuid32	__PNR_setresuid32
#endif /* __NR_setresuid32 */

#define __PNR_setreuid32	-10051
#ifndef __NR_setreuid32
#define __NR_setreuid32		__PNR_setreuid32
#endif /* __NR_setreuid32 */

#define __PNR_setuid32		-10052
#ifndef __NR_setuid32
#define __NR_setuid32		__PNR_setuid32
#endif /* __NR_setuid32 */

#define __PNR_sgetmask		-10053
#ifndef __NR_sgetmask
#define __NR_sgetmask		__PNR_sgetmask
#endif /* __NR_sgetmask */

#define __PNR_sigaction		-10054
#ifndef __NR_sigaction
#define __NR_sigaction		__PNR_sigaction
#endif /* __NR_sigaction */

#define __PNR_signal		-10055
#ifndef __NR_signal
#define __NR_signal		__PNR_signal
#endif /* __NR_signal */

#define __PNR_sigpending	-10056
#ifndef __NR_sigpending
#define __NR_sigpending		__PNR_sigpending
#endif /* __NR_sigpending */

#define __PNR_sigprocmask	-10057
#ifndef __NR_sigprocmask
#define __NR_sigprocmask	__PNR_sigprocmask
#endif /* __NR_sigprocmask */

#define __PNR_sigreturn		-10058
#ifndef __NR_sigreturn
#define __NR_sigreturn		__PNR_sigreturn
#endif /* __NR_sigreturn */

#define __PNR_sigsuspend	-10059
#ifndef __NR_sigsuspend
#define __NR_sigsuspend		__PNR_sigsuspend
#endif /* __NR_sigsuspend */

#define __PNR_socketcall	-10060
#ifndef __NR_socketcall
#define __NR_socketcall		__PNR_socketcall
#endif /* __NR_socketcall */

#define __PNR_ssetmask		-10061
#ifndef __NR_ssetmask
#define __NR_ssetmask		__PNR_ssetmask
#endif /* __NR_ssetmask */

#define __PNR_stat64		-10062
#ifndef __NR_stat64
#define __NR_stat64		__PNR_stat64
#endif /* __NR_stat64 */

#define __PNR_statfs64		-10063
#ifndef __NR_statfs64
#define __NR_statfs64		__PNR_statfs64
#endif /* __NR_statfs64 */

#define __PNR_stime		-10064
#ifndef __NR_stime
#define __NR_stime		__PNR_stime
#endif /* __NR_stime */

#define __PNR_stty		-10065
#ifndef __NR_stty
#define __NR_stty		__PNR_stty
#endif /* __NR_stty */

#define __PNR_truncate64	-10066
#ifndef __NR_truncate64
#define __NR_truncate64		__PNR_truncate64
#endif /* __NR_truncate64 */

#define __PNR_tuxcall		-10067
#ifndef __NR_tuxcall
#define __NR_tuxcall		__PNR_tuxcall
#endif /* __NR_tuxcall */

#define __PNR_ugetrlimit	-10068
#ifndef __NR_ugetrlimit
#define __NR_ugetrlimit		__PNR_ugetrlimit
#endif /* __NR_ugetrlimit */

#define __PNR_ulimit		-10069
#ifndef __NR_ulimit
#define __NR_ulimit		__PNR_ulimit
#endif /* __NR_ulimit */

#define __PNR_umount		-10070
#ifndef __NR_umount
#define __NR_umount		__PNR_umount
#endif /* __NR_umount */

#define __PNR_vm86		-10071
#ifndef __NR_vm86
#define __NR_vm86		__PNR_vm86
#endif /* __NR_vm86 */

#define __PNR_vm86old		-10072
#ifndef __NR_vm86old
#define __NR_vm86old		__PNR_vm86old
#endif /* __NR_vm86old */

#define __PNR_waitpid		-10073
#ifndef __NR_waitpid
#define __NR_waitpid		__PNR_waitpid
#endif /* __NR_waitpid */

#define __PNR_create_module	-10074
#ifndef __NR_create_module
#define __NR_create_module	__PNR_create_module
#endif /* __NR_create_module */

#define __PNR_get_kernel_syms	-10075
#ifndef __NR_get_kernel_syms
#define __NR_get_kernel_syms	__PNR_get_kernel_syms
#endif /* __NR_get_kernel_syms */

#define __PNR_get_thread_area	-10076
#ifndef __NR_get_thread_area
#define __NR_get_thread_area	__PNR_get_thread_area
#endif /* __NR_get_thread_area */

#define __PNR_nfsservctl	-10077
#ifndef __NR_nfsservctl
#define __NR_nfsservctl		__PNR_nfsservctl
#endif /* __NR_nfsservctl */

#define __PNR_query_module	-10078
#ifndef __NR_query_module
#define __NR_query_module __PNR_query_module
#endif /* __NR_query_module */

#define __PNR_set_thread_area	-10079
#ifndef __NR_set_thread_area
#define __NR_set_thread_area	__PNR_set_thread_area
#endif /* __NR_set_thread_area */

#define __PNR__sysctl		-10080
#ifndef __NR__sysctl
#define __NR__sysctl		__PNR__sysctl
#endif /* __NR__sysctl */

#define __PNR_uselib		-10081
#ifndef __NR_uselib
#define __NR_uselib		__PNR_uselib
#endif /* __NR_uselib */

#define __PNR_vserver		-10082
#ifndef __NR_vserver
#define __NR_vserver		__PNR_vserver
#endif /* __NR_vserver */

#define __PNR_arm_fadvise64_64	-10083
#ifndef __NR_arm_fadvise64_64
#define __NR_arm_fadvise64_64	__PNR_arm_fadvise64_64
#endif /* __NR_arm_fadvise64_64 */

#define __PNR_arm_sync_file_range	-10084
#ifndef __NR_arm_sync_file_range
#define __NR_arm_sync_file_range	__PNR_arm_sync_file_range
#endif /* __NR_arm_sync_file_range */

#define __PNR_finit_module	-10085
#ifndef __NR_finit_module
#define __NR_finit_module	__PNR_finit_module
#endif /* __NR_finit_module */

#define __PNR_pciconfig_iobase	-10086
#ifndef __NR_pciconfig_iobase
#define __NR_pciconfig_iobase	__PNR_pciconfig_iobase
#endif /* __NR_pciconfig_iobase */

#define __PNR_pciconfig_read	-10087
#ifndef __NR_pciconfig_read
#define __NR_pciconfig_read	__PNR_pciconfig_read
#endif /* __NR_pciconfig_read */

#define __PNR_pciconfig_write	-10088
#ifndef __NR_pciconfig_write
#define __NR_pciconfig_write	__PNR_pciconfig_write
#endif /* __NR_pciconfig_write */

#define __PNR_sync_file_range2	-10089
#ifndef __NR_sync_file_range2
#define __NR_sync_file_range2	__PNR_sync_file_range2
#endif /* __NR_sync_file_range2 */

#define __PNR_syscall		-10090
#ifndef __NR_syscall
#define __NR_syscall		__PNR_syscall
#endif /* __NR_syscall */

#define __PNR_afs_syscall	-10091
#ifndef __NR_afs_syscall
#define __NR_afs_syscall	__PNR_afs_syscall
#endif /* __NR_afs_syscall */

#define __PNR_fadvise64		-10092
#ifndef __NR_fadvise64
#define __NR_fadvise64		__PNR_fadvise64
#endif /* __NR_fadvise64 */

#define __PNR_getpmsg		-10093
#ifndef __NR_getpmsg
#define __NR_getpmsg		__PNR_getpmsg
#endif /* __NR_getpmsg */

#define __PNR_ioperm		-10094
#ifndef __NR_ioperm
#define __NR_ioperm		__PNR_ioperm
#endif /* __NR_ioperm */

#define __PNR_iopl		-10095
#ifndef __NR_iopl
#define __NR_iopl		__PNR_iopl
#endif /* __NR_iopl */

#define __PNR_kcmp		-10096
#ifndef __NR_kcmp
#define __NR_kcmp		__PNR_kcmp
#endif /* __NR_kcmp */

#define __PNR_migrate_pages	-10097
#ifndef __NR_migrate_pages
#define __NR_migrate_pages	__PNR_migrate_pages
#endif /* __NR_migrate_pages */

#define __PNR_modify_ldt	-10098
#ifndef __NR_modify_ldt
#define __NR_modify_ldt		__PNR_modify_ldt
#endif /* __NR_modify_ldt */

#define __PNR_putpmsg		-10099
#ifndef __NR_putpmsg
#define __NR_putpmsg		__PNR_putpmsg
#endif /* __NR_putpmsg */

#define __PNR_sync_file_range	-10100
#ifndef __NR_sync_file_range
#define __NR_sync_file_range	__PNR_sync_file_range
#endif /* __NR_sync_file_range */

#ifdef __cplusplus
}
#endif

#endif
