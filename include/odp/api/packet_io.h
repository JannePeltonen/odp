/* Copyright (c) 2013, Linaro Limited
 * All rights reserved.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 */


/**
 * @file
 *
 * ODP Packet IO
 */

#ifndef ODP_API_PACKET_IO_H_
#define ODP_API_PACKET_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <odp/api/packet_io_stats.h>
#include <odp/api/queue.h>

/** @defgroup odp_packet_io ODP PACKET IO
 *  Operations on a packet Input/Output interface.
 *
 * Packet IO is the Ingress and Egress interface to ODP processing. It
 * allows manipulation of the interface for setting such attributes as
 * the mtu, mac etc.
 * Pktio is usually followed by the classifier and a default class COS
 * can be set so that the scheduler may distribute flows. The interface
 * may be used directly in polled mode with odp_pktio_recv() &
 * odp_pktio_send().
 * Diagnostic messages can be enhanced by using odp_pktio_to_u64 which
 * will generate a printable reference for a pktio handle for use with
 * the logging.
 *  @{
 */

/**
 * @typedef odp_pktio_t
 * Packet IO handle
 */

/**
 * @typedef odp_pktin_queue_t
 * Direct packet input queue handle
 */

/**
 * @typedef odp_pktout_queue_t
 * Direct packet output queue handle
 */

/**
 * @def ODP_PKTIO_INVALID
 * Invalid packet IO handle
 */

/**
 * @def ODP_PKTIO_ANY
 * odp_pktio_t value to indicate any port
 */

/**
 * @def ODP_PKTIO_MACADDR_MAXSIZE
 * Minimum size of output buffer for odp_pktio_mac_addr()
 * Actual MAC address sizes may be different.
 */


/**
 * Packet input mode
 */
typedef enum odp_pktio_input_mode_t {
	/** Application polls packet input directly with odp_pktio_recv() */
	ODP_PKTIN_MODE_RECV = 0,
	/** Packet input through scheduled queues */
	ODP_PKTIN_MODE_SCHED,
	/** Application polls packet input queues */
	ODP_PKTIN_MODE_POLL,
	/** Application will never receive from this interface */
	ODP_PKTIN_MODE_DISABLED
} odp_pktio_input_mode_t;

/**
 * Packet output mode
 */
typedef enum odp_pktio_output_mode_t {
	/** Direct packet output on the interface with odp_pktio_send() */
	ODP_PKTOUT_MODE_SEND = 0,
	/** Packet output through traffic manager API */
	ODP_PKTOUT_MODE_TM,
	/** Application will never send to this interface */
	ODP_PKTOUT_MODE_DISABLED
} odp_pktio_output_mode_t;

/**
 * Packet input hash protocols
 *
 * The list of protocol header field combinations, which are included into
 * packet input hash calculation.
 */
typedef union odp_pktin_hash_proto_t {
	/** Protocol header fields for hashing */
	struct {
		/** IPv4 addresses and UDP port numbers */
		uint32_t ipv4_udp : 1;
		/** IPv4 addresses and TCP port numbers */
		uint32_t ipv4_tcp : 1;
		/** IPv4 addresses */
		uint32_t ipv4     : 1;
		/** IPv6 addresses and UDP port numbers */
		uint32_t ipv6_udp : 1;
		/** IPv6 addresses and TCP port numbers */
		uint32_t ipv6_tcp : 1;
		/** IPv6 addresses */
		uint32_t ipv6     : 1;
	} proto;

	/** All bits of the bit field structure */
	uint32_t all_bits;
} odp_pktin_hash_proto_t;

/**
 * Packet input queue parameters
 */
typedef struct odp_pktio_input_queue_param_t {
	/** Single thread per queue. Enable performance optimization when each
	  * queue has only single user.
	  * 0: Queue is multi-thread safe
	  * 1: Queue is used by single thread only */
	odp_bool_t single_user;

	/** Enable flow hashing
	  * 0: Do not hash flows
	  * 1: Hash flows to input queues */
	odp_bool_t hash_enable;

	/** Protocol field selection for hashing. Multiple protocols can be
	  * selected. */
	odp_pktin_hash_proto_t hash_proto;

	/** Number of input queues to be created. More than one input queue
	  * require input hashing or classifier setup. Hash_proto is ignored
	  * when hash_enable is zero or num_queues is one. This value must be
	  * between 1 and interface capability. Queue type is defined by the
	  * input mode. */
	unsigned num_queues;

	/** Queue parameters for creating input queues in ODP_PKTIN_MODE_POLL
	  * or ODP_PKTIN_MODE_SCHED modes. Scheduler parameters are considered
	  * only in ODP_PKTIN_MODE_SCHED mode. */
	odp_queue_param_t queue_param;

} odp_pktio_input_queue_param_t;

