/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2020-2023 Nokia
 */

#ifndef ODP_CHKSUM_INTERNAL_H_
#define ODP_CHKSUM_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <odp/api/hints.h>
#include <odp/api/byteorder.h>
#include <odp_cpu.h>
#include <stdint.h>
#include <string.h>

/*
 * Compute the final Internet checksum (RFC 1071) based on a partial
 * sum. A partial sum can be obtained e.g. by calling
 * chksum_partial().
 */
static inline uint16_t chksum_finalize(uint64_t sum)
{
	sum = (sum >> 32) + (sum & 0xffffffff);
	sum = (sum >> 16) + (sum & 0xffff);
	/*
	 * The final & 0xffff is intentionally omitted, the extra bits
	 * are discarded by the implicit cast to the return type.
	 */
	return (sum >> 16) + sum;
}


static inline uint16_t read_u16(const uint8_t *addr)
{
	uint16_t val;
	memcpy(&val, addr, sizeof(val));
	return val;
}

static inline uint32_t read_u32(const uint8_t *addr)
{
	uint32_t val;
	memcpy(&val, addr, sizeof(val));
	return val;
}

/*
 * Compute a partial checksum. Several partial checksums may be summed
 * together. The final checksum may be obtained by calling
 * chksum_finalize(). Parameter offset is the offset of this segment
 * of data from the start of IP header.
 *
 * This implementation
 *
 * - Accepts unaligned data.
 *
 * - Accepts data at any byte offset from the start of IP header,
 *   including odd offsets.
 *
 * - Uses unaligned memory access only if available.
 *
 * - Is optimized (for skylake, cn96, a53) by trial and error.
 *
 * The following did not improve performance (in synthetic tests):
 *
 * - 2 or 4 sub-sums in the main loop (to break dependency chains).
 *
 * - Aligning to 8 bytes instead of 4 (for ldp instruction). This
 *   makes the main loop faster on a53 (only), but the extra
 *   conditional branch has its cost.
 *
 * - __builtin_assume_aligned().
 */
static uint64_t chksum_partial(const void *addr, uint32_t len, uint32_t offset)
{
	const uint8_t *b;
	uint64_t sum = 0;

	/*
	 * Offset is either even or odd, the rest of it doesn't
	 * matter.
	 */
	offset &= 1;

	if (_ODP_UNALIGNED) {
		/*
		 * We have efficient unaligned access. Just read
		 * dwords starting at the given address.
		 */
		b = addr;
	} else {
		/*
		 * We must avoid unaligned access, so align to 4 bytes
		 * by summing up the first up to 3 bytes.
		 */
		b = addr;

		if (odp_unlikely((uintptr_t)b & 1) && len >= 1) {
			/*
			 * Align to 2 bytes by handling an odd
			 * byte. Since addr is unaligned, the first
			 * byte goes into the second byte of the sum.
			 */
			sum += odp_cpu_to_be_16(*b++);
			len -= 1;

			/* An odd byte negates the effect of offset. */
			offset ^= 1;
		}

		if ((uintptr_t)b & 2 && len >= 2) {
			/* Align bytes by handling an odd word. */

			sum += read_u16(b);
			b += 2;
			len -= 2;
		}
	}

	while (len >= 32)  {
		/* 8 dwords or 32 bytes per round. */

		sum += read_u32(b); b += 4;
		sum += read_u32(b); b += 4;
		sum += read_u32(b); b += 4;
		sum += read_u32(b); b += 4;

		sum += read_u32(b); b += 4;
		sum += read_u32(b); b += 4;
		sum += read_u32(b); b += 4;
		sum += read_u32(b); b += 4;

		len -= 32;
	}

	/* Last up to 7 dwords. */
	switch (len >> 2) {
	case 7:
		sum += read_u32(b); b += 4;
		/* FALLTHROUGH */
	case 6:
		sum += read_u32(b); b += 4;
		/* FALLTHROUGH */
	case 5:
		sum += read_u32(b); b += 4;
		/* FALLTHROUGH */
	case 4:
		sum += read_u32(b); b += 4;
		/* FALLTHROUGH */
	case 3:
		sum += read_u32(b); b += 4;
		/* FALLTHROUGH */
	case 2:
		sum += read_u32(b); b += 4;
		/* FALLTHROUGH */
	case 1:
		sum += read_u32(b); b += 4;
		/* FALLTHROUGH */
	default:
		break;
	}

	len &= 3;

	if (len > 1)  {
		/* Last word. */
		sum += read_u16(b);
		b += 2;
		len -= 2;
	}

	if (len) {
		/* Last byte. */
		sum += odp_cpu_to_be_16((uint16_t)*b << 8);
	}

	/*
	 * If offset is odd, our sum is byte-flipped and we need to
	 * flip odd and even bytes.
	 */
	if (odp_unlikely(offset))
		sum = ((sum & 0xff00ff00ff00ff) << 8) | ((sum & 0xff00ff00ff00ff00) >> 8);

	return sum;
}

#ifdef __cplusplus
}
#endif

#endif
