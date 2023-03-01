/*
 * linux/arch/arm/mach-footbridge/personal-pci.c
 *
 * PCI bios-type initialisation for PCI machines
 *
 * Bits taken from various places.
 */
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>

#include <asm/irq.h>
#include <asm/mach/pci.h>

static int irqmap_personal_server[] __initdata = {
	IRQ_IN0, IRQ_IN1, IRQ_IN2, IRQ_IN3, 0, 0, 0,
	IRQ_DOORBELLHOST, IRQ_DMA1, IRQ_DMA2, IRQ_PCI
};

static int __init personal_server_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	unsigned char line;

	pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &line);

	if (line > 0x40 && line <= 0x5f) {
		/* line corresponds to the bit controlling this interrupt
		 * in the footbridge.  Ignore the first 8 interrupt bits,
		 * look up the rest in the map.  IN0 is bit number 8
		 */
		return irqmap_personal_server[(line & 0x1f) - 8];
	} else if (line == 0) {
		/* no interrupt */
		return 0;
	} else
		return irqmap_personal_server[(line - 1) & 3];
}

struct hw_pci personal_server_pci __initdata = {
	init:		dc21285_init,
	swizzle:	no_swizzle,
	map_irq:	personal_server_map_irq,
};
