/*
 * Enhanced Direct Memory Access (EDMA3) Controller
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * Author: Ivan Khoronzhuk <ivan.khoronzhuk@ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <asm/io.h>
#include <common.h>
#include <asm/ti-common/ti-edma3.h>

#define EDMA3_SL_BASE(slot)			(0x4000 + ((slot) << 5))
#define EDMA3_SL_MAX_NUM			512
#define EDMA3_SLOPT_FIFO_WIDTH_MASK		(0x7 << 8)

#define EDMA3_QCHMAP(ch)			0x0200 + ((ch) << 2)
#define EDMA3_CHMAP_PARSET_MASK			0x1ff
#define EDMA3_CHMAP_PARSET_SHIFT		0x5
#define EDMA3_CHMAP_TRIGWORD_SHIFT		0x2

#define EDMA3_QEMCR				0x314
#define EDMA3_IPR				0x1068
#define EDMA3_IPRH				0x106c
#define EDMA3_ICR				0x1070
#define EDMA3_ICRH				0x1074
#define EDMA3_QEECR				0x1088
#define EDMA3_QEESR				0x108c
#define EDMA3_QSECR				0x1094

/**
 * qedma3_start - start qdma on a channel
 * @base: base address of edma
 * @cfg: pinter to struct edma3_channel_config where you can set
 * the slot number to associate with, the chnum, which corresponds
 * your quick channel number 0-7, complete code - transfer complete code
 * and trigger slot word - which has to correspond to the word number in
 * edma3_slot_layout struct for generating event.
 *
 */
void qedma3_start(u32 base, struct edma3_channel_config *cfg)
{
	u32 qchmap;

	/* Clear the pending int bit */
	if (cfg->complete_code < 32)
		__raw_writel(1 << cfg->complete_code, base + EDMA3_ICR);
	else
		__raw_writel(1 << cfg->complete_code, base + EDMA3_ICRH);

	/* Map parameter set and trigger word 7 to quick channel */
	qchmap = ((EDMA3_CHMAP_PARSET_MASK & cfg->slot)
		  << EDMA3_CHMAP_PARSET_SHIFT) |
		  (cfg->trigger_slot_word << EDMA3_CHMAP_TRIGWORD_SHIFT);

	__raw_writel(qchmap, base + EDMA3_QCHMAP(cfg->chnum));

	/* Clear missed event if set*/
	__raw_writel(1 << cfg->chnum, base + EDMA3_QSECR);
	__raw_writel(1 << cfg->chnum, base + EDMA3_QEMCR);

	/* Enable qdma channel event */
	__raw_writel(1 << cfg->chnum, base + EDMA3_QEESR);
}

/**
 * edma3_set_dest - set initial DMA destination address in parameter RAM slot
 * @base: base address of edma
 * @slot: parameter RAM slot being configured
 * @dst: physical address of destination (memory, controller FIFO, etc)
 * @addressMode: INCR, except in very rare cases
 * @width: ignored unless @addressMode is FIFO, else specifies the
 *	width to use when addressing the fifo (e.g. W8BIT, W32BIT)
 *
 * Note that the destination address is modified during the DMA transfer
 * according to edma3_set_dest_index().
 */
void edma3_set_dest(u32 base, int slot, u32 dst, enum edma3_address_mode mode,
		    enum edma3_fifo_width width)
{
	u32 opt;
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	opt = __raw_readl(&rg->opt);
	if (mode == FIFO)
		opt = (opt & EDMA3_SLOPT_FIFO_WIDTH_MASK) |
		       (EDMA3_SLOPT_DST_ADDR_CONST_MODE |
			EDMA3_SLOPT_FIFO_WIDTH_SET(width));
	else
		opt &= ~EDMA3_SLOPT_DST_ADDR_CONST_MODE;

	__raw_writel(opt, &rg->opt);
	__raw_writel(dst, &rg->dst);
}

/**
 * edma3_set_dest_index - configure DMA destination address indexing
 * @base: base address of edma
 * @slot: parameter RAM slot being configured
 * @bidx: byte offset between destination arrays in a frame
 * @cidx: byte offset between destination frames in a block
 *
 * Offsets are specified to support either contiguous or discontiguous
 * memory transfers, or repeated access to a hardware register, as needed.
 * When accessing hardware registers, both offsets are normally zero.
 */
void edma3_set_dest_index(u32 base, unsigned slot, int bidx, int cidx)
{
	u32 src_dst_bidx;
	u32 src_dst_cidx;
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	src_dst_bidx = __raw_readl(&rg->src_dst_bidx);
	src_dst_cidx = __raw_readl(&rg->src_dst_cidx);

	__raw_writel((src_dst_bidx & 0x0000ffff) | (bidx << 16),
		     &rg->src_dst_bidx);
	__raw_writel((src_dst_cidx & 0x0000ffff) | (cidx << 16),
		     &rg->src_dst_cidx);
}

