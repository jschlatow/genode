/*
 * \brief  Registry containing thread info about sources and core threads
 * \author Johannes Schlatow
 * \date   2017-12-21
 */

/*
 * Copyright (C) 2013-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__INCLUDE__TRACE__CORE_INFO_H_
#define _CORE__INCLUDE__TRACE__CORE_INFO_H_

#include <base/trace/types.h>
#include <trace/source_registry.h>

#include <util/xml_generator.h>

namespace Genode { namespace Trace {
	class  Core_thread;
	struct Core_info_registry;

	/**
	 * Return singleton instance of core info registry
	 */
	Core_info_registry &core_info();

} }

class Genode::Trace::Core_thread
:
	public Genode::List<Genode::Trace::Core_thread>::Element
{
	private:

		unsigned    _id;
		char const *_label;

		Core_thread(Core_thread const &);
		Core_thread &operator=(Core_thread const);

	public:

		Core_thread(unsigned id, char const *label)
		:
			_id(id), _label(label)
		{ }

		~Core_thread()
		{ }

		Thread_id    id()   const { return _id; }
		char const * name() const { return _label; }

};


class Genode::Trace::Core_info_registry
{
	private:

		Source_registry       &_sources;
		Lock                   _lock    { };
		List<Core_thread>      _threads { };
		
		struct Writer
		{
			Xml_generator &xml;

			Writer(Xml_generator &xml) : xml(xml) { }

			void operator () (Thread_id const &id, Session_label const &label, Thread_name const &name)
			{
				xml.node("thread", [&] () {
					xml.attribute("id", id.id);
					xml.attribute("name", name);
					xml.attribute("label", label);
				});
			}

			void operator () (Thread_id const &id, char const *name)
			{
				xml.node("thread", [&] () {
					xml.attribute("id", id.id);
					xml.attribute("name", name);
				});
			}
		};

	public:

		Core_info_registry(Source_registry &sources)
		:
			_sources(sources)
		{ }

		/****************************
		 ** Interface used by core **
		 ****************************/

		void insert(Core_thread *thread)
		{
			Lock::Guard guard(_lock);
			_threads.insert(thread);
		}

		void remove(Core_thread *thread)
		{
			Lock::Guard guard(_lock);
			_threads.remove(thread);
		}


		/*************************************
		 ** Interface used by TRACE service **
		 *************************************/

		template <typename WRITER>
		void export_core_threads(WRITER &writer)
		{
			for (Core_thread *th = _threads.first(); th; th = th->next()) {
				writer(th->id(), th->name());
			}
		}

		size_t export_info(char * buf_base, size_t len)
		{
			Xml_generator xml(buf_base, len, "info", [&] () {
				Writer _writer(xml);
				_sources.export_info(_writer);
				export_core_threads(_writer);
			});
			return xml.used();
		}

};

#endif /* _CORE__INCLUDE__TRACE__CORE_INFO_H_ */
