/* SPDX-License-Identifier: GPL-2.0-only OR MIT */
/*
 * Apple RTKit IPC library
 * Copyright (C) The Asahi Linux Contributors
 */

#ifndef _APPLE_RTKIT_INTERAL_H
#define _APPLE_RTKIT_INTERAL_H

#include <linux/bitfield.h>
#include <linux/bitmap.h>
#include <linux/completion.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/soc/apple/mailbox.h>
#include <linux/soc/apple/rtkit.h>
#include <linux/workqueue.h>

/* Shared with rtkit-crashlog.c: the crashlog header magic. Moved here
 * so the crash-capture validation in rtkit.c uses the same constant
 * as the existing dump (previously file-local to rtkit-crashlog.c). */
#define FOURCC(a, b, c, d) \
	(((u32)(a) << 24) | ((u32)(b) << 16) | ((u32)(c) << 8) | ((u32)(d)))

#define APPLE_RTKIT_CRASHLOG_HEADER FOURCC('C', 'L', 'H', 'E')

#define APPLE_RTKIT_APP_ENDPOINT_START 0x20
#define APPLE_RTKIT_MAX_ENDPOINTS 0x100

struct apple_rtkit {
	void *cookie;
	const struct apple_rtkit_ops *ops;
	struct device *dev;

	struct apple_mbox *mbox;

	struct completion epmap_completion;
	struct completion iop_pwr_ack_completion;
	struct completion ap_pwr_ack_completion;

	int boot_result;
	int version;

	unsigned int iop_power_state;
	unsigned int ap_power_state;
	bool crashed;

	/* DONOR-INTEGRATION P1 (Aurora inherited flags, adapted):
	 * true while this endpoint's buffer/state still belongs to the
	 * producer session. Cleared when a fresh (non-preset) buffer is
	 * installed for that endpoint, or by reinit (fresh session). */
	bool crashlog_inherited;
	bool syslog_inherited;
	bool oslog_inherited;

	DECLARE_BITMAP(endpoints, APPLE_RTKIT_MAX_ENDPOINTS);

	struct apple_rtkit_shmem ioreport_buffer;
	struct apple_rtkit_shmem system7_buffer;
	struct apple_rtkit_shmem crashlog_buffer;
	struct apple_rtkit_shmem oslog_buffer;

	struct apple_rtkit_shmem syslog_buffer;
	char *syslog_msg_buffer;
	size_t syslog_n_entries;
	size_t syslog_msg_size;

	struct workqueue_struct *wq;

	/* TIER-A (print-only diagnostic, message-free): RX counters.
	 * Counters are the only state change; they change no value,
	 * timeout, path, or register write. */
	unsigned long rx_count_mgmt;
	unsigned long rx_count_crashlog;
	unsigned long rx_count_syslog;
	unsigned long rx_count_debug;
	unsigned long rx_count_ioreport;
	unsigned long rx_count_unknown7;
	unsigned long rx_count_oslog;
	unsigned long rx_count_tracekit;
	unsigned long rx_count_app;
	unsigned long rx_count_other;
	unsigned long rx_alloc_fail;
	atomic_t rx_queued;

	/* CRASH-CAPTURE (print-only diagnostic): last endpoint-1
	 * message record, for the C13 watch-point print. Written on
	 * every endpoint-1 dispatch; read-only elsewhere. */
	struct apple_rtkit_crashlog_last last_crashlog;
};

void apple_rtkit_crashlog_dump(struct apple_rtkit *rtk, u8 *bfr, size_t size);

#endif
