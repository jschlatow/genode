/*
 * \brief  IOAPIC implementation
 * \author Johannes Schlatow
 * \date   2024-03-20
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* local includes */
#include <ioapic.h>

unsigned Driver::Ioapic::_read_max_entries()
{
	write<Ioregsel>(Ioregsel::IOAPICVER);
	return read<Iowin::Maximum_entries>() + 1;
}

void Driver::Ioapic::remap_irq(unsigned irq_number)
{
	/* ignore if irq_number is not in range */
	if (irq_number < _irq_start || irq_number >= _irq_start + _max_entries)
		return;

	unsigned idx = irq_number - _irq_start;

	/* dump entry */


	/* upper 32 bit */
	write<Ioregsel>(Ioregsel::IOREDTBL + 2 * idx + 1);
	Irte::access_t irte { read<Iowin>() };
	irte <<= 32;

	/* lower 32 bit */
	write<Ioregsel>(Ioregsel::IOREDTBL + 2 * idx);
	irte |= read<Iowin>();

	if (Irte::Remap::get(irte))
		Genode::log("IRQ ", irq_number, " is remapped to ", Irte::Index::get(irte));
	else
		Genode::log("IRQ ", irq_number, " is not remapped");

	/* TODO remap entry */
}