/**
 * Packet output queue parameters
 *
 * These parameters are used only in ODP_PKTOUT_MODE_SEND mode.
 */
typedef struct odp_pktio_output_queue_param_t {
	/** Single thread per queue. Enable performance optimization when each
	  * queue has only single user.
	  * 0: Queue is multi-thread safe
	  * 1: Queue is used by single thread only */
	odp_bool_t single_user;

	/** Number of output queues to be created. The value must be between
	  * 1 and interface capability */
	unsigned num_queues;

} odp_pktio_output_queue_param_t;

/**
 * Packet IO parameters
 *
 * In minimum, user must select input and output modes. Use 0 for defaults.
 * Initialize entire struct with zero to maintain API compatibility.
 */
typedef struct odp_pktio_param_t {
	/** Packet input mode */
	odp_pktio_input_mode_t in_mode;
	/** Packet output mode */
	odp_pktio_output_mode_t out_mode;
} odp_pktio_param_t;

/**
 * Packet IO capabilities
 */
typedef struct odp_pktio_capability_t {
	/** Maximum number of input queues */
	unsigned max_input_queues;
	/** Maximum number of output queues */
	unsigned max_output_queues;
} odp_pktio_capability_t;

/**
 * Open a packet IO interface
 *
 * An ODP program can open a single packet IO interface per device, attempts
 * to open an already open device will fail, returning ODP_PKTIO_INVALID with
 * errno set. Use odp_pktio_lookup() to obtain a handle to an already open
 * device. Packet IO parameters provide interface level configuration options.
 *
 * This call does not activate packet receive and transmit on the interface.
 * The interface is activated with a call to odp_pktio_start(). If not
 * specified otherwise, any interface level configuration must not be changed
 * when the interface is active (between start and stop calls).
 *
 * @param dev    Packet IO device name
 * @param pool   Default pool from which to allocate storage for packets
 *               received over this interface, must be of type ODP_POOL_PACKET
 * @param param  Packet IO parameters
 *
 * @return Packet IO handle
 * @retval ODP_PKTIO_INVALID on failure
 *
 * @note The device name "loop" is a reserved name for a loopback device used
 *	 for testing purposes.
 *
 * @note Packets arriving via this interface assigned to a CoS by the
 *	 classifier are received into the pool associated with that CoS. This
 *	 will occur either because this pktio is assigned a default CoS via
 *	 the odp_pktio_default_cos_set() routine, or because a matching PMR
 *	 assigned the packet to a specific CoS. The default pool specified
 *	 here is applicable only for those packets that are not assigned to a
 *	 more specific CoS.
 *
 * @see odp_pktio_start(), odp_pktio_stop(), odp_pktio_close()
 */
odp_pktio_t odp_pktio_open(const char *dev, odp_pool_t pool,
			   const odp_pktio_param_t *param);

/**
 * Query packet IO interface capabilities
 *
 * Outputs packet IO interface capabilities on success.
 *
 * @param      pktio  Packet IO handle
 * @param[out] capa   Pointer to capability structure for output
 *
 * @retval 0 on success
 * @retval <0 on failure
 */
int odp_pktio_capability(odp_pktio_t pktio, odp_pktio_capability_t *capa);

/**
 * Configure packet input queues
 *
 * Setup a number of packet input queues and configure those. The maximum number
 * of queues is platform dependent and can be queried with
 * odp_pktio_capability(). Queue handles for input queues can be requested with
 * odp_pktio_in_queues() or odp_pktio_pktin_queues() after this call. All
 * requested queues are setup on success, no queues are setup on failure.
 *
 * @param pktio    Packet IO handle
 * @param param    Packet input queue configuration parameters
 *
 * @retval 0 on success
 * @retval <0 on failure
 *
 * @see odp_pktio_capability(), odp_pktio_in_queues(), odp_pktio_pktin_queues()
 */
