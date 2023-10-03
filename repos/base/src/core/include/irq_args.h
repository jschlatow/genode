/*
 * \brief  Utility to parse IRQ session arguments
 * \author Alexander Boettcher
 * \date   2016-07-20
 */

/*
 * Copyright (C) 2016-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__INCLUDE__IRQ_ARGS_H_
#define _CORE__INCLUDE__IRQ_ARGS_H_

#include <util/arg_string.h>
#include <irq_session/irq_session.h>

/* core includes */
#include <types.h>

namespace Core { class Irq_args; }


class Core::Irq_args
{
	private:

		Irq_session::Trigger  _irq_trigger  { Irq_session::TRIGGER_UNCHANGED };
		Irq_session::Polarity _irq_polarity { Irq_session::POLARITY_UNCHANGED };
		Irq_session::Type     _irq_type     { Irq_session::TYPE_LEGACY };

		long const _irq_number;

	public:

		Irq_args(const char * args)
		:
			_irq_number(Arg_string::find_arg(args, "irq_number").long_value(-1))
		{
			long irq_trg  = Arg_string::find_arg(args, "irq_trigger").long_value(-1);
			long irq_pol  = Arg_string::find_arg(args, "irq_polarity").long_value(-1);
			long irq_type = Arg_string::find_arg(args, "irq_type").long_value(-1);

			switch (irq_trg) {
			case -1:
			case Irq_session::TRIGGER_UNCHANGED:
				_irq_trigger = Irq_session::TRIGGER_UNCHANGED;
				break;
			case Irq_session::TRIGGER_EDGE:
				_irq_trigger = Irq_session::TRIGGER_EDGE;
				break;
			case Irq_session::TRIGGER_LEVEL:
				_irq_trigger = Irq_session::TRIGGER_LEVEL;
				break;
			default:
				error("invalid trigger mode ", irq_trg, " specified for IRQ ",
				      _irq_number);
				throw Service_denied();
			}

			switch (irq_pol) {
			case -1:
			case Irq_session::POLARITY_UNCHANGED:
				_irq_polarity = Irq_session::POLARITY_UNCHANGED;
				break;
			case Irq_session::POLARITY_HIGH:
				_irq_polarity = Irq_session::POLARITY_HIGH;
				break;
			case Irq_session::POLARITY_LOW:
				_irq_polarity = Irq_session::POLARITY_LOW;
				break;
			default:
				error("invalid polarity ", irq_pol, " specified for IRQ ",
				      _irq_number);
				throw Service_denied();
			}

			switch (irq_type) {
			case -1:
			case Irq_session::TYPE_LEGACY:
				_irq_type = Irq_session::TYPE_LEGACY;
				break;
			case Irq_session::TYPE_MSI:
				_irq_type = Irq_session::TYPE_MSI;
				break;
			case Irq_session::TYPE_MSIX:
				_irq_type = Irq_session::TYPE_MSIX;
				break;
			default:
				error("invalid type ", irq_type, " specified for IRQ ",
				      _irq_number);
				throw Service_denied();
			}
		}

		long                  irq_number() const { return _irq_number; }
		Irq_session::Trigger  trigger()    const { return _irq_trigger; }
		Irq_session::Polarity polarity()   const { return _irq_polarity; }
		Irq_session::Type     type()       const { return _irq_type; }
};

#endif /* _CORE__INCLUDE__IRQ_ARGS_H_ */