/**
 * edma3_set_dest_addr - set destination address for slot only
 */
void edma3_set_dest_addr(u32 base, int slot, u32 dst)
{
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));
	__raw_writel(dst, &rg->dst);
}

/**
 * edma3_set_src - set initial DMA source address in parameter RAM slot
 * @base: base address of edma
 * @slot: parameter RAM slot being configured
 * @src_port: physical address of source (memory, controller FIFO, etc)
 * @mode: INCR, except in very rare cases
 * @width: ignored unless @addressMode is FIFO, else specifies the
 *	width to use when addressing the fifo (e.g. W8BIT, W32BIT)
 *
 * Note that the source address is modified during the DMA transfer
 * according to edma3_set_src_index().
 */
void edma3_set_src(u32 base, int slot, u32 src, enum edma3_address_mode mode,
		   enum edma3_fifo_width width)
{
	u32 opt;
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	opt = __raw_readl(&rg->opt);
	if (mode == FIFO)
		opt = (opt & EDMA3_SLOPT_FIFO_WIDTH_MASK) |
		       (EDMA3_SLOPT_DST_ADDR_CONST_MODE |
			EDMA3_SLOPT_FIFO_WIDTH_SET(width));
	else
		opt &= ~EDMA3_SLOPT_DST_ADDR_CONST_MODE;

	__raw_writel(opt, &rg->opt);
	__raw_writel(src, &rg->src);
}

/**
 * edma3_set_src_index - configure DMA source address indexing
 * @base: base address of edma
 * @slot: parameter RAM slot being configured
 * @bidx: byte offset between source arrays in a frame
 * @cidx: byte offset between source frames in a block
 *
 * Offsets are specified to support either contiguous or discontiguous
 * memory transfers, or repeated access to a hardware register, as needed.
 * When accessing hardware registers, both offsets are normally zero.
 */
void edma3_set_src_index(u32 base, unsigned slot, int bidx, int cidx)
{
	u32 src_dst_bidx;
	u32 src_dst_cidx;
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	src_dst_bidx = __raw_readl(&rg->src_dst_bidx);
	src_dst_cidx = __raw_readl(&rg->src_dst_cidx);

	__raw_writel((src_dst_bidx & 0xffff0000) | bidx,
		     &rg->src_dst_bidx);
	__raw_writel((src_dst_cidx & 0xffff0000) | cidx,
		     &rg->src_dst_cidx);
}

/**
 * edma3_set_src_addr - set source address for slot only
 */
void edma3_set_src_addr(u32 base, int slot, u32 src)
{
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));
	__raw_writel(src, &rg->src);
}

/**
 * edma3_set_transfer_params - configure DMA transfer parameters
 * @base: base address of edma
 * @slot: parameter RAM slot being configured
 * @acnt: how many bytes per array (at least one)
 * @bcnt: how many arrays per frame (at least one)
 * @ccnt: how many frames per block (at least one)
 * @bcnt_rld: used only for A-Synchronized transfers; this specifies
 *	the value to reload into bcnt when it decrements to zero
 * @sync_mode: ASYNC or ABSYNC
 *
 * See the EDMA3 documentation to understand how to configure and link
 * transfers using the fields in PaRAM slots.  If you are not doing it
 * all at once with edma3_write_slot(), you will use this routine
 * plus two calls each for source and destination, setting the initial
 * address and saying how to index that address.
 *
 * An example of an A-Synchronized transfer is a serial link using a
 * single word shift register.  In that case, @acnt would be equal to
 * that word size; the serial controller issues a DMA synchronization
 * event to transfer each word, and memory access by the DMA transfer
 * controller will be word-at-a-time.
 *
 * An example of an AB-Synchronized transfer is a device using a FIFO.
 * In that case, @acnt equals the FIFO width and @bcnt equals its depth.
 * The controller with the FIFO issues DMA synchronization events when
 * the FIFO threshold is reached, and the DMA transfer controller will
 * transfer one frame to (or from) the FIFO.  It will probably use
 * efficient burst modes to access memory.
 */
void edma3_set_transfer_params(u32 base, int slot, int acnt,
			       int bcnt, int ccnt, u16 bcnt_rld,
			       enum edma3_sync_dimension sync_mode)
{
	u32 opt;
	u32 link_bcntrld;
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	link_bcntrld = __raw_readl(&rg->link_bcntrld);

	__raw_writel((bcnt_rld << 16) | (0x0000ffff & link_bcntrld),
		     &rg->link_bcntrld);

	opt = __raw_readl(&rg->opt);
	if (sync_mode == ASYNC)
		__raw_writel(opt & ~EDMA3_SLOPT_AB_SYNC, &rg->opt);
	else
		__raw_writel(opt | EDMA3_SLOPT_AB_SYNC, &rg->opt);