int odp_pktio_input_queues_config(odp_pktio_t pktio,
				  const odp_pktio_input_queue_param_t *param);

/**
 * Configure packet output queues
 *
 * Setup a number of packet output queues and configure those. The maximum
 * number of queues is platform dependent and can be queried with
 * odp_pktio_capability(). All requested queues are setup on success, no
 * queues are setup on failure.
 *
 * @param pktio    Packet IO handle
 * @param param    Packet output queue configuration parameters
 *
 * @retval 0 on success
 * @retval <0 on failure
 *
 * @see odp_pktio_capability(), odp_pktio_pktout_queues()
 */
int odp_pktio_output_queues_config(odp_pktio_t pktio,
				   const odp_pktio_output_queue_param_t *param);

/**
 * Queues for packet input
 *
 * Returns the number of input queues configured for the interface in
 * ODP_PKTIN_MODE_POLL and ODP_PKTIN_MODE_SCHED modes. Outputs up to 'num' queue
 * handles when the 'queues' array pointer is not NULL. If return value is
 * larger than 'num', there are more queues than the function was allowed to
 * output.
 *
 * Packets (and other events) from these queues are received with
 * odp_queue_deq(), odp_schedule(), etc calls.
 *
 * @param      pktio    Packet IO handle
 * @param[out] queues   Points to an array of queue handles for output
 * @param      num      Maximum number of queue handles to output
 *
 * @return Number of packet input queues
 * @retval <0 on failure
 */
int odp_pktio_in_queues(odp_pktio_t pktio, odp_queue_t queues[], int num);

/**
 * Direct packet input queues
 *
 * Returns the number of input queues configured for the interface in
 * ODP_PKTIN_MODE_RECV mode. Outputs up to 'num' queue handles when the
 * 'queues' array pointer is not NULL. If return value is larger than 'num',
 * there are more queues than the function was allowed to output.
 *
 * Packets from these queues are received with odp_pktio_recv_queue().
 *
 * @param      pktio    Packet IO handle
 * @param[out] queues   Points to an array of queue handles for output
 * @param      num      Maximum number of queue handles to output
 *
 * @return Number of packet input queues
 * @retval <0 on failure
 */
int odp_pktio_pktin_queues(odp_pktio_t pktio, odp_pktin_queue_t queues[],
			   int num);

/**
 * Direct packet output queues
 *
 * Returns the number of output queues configured for the interface in
 * ODP_PKTOUT_MODE_SEND mode. Outputs up to 'num' queue handles when the
 * 'queues' array pointer is not NULL. If return value is larger than 'num',
 * there are more queues than the function was allowed to output.
 *
 * Packets are sent to these queues with odp_pktio_send_queue().
 *
 * @param      pktio    Packet IO handle
 * @param[out] queues   Points to an array of queue handles for output
 * @param      num      Maximum number of queue handles to output
 *
 * @return Number of packet output queues
 * @retval <0 on failure
 */
int odp_pktio_pktout_queues(odp_pktio_t pktio, odp_pktout_queue_t queues[],
			    int num);

/**
 * Start packet receive and transmit
 *
 * Activate packet receive and transmit on a previously opened or stopped
 * interface. The interface can be stopped with a call to odp_pktio_stop().
 *
 * @param pktio  Packet IO handle
 *
 * @retval 0 on success
 * @retval <0 on failure
 *
 * @see odp_pktio_open(), odp_pktio_stop()
 */
int odp_pktio_start(odp_pktio_t pktio);

/**
 * Stop packet receive and transmit
 *
 * Stop packet receive and transmit on a previously started interface. New
 * packets are not received from or transmitted to the network. Packets already
 * received from the network may be still available from interface and
 * application can receive those normally. New packets may not be accepted for
 * transmit. Packets already stored for transmit are not freed. A following
 * odp_packet_start() call restarts packet receive and transmit.
 *
 * @param pktio  Packet IO handle
 *
 * @retval 0 on success
 * @retval <0 on failure
 *
 * @see odp_pktio_start(), odp_pktio_close()
 */
