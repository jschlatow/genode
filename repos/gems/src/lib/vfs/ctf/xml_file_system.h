/*
 * \brief  File system for providing a XML node as a file
 * \author Johannes Schlatow
 * \date   2021-08-05
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _XML_FILE_SYSTEM_H_
#define _XML_FILE_SYSTEM_H_

/* Genode includes */
#include <util/xml_generator.h>
#include <vfs/single_file_system.h>

namespace Vfs {
	template <Genode::size_t BUF_SIZE = 1024>
	class Xml_file_system;

	template <Genode::size_t CAPACITY>
	class Buffer;
}

/* Read/write buffer with seek support. */
template <Genode::size_t CAPACITY>
class Vfs::Buffer
{
	public:
		struct Eof : Genode::Exception { };

	private:
		char           _buf[CAPACITY] { };
		Genode::size_t _len           { };

	public:

		Genode::size_t read(char *dst, Genode::size_t dst_len, Genode::size_t seek_offset=0)
		{
			if (seek_offset > _len-1)
				return 0;

			Genode::size_t const len = min(_len - seek_offset, dst_len);
			Genode::memcpy(dst, &_buf[seek_offset], len);

			return len;
		}

		Genode::size_t write(char const *src, Genode::size_t src_len, Genode::size_t seek_offset=0)
		{
			if (seek_offset > CAPACITY-1)
				throw Eof();

			Genode::size_t const len = min(CAPACITY - seek_offset, src_len);
			Genode::memcpy(&_buf[seek_offset], src, len);

			_len = seek_offset + len;

			return len;
		}

		char const    *string() const { return _buf; }
		Genode::size_t length() const { return _len; }
};

template <Genode::size_t BUF_SIZE>
class Vfs::Xml_file_system : public Vfs::Single_file_system
{
	public:

		typedef Genode::String<64> Name;

	private:

		typedef Vfs::Buffer<BUF_SIZE + 1> Buffer;

		Name const _file_name;

		Buffer     _buffer { };

		struct Vfs_handle : Single_vfs_handle
		{
			Xml_file_system &_xml_fs;
			Buffer          &_buffer { _xml_fs._buffer };

			Vfs_handle(Xml_file_system &xml_fs,
			           Allocator       &alloc)
			:
				Single_vfs_handle(xml_fs, xml_fs, alloc, 0),
				_xml_fs(xml_fs)
			{ }

			Read_result read(char *dst,
			                 file_size count,
			                 file_size &out_count) override
			{
				out_count = _buffer.read(dst, count, seek());

				return READ_OK;
			}

			Write_result write(char const *src, file_size count, file_size &out_count) override
			{
				out_count = 0;

				try {
					out_count = _buffer.write(src, count, seek());
				} catch (...) {
					return WRITE_ERR_INVALID; }

				/* inform watchers */
				_xml_fs._watch_response();

				return WRITE_OK;
			}

			bool read_ready() override { return true; }

			private:

			Vfs_handle(Vfs_handle const &);
			Vfs_handle &operator = (Vfs_handle const &); 
		};

		struct Watch_handle;
		using Watch_handle_registry = Genode::Registry<Watch_handle>;

		struct Watch_handle : Vfs_watch_handle
		{
			typename Watch_handle_registry::Element elem;

			Watch_handle(Watch_handle_registry &registry,
			             Vfs::File_system      &fs,
			             Allocator             &alloc)
			: Vfs_watch_handle(fs, alloc), elem(registry, *this) { }
		};

		Watch_handle_registry _watch_handle_registry { };


		void _watch_response() {
			_watch_handle_registry.for_each([&] (Watch_handle &h) {
				h.watch_response();
			});
		}

		typedef Genode::String<200> Config;
		Config _config(Name const &name) const
		{
			char buf[Config::capacity()] { };
			Genode::Xml_generator xml(buf, sizeof(buf), type_name(), [&] () {
				xml.attribute("name", name); });
			return Config(Genode::Cstring(buf));
		}


	public:

		Xml_file_system(Name const &name, char const *initial_value)
		:
			Single_file_system(Node_type::TRANSACTIONAL_FILE, type(),
			                   Node_rwx::rw(),
			                   Xml_node(_config(name).string())),
			_file_name(name)
		{
			_buffer.write(initial_value, Genode::strlen(initial_value));
		}

		static char const *type_name() { return "xml"; }

		char const *type() override { return type_name(); }

		Xml_node xml() const
		{
			try {
				return Xml_node(_buffer.string(), _buffer.length());
			} catch (Xml_node::Invalid_syntax) { }

			return Xml_node("<empty/>");
		}

		bool matches(Xml_node node) const
		{
			return node.has_type(type_name()) &&
			       node.attribute_value("name", Name()) == _file_name;
		}


		/********************************
		 ** File I/O service interface **
		 ********************************/

		Ftruncate_result ftruncate(Vfs::Vfs_handle *, file_size size) override
		{
			if (size >= BUF_SIZE)
				return FTRUNCATE_ERR_NO_SPACE;

			return FTRUNCATE_OK;
		}


		/*********************************
		 ** Directory-service interface **
		 *********************************/

		Open_result open(char const  *path, unsigned,
		                 Vfs::Vfs_handle **out_handle,
		                 Allocator   &alloc) override
		{
			if (!_single_file(path))
				return OPEN_ERR_UNACCESSIBLE;

			try {
				*out_handle = new (alloc) Vfs_handle(*this, alloc);
				return OPEN_OK;
			}
			catch (Genode::Out_of_ram)  { Genode::error("out of ram"); return OPEN_ERR_OUT_OF_RAM; }
			catch (Genode::Out_of_caps) { Genode::error("out of caps");return OPEN_ERR_OUT_OF_CAPS; }
		}

		Stat_result stat(char const *path, Stat &out) override
		{
			Stat_result result = Single_file_system::stat(path, out);
			out.size = _buffer.length();
			return result;
		}

		Watch_result watch(char const      *path,
		                   Vfs_watch_handle **handle,
		                   Allocator        &alloc) override
		{
			if (!_single_file(path))
				return WATCH_ERR_UNACCESSIBLE;

			try {
				Watch_handle *wh = new (alloc)
					Watch_handle(_watch_handle_registry, *this, alloc);
				*handle = wh;
				return WATCH_OK;
			}
			catch (Genode::Out_of_ram)  { return WATCH_ERR_OUT_OF_RAM;  }
			catch (Genode::Out_of_caps) { return WATCH_ERR_OUT_OF_CAPS; }
		}

		void close(Vfs_watch_handle *handle) override
		{
			if (handle && (&handle->fs() == this))
				destroy(handle->alloc(), handle);
		}
};

#endif /* _XML_FILE_SYSTEM_H_ */
