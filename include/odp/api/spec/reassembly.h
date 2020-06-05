/* SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file
 *
 * ODP REASSEMBLY API
 */

#ifndef ODP_API_SPEC_REASSEMBLY_H_
#define ODP_API_SPEC_REASSEMBLY_H_
#include <odp/visibility_begin.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup odp_reassembly ODP REASSEMBLY
 *  Reassembly
 *  @{
 */

/**
 * Fragmentation reassembly layer configuration
 *
 */
typedef struct odp_reassembly_layer_config_t {
	/** Attempt inline reassembly of IPv4 packets. Disabled by default */
	odp_bool_t en_ipv4;

	/** Attempt inline reassembly of IPv6 packets. Disabled by default */
	odp_bool_t en_ipv6;

	/** Maximum wait time in ns, the fragment would wait in the reassembly
	 *  offload for the arrival of further fragments.
	 *
	 *  Default value is 0.
	 */
	uint64_t max_wait_time;

} odp_reassembly_layer_config_t;

/**
 * Reassembly capabilities
 *
 */
typedef struct odp_reassembly_capability_t {
	/** Reassembly offload for all IP packets */
	odp_bool_t ip;

	/** Reassembly offload for IPv4 packets only. Ignored if 'ip' is true */
	odp_bool_t ipv4;

	/** Reassembly offload for IPv6 packets only. Ignored if 'ip' is true */
	odp_bool_t ipv6;

	/** Maximum wait time in ns, the fragment would wait in the reassembly
	 *  offload for the arrival of further fragments.
	 */
	uint64_t max_wait_time;

	/** Maximum number of fragments that can be reassembled */
	uint16_t max_num_frags;

} odp_reassembly_capability_t;

/**
 * Fragment reassembly configuration
 *
 * Configure inline fragment reassembly offload support. Fragment
 * reassembly offload can be enabled in IPSEC and PKTIN operations.
 *
 * When the offload is enabled, the fragments will be delayed for a specified
 * time period to allow reassembly.
 *
 * In case of complete reassembly, the reassembled packet would be identical to
 * a regular ODP packet thereby eliminating the need to have reassembly done in
 * the application.
 *
 * For fragments which failed to complete reassembly, the partial reassembly
 * result must be checked to obtain fragments from the reassembly attempt.
 *
 * When reassembly is enabled, application must check the reassembly status
 * before performing any other packet operation.
 *
 */
typedef struct odp_reassembly_config_t {
	/** IP reassembly */
	odp_reassembly_layer_config_t ip;
} odp_reassembly_config_t;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#include <odp/visibility_end.h>
#endif