int odp_pktio_stop(odp_pktio_t pktio);

/**
 * Close a packet IO interface
 *
 * Close a stopped packet IO interface. This call frees all remaining packets
 * stored in pktio receive and transmit side buffers. The pktio is destroyed
 * and the handle must not be used for other calls. After a successful call,
 * the same pktio device can be opened again with a odp_packet_open() call.
 *
 * @param pktio  Packet IO handle
 *
 * @retval 0 on success
 * @retval <0 on failure
 *
 * @see odp_pktio_stop(), odp_pktio_open()
 */
int odp_pktio_close(odp_pktio_t pktio);

/**
 * Return a packet IO handle for an already open device
 *
 * @param dev Packet IO device name
 *
 * @return Packet IO handle
 * @retval ODP_PKTIO_INVALID on failure
 */
odp_pktio_t odp_pktio_lookup(const char *dev);

/**
 * Receive packets directly from an interface
 *
 * Receives up to 'num' packets from the interface. The operation is
 * multi-thread safe.
 *
 * @param      pktio      Packet IO handle
 * @param[out] packets[]  Packet handle array for output of received packets
 * @param      num        Maximum number of packets to receive
 *
 * @return Number of packets received
 * @retval <0 on failure
 */
int odp_pktio_recv(odp_pktio_t pktio, odp_packet_t packets[], int num);

/**
 * Receive packets directly from an interface input queue
 *
 * Receives up to 'num' packets from the pktio interface input queue. When
 * 'single_user' input queue parameter has been set, the operation is optimized
 * for single thread per queue usage model and the same queue must not be
 * accessed from multiple threads.
 *
 * @param      queue      Pktio input queue handle for receiving packets
 * @param[out] packets[]  Packet handle array for output of received packets
 * @param      num        Maximum number of packets to receive
 *
 * @return Number of packets received
 * @retval <0 on failure
 *
 * @see odp_pktio_pktin_queues()
 */
int odp_pktio_recv_queue(odp_pktin_queue_t queue, odp_packet_t packets[],
			 int num);

/**
 * Send packets
 *
 * Sends out a number of packets. A successful call returns the actual number of
 * packets sent. If return value is less than 'len', the remaining packets at
 * the end of pkt_table[] are not consumed, and the caller has to take care of
 * them.
 *
 * @param pktio        Packet IO handle
 * @param pkt_table[]  Array of packets to send
 * @param len          length of pkt_table[]
 *
 * @return Number of packets sent
 * @retval <0 on failure
 */
int odp_pktio_send(odp_pktio_t pktio, odp_packet_t pkt_table[], int len);

/**
 * Set the default input queue to be associated with a pktio handle
 *
 * @param pktio		Packet IO handle
 * @param queue		default input queue set
 * @retval  0 on success
 * @retval <0 on failure
 */
int odp_pktio_inq_setdef(odp_pktio_t pktio, odp_queue_t queue);

/**
 * Get default input queue associated with a pktio handle
 *
 * @param pktio  Packet IO handle
 *
 * @return Default input queue set
 * @retval ODP_QUEUE_INVALID on failure
 */
odp_queue_t odp_pktio_inq_getdef(odp_pktio_t pktio);

/**
 * Remove default input queue (if set)
 *
 * @param pktio  Packet IO handle
 *
 * @retval 0 on success
 * @retval <0 on failure
 */
int odp_pktio_inq_remdef(odp_pktio_t pktio);

/**
 * Query default output queue
 *
 * @param pktio Packet IO handle
 *
 * @return Default out queue
 * @retval ODP_QUEUE_INVALID on failure
 */
odp_queue_t odp_pktio_outq_getdef(odp_pktio_t pktio);

/**
 * Return the currently configured MTU value of a packet IO interface.
 *
 * @param[in] pktio  Packet IO handle.
 *
 * @return MTU value on success
 * @retval <0 on failure
 */
int odp_pktio_mtu(odp_pktio_t pktio);

/**
 * Enable/Disable promiscuous mode on a packet IO interface.
 *
 * @param[in] pktio	Packet IO handle.
 * @param[in] enable	1 to enable, 0 to disable.
 *
 * @retval 0 on success
 * @retval <0 on failure
 */
