/*
 * (C) Copyright IBM Corporation 2006
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file common_bridge.c
 * Support routines used to process PCI header information for bridges.
 * 
 * \author Ian Romanick <idr@us.ibm.com>
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(HAVE_STRING_H)
# include <string.h>
#elif defined(HAVE_STRINGS_H)
# include <strings.h>
#endif

#if defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#elif defined(HAVE_STDINT_H)
# include <stdint.h>
#endif

#include "pciaccess.h"
#include "pciaccess_private.h"

static int
read_bridge_info( struct pci_device_private * priv )
{
    uint8_t  buf[0x40];
    pciaddr_t bytes;


    switch ( priv->header_type & 0x7f ) {
    case 0x00:
	break;

    case 0x01: {
	struct pci_bridge_info *info;

	info = malloc(sizeof(*info));
	if (info != NULL) {
	    pci_device_cfg_read( (struct pci_device *) priv, buf + 0x18, 0x18, 
				 0x40 - 0x18, & bytes );

	    info->primary_bus = buf[0x18];
	    info->secondary_bus = buf[0x19];
	    info->subordinate_bus = buf[0x1a];
	    info->secondary_latency_timer = buf[0x1b];

	    info->io_type = buf[0x1c] & 0x0f;
	    info->io_base = (((uint32_t) (buf[0x1c] & 0x0f0)) << 8)
	      + (((uint32_t) buf[0x30]) << 16)
	      + (((uint32_t) buf[0x31]) << 24);

	    info->io_limit = 0x00000fff
	      + (((uint32_t) (buf[0x1d] & 0x0f0)) << 8)
	      + (((uint32_t) buf[0x32]) << 16)
	      + (((uint32_t) buf[0x33]) << 24);

	    info->mem_type = buf[0x20] & 0x0f;
	    info->mem_base = (((uint32_t) (buf[0x20] & 0x0f0)) << 16)
	      + (((uint32_t) buf[0x21]) << 24);

	    info->mem_limit = 0x0000ffff
	      + (((uint32_t) (buf[0x22] & 0x0f0)) << 16)
	      + (((uint32_t) buf[0x23]) << 24);

	    info->prefetch_mem_type = buf[0x24] & 0x0f;
	    info->prefetch_mem_base = (((uint64_t) (buf[0x24] & 0x0f0)) << 16)
	      + (((uint64_t) buf[0x25]) << 24)
	      + (((uint64_t) buf[0x28]) << 32)
	      + (((uint64_t) buf[0x29]) << 40)
	      + (((uint64_t) buf[0x2a]) << 48)
	      + (((uint64_t) buf[0x2b]) << 56);

	    info->prefetch_mem_limit = 0x0000ffff
	      + (((uint64_t) (buf[0x26] & 0x0f0)) << 16)
	      + (((uint64_t) buf[0x27]) << 24)
	      + (((uint64_t) buf[0x2c]) << 32)
	      + (((uint64_t) buf[0x2d]) << 40)
	      + (((uint64_t) buf[0x2e]) << 48)
	      + (((uint64_t) buf[0x2f]) << 56);

	    info->bridge_control = ((uint16_t) buf[0x3e])
	      + (((uint16_t) buf[0x3f]) << 8);

	    info->secondary_status = ((uint16_t) buf[0x1e])
	      + (((uint16_t) buf[0x1f]) << 8);
	}

	priv->bridge.pci = info;
	break;
    }

    case 0x02: {
	struct pci_pcmcia_bridge_info *info;

	info = malloc(sizeof(*info));
	if (info != NULL) {
	    pci_device_cfg_read( (struct pci_device *) priv, buf + 0x16, 0x16,
				 0x40 - 0x16, & bytes );

	    info->primary_bus = buf[0x18];
	    info->card_bus = buf[0x19];
	    info->subordinate_bus = buf[0x1a];
	    info->cardbus_latency_timer = buf[0x1b];

	    info->mem[0].base = (((uint32_t) buf[0x1c]))
	      + (((uint32_t) buf[0x1d]) << 8)
	      + (((uint32_t) buf[0x1e]) << 16)
	      + (((uint32_t) buf[0x1f]) << 24);

	    info->mem[0].limit = (((uint32_t) buf[0x20]))
	      + (((uint32_t) buf[0x21]) << 8)
	      + (((uint32_t) buf[0x22]) << 16)
	      + (((uint32_t) buf[0x23]) << 24);

	    info->mem[1].base = (((uint32_t) buf[0x24]))
	      + (((uint32_t) buf[0x25]) << 8)
	      + (((uint32_t) buf[0x26]) << 16)
	      + (((uint32_t) buf[0x27]) << 24);

	    info->mem[1].limit = (((uint32_t) buf[0x28]))
	      + (((uint32_t) buf[0x29]) << 8)
	      + (((uint32_t) buf[0x2a]) << 16)
	      + (((uint32_t) buf[0x2b]) << 24);

	    info->io[0].base = (((uint32_t) buf[0x2c]))
	      + (((uint32_t) buf[0x2d]) << 8)
	      + (((uint32_t) buf[0x2e]) << 16)
	      + (((uint32_t) buf[0x2f]) << 24);

	    info->io[0].limit = (((uint32_t) buf[0x30]))
	      + (((uint32_t) buf[0x31]) << 8)
	      + (((uint32_t) buf[0x32]) << 16)
	      + (((uint32_t) buf[0x33]) << 24);

	    info->io[1].base = (((uint32_t) buf[0x34]))
	      + (((uint32_t) buf[0x35]) << 8)
	      + (((uint32_t) buf[0x36]) << 16)
	      + (((uint32_t) buf[0x37]) << 24);

	    info->io[1].limit = (((uint32_t) buf[0x38]))
	      + (((uint32_t) buf[0x39]) << 8)
	      + (((uint32_t) buf[0x3a]) << 16)
	      + (((uint32_t) buf[0x3b]) << 24);

	    info->secondary_status = ((uint16_t) buf[0x16])
	      + (((uint16_t) buf[0x17]) << 8);

	    info->bridge_control = ((uint16_t) buf[0x3e])
	      + (((uint16_t) buf[0x3f]) << 8);
	}

	priv->bridge.pcmcia = info;
	break;
    }
    }

    return 0;
}


const struct pci_bridge_info *
pci_device_get_bridge_info( struct pci_device * dev )
{
    struct pci_device_private * priv = (struct pci_device_private *) dev;

    if (priv->bridge.pci == NULL) {
	read_bridge_info(priv);
    }

    return (priv->header_type == 1) ? priv->bridge.pci : NULL;
}


const struct pci_pcmcia_bridge_info *
pci_device_get_pcmcia_bridge_info( struct pci_device * dev )
{
    struct pci_device_private * priv = (struct pci_device_private *) dev;

    if (priv->bridge.pcmcia == NULL) {
	read_bridge_info(priv);
    }

    return (priv->header_type == 2) ? priv->bridge.pcmcia : NULL;
}