	/* Set the acount, bcount, ccount registers */
	__raw_writel((bcnt << 16) | (acnt & 0xffff), &rg->a_b_cnt);
	__raw_writel(0xffff & ccnt, &rg->ccnt);
}

/**
 * edma3_write_slot - write parameter RAM data for slot
 * @base: base address of edma
 * @slot: number of parameter RAM slot being modified
 * @param: data to be written into parameter RAM slot
 *
 * Use this to assign all parameters of a transfer at once.  This
 * allows more efficient setup of transfers than issuing multiple
 * calls to set up those parameters in small pieces, and provides
 * complete control over all transfer options.
 */
void edma3_write_slot(u32 base, int slot, struct edma3_slot_layout *param)
{
	int i;
	u32 *p = (u32 *)param;
	u32 *addr = (u32 *)(base + EDMA3_SL_BASE(slot));

	for (i = 0; i < sizeof(struct edma3_slot_layout)/4; i += 4)
		__raw_writel(*p++, addr++);
}

/**
 * edma3_read_slot - read parameter RAM data from slot
 * @base: base address of edma
 * @slot: number of parameter RAM slot being copied
 * @param: where to store copy of parameter RAM data
 *
 * Use this to read data from a parameter RAM slot, perhaps to
 * save them as a template for later reuse.
 */
void edma3_read_slot(u32 base, int slot, struct edma3_slot_layout *param)
{
	int i;
	u32 *p = (u32 *)param;
	u32 *addr = (u32 *)(base + EDMA3_SL_BASE(slot));

	for (i = 0; i < sizeof(struct edma3_slot_layout)/4; i += 4)
		*p++ = __raw_readl(addr++);
}

void edma3_slot_configure(u32 base, int slot, struct edma3_slot_config *cfg)
{
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	__raw_writel(cfg->opt, &rg->opt);
	__raw_writel(cfg->src, &rg->src);
	__raw_writel((cfg->bcnt << 16) | (cfg->acnt & 0xffff), &rg->a_b_cnt);
	__raw_writel(cfg->dst, &rg->dst);
	__raw_writel((cfg->dst_bidx << 16) |
		     (cfg->src_bidx & 0xffff), &rg->src_dst_bidx);
	__raw_writel((cfg->bcntrld << 16) |
		     (cfg->link & 0xffff), &rg->link_bcntrld);
	__raw_writel((cfg->dst_cidx << 16) |
		     (cfg->src_cidx & 0xffff), &rg->src_dst_cidx);
	__raw_writel(0xffff & cfg->ccnt, &rg->ccnt);
}

/**
 * edma3_check_for_transfer - check if transfer coplete by checking
 * interrupt pending bit. Clear interrupt pending bit if complete.
 * @base: base address of edma
 * @cfg: pinter to struct edma3_channel_config which was passed
 * to qedma3_start when you started qdma channel
 *
 * Return 0 if complete, 1 if not.
 */
int edma3_check_for_transfer(u32 base, struct edma3_channel_config *cfg)
{
	u32 inum;
	u32 ipr_base;
	u32 icr_base;

	if (cfg->complete_code < 32) {
		ipr_base = base + EDMA3_IPR;
		icr_base = base + EDMA3_ICR;
		inum = 1 << cfg->complete_code;
	} else {
		ipr_base = base + EDMA3_IPRH;
		icr_base = base + EDMA3_ICRH;
		inum = 1 << (cfg->complete_code - 32);
	}

	/* check complete interrupt */
	if (!(__raw_readl(ipr_base) & inum))
		return 1;

	/* clean up the pending int bit */
	__raw_writel(inum, icr_base);

	return 0;
}

/**
 * qedma3_stop - stops dma on the channel passed
 * @base: base address of edma
 * @cfg: pinter to struct edma3_channel_config which was passed
 * to qedma3_start when you started qdma channel
 */
void qedma3_stop(u32 base, struct edma3_channel_config *cfg)
{
	/* Disable qdma channel event */
	__raw_writel(1 << cfg->chnum, base + EDMA3_QEECR);

	/* clean up the interrupt indication */
	if (cfg->complete_code < 32)
		__raw_writel(1 << cfg->complete_code, base + EDMA3_ICR);
	else
		__raw_writel(1 << cfg->complete_code, base + EDMA3_ICRH);

	/* Clear missed event if set*/
	__raw_writel(1 << cfg->chnum, base + EDMA3_QSECR);
	__raw_writel(1 << cfg->chnum, base + EDMA3_QEMCR);

	/* Clear the channel map */
	__raw_writel(0, base + EDMA3_QCHMAP(cfg->chnum));
}