int odp_pktio_promisc_mode_set(odp_pktio_t pktio, odp_bool_t enable);

/**
 * Determine if promiscuous mode is enabled for a packet IO interface.
 *
 * @param[in] pktio Packet IO handle.
 *
 * @retval  1 if promiscuous mode is enabled.
 * @retval  0 if promiscuous mode is disabled.
 * @retval <0 on failure
*/
int odp_pktio_promisc_mode(odp_pktio_t pktio);

/**
 * Get the default MAC address of a packet IO interface.
 *
 * @param	pktio     Packet IO handle
 * @param[out]	mac_addr  Output buffer (use ODP_PKTIO_MACADDR_MAXSIZE)
 * @param       size      Size of output buffer
 *
 * @return Number of bytes written (actual size of MAC address)
 * @retval <0 on failure
 */
int odp_pktio_mac_addr(odp_pktio_t pktio, void *mac_addr, int size);

/**
 * Setup per-port default class-of-service.
 *
 * @param[in]	pktio		Ingress port pktio handle.
 * @param[in]	default_cos	Class-of-service set to all packets arriving
 *				at this ingress port,
 *				unless overridden by subsequent
 *				header-based filters.
 *
 * @retval			0 on success
 * @retval			<0 on failure
 */
int odp_pktio_default_cos_set(odp_pktio_t pktio, odp_cos_t default_cos);

/**
 * Setup per-port error class-of-service
 *
 * @param[in]	pktio		Ingress port pktio handle.
 * @param[in]	error_cos	class-of-service set to all packets arriving
 *				at this ingress port that contain an error.
 *
 * @retval			0 on success
 * @retval			<0 on failure
 *
 * @note Optional.
 */
int odp_pktio_error_cos_set(odp_pktio_t pktio, odp_cos_t error_cos);

/**
 * Setup per-port header offset
 *
 * @param[in]	pktio		Ingress port pktio handle.
 * @param[in]	offset		Number of bytes the classifier must skip.
 *
 * @retval			0 on success
 * @retval			<0 on failure
 * @note  Optional.
 *
 */
int odp_pktio_skip_set(odp_pktio_t pktio, uint32_t offset);

/**
 * Specify per-port buffer headroom
 *
 * @param[in]	pktio		Ingress port pktio handle.
 * @param[in]	headroom	Number of bytes of space preceding
 *				packet data to reserve for use as headroom.
 *				Must not exceed the implementation
 *				defined ODP_PACKET_MAX_HEADROOM.
 *
 * @retval			0 on success
 * @retval			<0 on failure
 *
 * @note Optional.
 */
int odp_pktio_headroom_set(odp_pktio_t pktio, uint32_t headroom);

/**
 * Get printable value for an odp_pktio_t
 *
 * @param pktio   odp_pktio_t handle to be printed
 * @return     uint64_t value that can be used to print/display this
 *             handle
 *
 * @note This routine is intended to be used for diagnostic purposes
 * to enable applications to generate a printable value that represents
 * an odp_pktio_t handle.
 */
uint64_t odp_pktio_to_u64(odp_pktio_t pktio);

/**
 * Intiailize pktio params
 *
 * Initialize an odp_pktio_param_t to its default values for all fields
 *
 * @param param Address of the odp_pktio_param_t to be initialized
 */
void odp_pktio_param_init(odp_pktio_param_t *param);

/**
 * Initialize packet input queue parameters
 *
 * Initialize an odp_pktio_input_queue_param_t to its default values.
 *
 * @param param   Input queue parameter structure to be initialized
 */
void odp_pktio_input_queue_param_init(odp_pktio_input_queue_param_t *param);

/**
 * Initialize packet output queue parameters
 *
 * Initialize an odp_pktio_output_queue_param_t to its default values.
 *
 * @param param   Output queue parameter structure to be initialized
 */
void odp_pktio_output_queue_param_init(odp_pktio_output_queue_param_t *param);

/**
 * Print pktio info to the console
 *
 * Print implementation-defined pktio debug information to the console.
 *
 * @param pktio	                Packet IO handle
 */
void odp_pktio_print(odp_pktio_t pktio);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
