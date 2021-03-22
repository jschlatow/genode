/*
 * \brief  Integration of the Consistent Block Encrypter (CBE)
 * \author Martin Stein
 * \author Josef Soentgen
 * \date   2020-11-10
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <vfs/dir_file_system.h>
#include <vfs/single_file_system.h>
#include <util/arg_string.h>
#include <util/xml_generator.h>

/* CBE includes */
#include <cbe/vfs/io_job.h>


static void xor_bytes(unsigned char const *p, int p_len,
                      unsigned char *k, int k_len)
{
	for (int i = 0; i < k_len; i++) {
		k[i] ^= p[i % p_len];
	}
}


static void fill_bytes(unsigned char *v, int v_len)
{
	static unsigned char _fill_counter = 0x23;

	for (int i = 0; i < v_len; i++) {
		v[i] = _fill_counter;
	}

	++_fill_counter;
}


namespace Vfs_cbe_trust_anchor {

	using namespace Vfs;
	using namespace Genode;

	class Generate_key_file_system;
	class Hashsum_file_system;
	class Encrypt_file_system;
	class Decrypt_file_system;
	class Initialize_file_system;

	struct Local_factory;
	class  File_system;
}


class Trust_anchor
{
	public:

		using Path = Genode::Path<256>;

		Path const key_file_name  { "keyfile" };
		Path const hash_file_name { "secured_superblock" };

		struct Complete_request
		{
			bool valid;
			bool success;
		};

	private:

		Trust_anchor(Trust_anchor const &) = delete;
		Trust_anchor &operator=(Trust_anchor const&) = delete;

		using size_t = Genode::size_t;

		Vfs::Env &_vfs_env;

		enum class State {
			UNINITIALIZED,
			INITIALIZIE_IN_PROGRESS,
			INITIALIZED,
		};
		State _state { State::UNINITIALIZED };

		enum class Lock_state { LOCKED, UNLOCKED };
		Lock_state _lock_state { Lock_state::LOCKED };

		enum class Job {
			NONE,
			DECRYPT,
			ENCRYPT,
			GENERATE,
			INIT,
			READ_HASH,
			UPDATE_HASH,
			UNLOCK
		};
		Job _job { Job::NONE };

		enum class Job_state { NONE, PENDING, IN_PROGRESS, FINAL_SYNC, COMPLETE };
		Job_state _job_state { Job_state::NONE };

		bool _job_success { false };

		struct Private_key
		{
			enum { KEY_LEN = 32 };
			unsigned char value[KEY_LEN] { };
		};
		Private_key _private_key { };

		struct Last_hash
		{
			enum { HASH_LEN = 32 };
			unsigned char value[HASH_LEN] { };
			static constexpr size_t length = HASH_LEN;
		};
		Last_hash _last_hash { };

		struct Key
		{
			enum { KEY_LEN = 32 };
			unsigned char value[KEY_LEN] { };
			static constexpr size_t length = KEY_LEN;
		};
		Key _decrypt_key   { };
		Key _encrypt_key   { };
		Key _generated_key { };

		void _xcrypt_key(Private_key const &priv_key, Key &key)
		{
			xor_bytes(priv_key.value, (int)Private_key::KEY_LEN,
			          key.value,      (int)Key::KEY_LEN);
		}

		void _fill_key(Key &key)
		{
			fill_bytes(key.value, (int)Key::KEY_LEN);
		}

		bool _execute_xcrypt(Key &key)
		{
			switch (_job_state) {
			case Job_state::PENDING:
				_xcrypt_key(_private_key, key);
				_job_state = Job_state::COMPLETE;
				_job_success = true;
				[[fallthrough]];
			case Job_state::COMPLETE:
				return true;

			case Job_state::IN_PROGRESS: [[fallthrough]];
			case Job_state::NONE:        [[fallthrough]];
			default:                     return false;
			}

			/* never reached */
			return false;
		}

		bool _execute_generate(Key &key)
		{
			switch (_job_state) {
			case Job_state::PENDING:
				_fill_key(key);
				_job_state = Job_state::COMPLETE;
				_job_success = true;
				[[fallthrough]];
			case Job_state::COMPLETE:
				return true;

			case Job_state::IN_PROGRESS: [[fallthrough]];
			case Job_state::NONE:        [[fallthrough]];
			default:                     return false;
			}

			/* never reached */
			return false;
		}

		bool _execute_unlock()
		{
			bool progress = false;

			switch (_job_state) {
			case Job_state::PENDING:
			{
				if (!_open_key_file_and_queue_read(_base_path)) {
					break;
				}

				_job_state = Job_state::IN_PROGRESS;
				progress |= true;
			}

			[[fallthrough]];
			case Job_state::IN_PROGRESS:
			{
				if (!_read_key_file_finished()) {
					break;
				}

				Private_key key { };

				/* copy passphrase to key object */
				size_t const key_len =
					Genode::min(_key_io_job_buffer.size,
					            sizeof (key.value));

				Genode::memset(key.value, 0xa5, sizeof (key.value));
				Genode::memcpy(key.value, _key_io_job_buffer.buffer, key_len);

				_job_state   = Job_state::COMPLETE;
				_job_success = Genode::memcmp(_private_key.value, key.value,
				                              sizeof (key.value));

				progress |= true;
			}

			[[fallthrough]];
			case Job_state::COMPLETE:
				break;

			case Job_state::NONE: [[fallthrough]];
			default:              break;
			}

			return progress;
		}

		bool _execute_init()
		{
			bool progress = false;

			switch (_job_state) {
			case Job_state::PENDING:
			{
				if (!_open_key_file_and_write(_base_path)) {
					_job_state = Job_state::COMPLETE;
					_job_success = false;
					return true;
				}

				/* copy passphrase to key object */
				size_t const key_len =
					Genode::min(_key_io_job_buffer.size,
					            sizeof (_private_key.value));
				Genode::memset(_private_key.value, 0xa5, sizeof (_private_key.value));
				Genode::memcpy(_private_key.value, _key_io_job_buffer.buffer, key_len);

				_job_state = Job_state::IN_PROGRESS;
				progress |= true;
			}

			[[fallthrough]];
			case Job_state::IN_PROGRESS:
				if (!_write_key_file_finished()) {
					break;
				}

				_job_state   = Job_state::COMPLETE;
				_job_success = true;

				progress |= true;
			[[fallthrough]];
			case Job_state::COMPLETE:
				break;

			case Job_state::NONE: [[fallthrough]];
			default:              break;
			}

			return progress;
		}

		bool _execute_read_hash()
		{
			bool progress = false;

			switch (_job_state) {
			case Job_state::PENDING:
				if (!_open_hash_file_and_queue_read(_base_path)) {
					_job_state = Job_state::COMPLETE;
					_job_success = false;
					return true;
				}

				_job_state = Job_state::IN_PROGRESS;
				progress |= true;

			[[fallthrough]];
			case Job_state::IN_PROGRESS:
			{
				if (!_read_hash_file_finished()) {
					break;
				}

				size_t const hash_len =
					Genode::min(_hash_io_job_buffer.size,
					            sizeof (_last_hash.value));
				Genode::memcpy(_last_hash.value, _hash_io_job_buffer.buffer,
				               hash_len);

				_job_state   = Job_state::COMPLETE;
				_job_success = true;

				progress |= true;
			}
			[[fallthrough]];
			case Job_state::COMPLETE:
				break;

			case Job_state::NONE: [[fallthrough]];
			default:              break;
			}

			return progress;
		}


		bool _execute_update_hash()
		{
			bool progress = false;

			switch (_job_state) {
			case Job_state::PENDING:
			{
				if (!_open_hash_file_and_write(_base_path)) {
					_job_state = Job_state::COMPLETE;
					_job_success = false;
					return true;
				}

				/* keep new hash in last hash */
				size_t const hash_len =
					Genode::min(_hash_io_job_buffer.size,
					            sizeof (_hash_io_job_buffer.size));
				Genode::memcpy(_last_hash.value, _hash_io_job_buffer.buffer,
				               hash_len);

				_job_state = Job_state::IN_PROGRESS;
				progress |= true;
			}
			[[fallthrough]];
			case Job_state::IN_PROGRESS:
				if (!_write_op_on_hash_file_is_in_final_sync_step()) {
					break;
				}

				_job_state   = Job_state::FINAL_SYNC;
				_job_success = true;

				progress |= true;
			[[fallthrough]];
			case Job_state::FINAL_SYNC:

				if (!_final_sync_of_write_op_on_hash_file_finished()) {
					break;
				}

				_job_state   = Job_state::COMPLETE;
				_job_success = true;

				progress |= true;
			[[fallthrough]];
			case Job_state::COMPLETE:
				break;

			case Job_state::NONE: [[fallthrough]];
			default:              break;
			}

			return progress;
		}

		bool _execute()
		{
			switch (_job) {
			case Job::DECRYPT:     return _execute_xcrypt(_decrypt_key);
			case Job::ENCRYPT:     return _execute_xcrypt(_encrypt_key);
			case Job::GENERATE:    return _execute_generate(_generated_key);
			case Job::INIT:        return _execute_init();
			case Job::READ_HASH:   return _execute_read_hash();
			case Job::UPDATE_HASH: return _execute_update_hash();
			case Job::UNLOCK     : return _execute_unlock();
			case Job::NONE: [[fallthrough]];
			default: return false;
			}

			/* never reached */
			return false;
		}

		friend struct Io_response_handler;

		struct Io_response_handler : Vfs::Io_response_handler
		{
			Genode::Signal_context_capability _io_sigh;

			Io_response_handler(Genode::Signal_context_capability io_sigh)
			: _io_sigh(io_sigh) { }

			void read_ready_response() override { }

			void io_progress_response() override
			{
				if (_io_sigh.valid()) {
					Genode::Signal_transmitter(_io_sigh).submit();
				}
			}
		};

		void _handle_io()
		{
			(void)_execute();
		}

		Genode::Io_signal_handler<Trust_anchor> _io_handler {
			_vfs_env.env().ep(), *this, &Trust_anchor::_handle_io };

		Io_response_handler _io_response_handler { _io_handler };

		void _close_handle(Vfs::Vfs_handle **handle)
		{
			(*handle)->close();
			(*handle) = nullptr;
		}

		/* key */

		Vfs::Vfs_handle *_key_handle  { nullptr };
		Genode::Constructible<Util::Io_job> _key_io_job { };

		struct Key_io_job_buffer : Util::Io_job::Buffer
		{
			char buffer[64] { };

			Key_io_job_buffer()
			{
				Buffer::base = buffer;
				Buffer::size = sizeof (buffer);
			}
		};

		Key_io_job_buffer _key_io_job_buffer { };

		bool _check_key_file(Path const &path)
		{
			Path file_path = path;
			file_path.append_element(key_file_name.string());

			using Stat_result = Vfs::Directory_service::Stat_result;

			Vfs::Directory_service::Stat out_stat { };
			Stat_result const stat_res =
				_vfs_env.root_dir().stat(file_path.string(), out_stat);

			if (stat_res == Stat_result::STAT_OK) {

				_state = State::INITIALIZED;
				return true;
			}
			_state = State::UNINITIALIZED;
			return false;
		}

		bool _open_key_file_and_queue_read(Path const &path)
		{
			Path file_path = path;
			file_path.append_element(key_file_name.string());

			using Result = Vfs::Directory_service::Open_result;

			Result const res =
				_vfs_env.root_dir().open(file_path.string(),
				                         Vfs::Directory_service::OPEN_MODE_RDONLY,
				                         (Vfs::Vfs_handle **)&_key_handle,
				                         _vfs_env.alloc());
			if (res != Result::OPEN_OK) {
				Genode::error("could not open '", file_path.string(), "'");
				return false;
			}

			_key_handle->handler(&_io_response_handler);
			_key_io_job.construct(*_key_handle, Util::Io_job::Operation::READ,
			                      _key_io_job_buffer, 0,
			                      Util::Io_job::Partial_result::ALLOW);
			if (_key_io_job->execute() && _key_io_job->completed()) {
				_state = State::INITIALIZED;
				_close_handle(&_key_handle);
				return true;
			}
			return true;
		}

		bool _read_key_file_finished()
		{
			if (!_key_io_job.constructed()) {
				return true;
			}

			// XXX trigger sync

			bool const progress  = _key_io_job->execute();
			bool const completed = _key_io_job->completed();
			if (completed) {
				_state = State::INITIALIZED;
				_close_handle(&_key_handle);
				_key_io_job.destruct();
			}

			return progress && completed;
		}

		bool _open_key_file_and_write(Path const &path)
		{
			using Result = Vfs::Directory_service::Open_result;

			Path file_path = path;
			file_path.append_element(key_file_name.string());

			unsigned const mode =
				Vfs::Directory_service::OPEN_MODE_WRONLY | Vfs::Directory_service::OPEN_MODE_CREATE;

			Result const res =
				_vfs_env.root_dir().open(file_path.string(), mode,
				                         (Vfs::Vfs_handle **)&_key_handle,
				                         _vfs_env.alloc());
			if (res != Result::OPEN_OK) {
				return false;
			}

			_key_handle->handler(&_io_response_handler);
			_key_io_job.construct(*_key_handle, Util::Io_job::Operation::WRITE,
			                      _key_io_job_buffer, 0);
			if (_key_io_job->execute() && _key_io_job->completed()) {
				_state = State::INITIALIZED;
				_close_handle(&_key_handle);
				_key_io_job.destruct();
				return true;
			}
			return true;
		}

		bool _write_key_file_finished()
		{
			if (!_key_io_job.constructed()) {
				return true;
			}

			// XXX trigger sync

			bool const progress  = _key_io_job->execute();
			bool const completed = _key_io_job->completed();
			if (completed) {
				_state = State::INITIALIZED;
				_close_handle(&_key_handle);
				_key_io_job.destruct();
			}

			return progress && completed;
		}


		/* hash */

		Vfs::Vfs_handle *_hash_handle { nullptr };

		Genode::Constructible<Util::Io_job> _hash_io_job { };

		struct Hash_io_job_buffer : Util::Io_job::Buffer
		{
			char buffer[64] { };

			Hash_io_job_buffer()
			{
				Buffer::base = buffer;
				Buffer::size = sizeof (buffer);
			}
		};

		Hash_io_job_buffer _hash_io_job_buffer { };

		bool _open_hash_file_and_queue_read(Path const &path)
		{
			using Result = Vfs::Directory_service::Open_result;

			Path file_path = path;
			file_path.append_element(hash_file_name.string());

			Result const res =
				_vfs_env.root_dir().open(file_path.string(),
				                         Vfs::Directory_service::OPEN_MODE_RDONLY,
				                         (Vfs::Vfs_handle **)&_hash_handle,
				                         _vfs_env.alloc());
			if (res != Result::OPEN_OK) {
				return false;
			}

			_hash_handle->handler(&_io_response_handler);
			_hash_io_job.construct(*_hash_handle, Util::Io_job::Operation::READ,
			                       _hash_io_job_buffer, 0,
			                       Util::Io_job::Partial_result::ALLOW);
			if (_hash_io_job->execute() && _hash_io_job->completed()) {
				_close_handle(&_hash_handle);
				_hash_io_job.destruct();
				return true;
			}
			return true;
		}

		bool _read_hash_file_finished()
		{
			if (!_hash_io_job.constructed()) {
				return true;
			}
			bool const progress  = _hash_io_job->execute();
			bool const completed = _hash_io_job->completed();
			if (completed) {
				_close_handle(&_hash_handle);
				_hash_io_job.destruct();
			}

			return progress && completed;
		}

		void _start_sync_at_hash_io_job()
		{
			_hash_io_job.construct(*_hash_handle, Util::Io_job::Operation::SYNC,
			                       _hash_io_job_buffer, 0);
		}

		bool _open_hash_file_and_write(Path const &path)
		{
			using Result = Vfs::Directory_service::Open_result;

			Path file_path = path;
			file_path.append_element(hash_file_name.string());

			using Stat_result = Vfs::Directory_service::Stat_result;

			Vfs::Directory_service::Stat out_stat { };
			Stat_result const stat_res =
				_vfs_env.root_dir().stat(file_path.string(), out_stat);

			bool const file_exists = stat_res == Stat_result::STAT_OK;

			unsigned const mode =
				Vfs::Directory_service::OPEN_MODE_WRONLY |
				(file_exists ? 0 : Vfs::Directory_service::OPEN_MODE_CREATE);

			Result const res =
				_vfs_env.root_dir().open(file_path.string(), mode,
				                         (Vfs::Vfs_handle **)&_hash_handle,
				                         _vfs_env.alloc());
			if (res != Result::OPEN_OK) {
				Genode::error("could not open '", file_path.string(), "'");
				return false;
			}

			_hash_handle->handler(&_io_response_handler);
			_hash_io_job.construct(*_hash_handle, Util::Io_job::Operation::WRITE,
			                      _hash_io_job_buffer, 0);

			if (_hash_io_job->execute() && _hash_io_job->completed()) {
				_start_sync_at_hash_io_job();
			}
			return true;
		}

		bool _write_op_on_hash_file_is_in_final_sync_step()
		{
			if (_hash_io_job->op() == Util::Io_job::Operation::SYNC) {
				return true;
			}
			bool const progress  = _hash_io_job->execute();
			bool const completed = _hash_io_job->completed();
			if (completed) {
				_start_sync_at_hash_io_job();
			}
			return progress && completed;
		}

		bool _final_sync_of_write_op_on_hash_file_finished()
		{
			if (!_hash_io_job.constructed()) {
				return true;
			}
			bool const progress  = _hash_io_job->execute();
			bool const completed = _hash_io_job->completed();
			if (completed) {
				_close_handle(&_hash_handle);
				_hash_io_job.destruct();
			}
			return progress && completed;
		}

		Path const _base_path;

	public:

		Trust_anchor(Vfs::Env &vfs_env, Path const &path)
		:
			_vfs_env   { vfs_env },
			_base_path { path }
		{
			if (_check_key_file(_base_path)) {

				if (_open_key_file_and_queue_read(_base_path)) {

					while (!_read_key_file_finished()) {
						_vfs_env.env().ep().wait_and_dispatch_one_io_signal();
					}
				}
			}
			else {
				Genode::log("No key file found, TA not initialized");
			}
		}

		bool initialized() const
		{
			return _state == State:: INITIALIZED;
		}

		bool execute()
		{
			return _execute();
		}

		bool queue_initialize(char const *src, size_t len)
		{
			if (_job != Job::NONE) {
				return false;
			}

			if (_state != State::UNINITIALIZED) {
				return false;
			}

			if (len > _key_io_job_buffer.size) {
				len = _key_io_job_buffer.size;
			}

			_key_io_job_buffer.size = len;

			Genode::memcpy(_key_io_job_buffer.buffer, src,
			               _key_io_job_buffer.size);

			_job       = Job::INIT;
			_job_state = Job_state::PENDING;
			return true;
		}

		Complete_request complete_queue_initialize()
		{
			if (_job != Job::INIT || _job_state != Job_state::COMPLETE) {
				return { false, false };
			}

			_lock_state = Lock_state::UNLOCKED;

			_job       = Job::NONE;
			_job_state = Job_state::NONE;
			return { true, _job_success };
		}

		bool queue_unlock(char const *src, size_t len)
		{
			if (_job != Job::NONE) {
				return false;
			}

			if (_state != State::INITIALIZED) {
				return false;
			}

			if (_lock_state == Lock_state::UNLOCKED) {
				_job         = Job::UNLOCK;
				_job_state   = Job_state::COMPLETE;
				_job_success = true;
				return true;
			}

			if (len > _key_io_job_buffer.size) {
				len = _key_io_job_buffer.size;
			}

			_key_io_job_buffer.size = len;

			Genode::memcpy(_key_io_job_buffer.buffer, src,
			               _key_io_job_buffer.size);

			_job       = Job::UNLOCK;
			_job_state = Job_state::PENDING;
			return true;
		}

		Complete_request complete_queue_unlock()
		{
			if (_job != Job::UNLOCK || _job_state != Job_state::COMPLETE) {
				return { false, false };
			}

			_lock_state = Lock_state::UNLOCKED;

			_job       = Job::NONE;
			_job_state = Job_state::NONE;
			return { true, _job_success };
		}

		bool queue_read_last_hash()
		{
			if (_job != Job::NONE) {
				return false;
			}

			if (_lock_state != Lock_state::UNLOCKED) {
				return false;
			}

			_job       = Job::READ_HASH;
			_job_state = Job_state::PENDING;
			return true;
		}

		Complete_request complete_read_last_hash(char *dst, size_t len)
		{
			if (_job != Job::READ_HASH || _job_state != Job_state::COMPLETE) {
				return { false, false };
			}

			if (len < _last_hash.length) {
				Genode::warning("truncate hash");
			}

			Genode::memcpy(dst, _last_hash.value, len);

			_job       = Job::NONE;
			_job_state = Job_state::NONE;
			return { true, _job_success };
		}

		bool queue_update_last_hash(char const *src, size_t len)
		{
			if (_job != Job::NONE) {
				return false;
			}

			if (_lock_state != Lock_state::UNLOCKED) {
				return false;
			}

			if (len != _last_hash.length) {
				return false;
			}

			if (len > _hash_io_job_buffer.size) {
				len = _hash_io_job_buffer.size;
			}

			_hash_io_job_buffer.size = len;

			Genode::memcpy(_hash_io_job_buffer.buffer, src,
			               _hash_io_job_buffer.size);

			Genode::memcpy(_last_hash.value, src, len);

			_job       = Job::UPDATE_HASH;
			_job_state = Job_state::PENDING;
			return true;
		}

		Complete_request complete_update_last_hash()
		{
			if (_job != Job::UPDATE_HASH || _job_state != Job_state::COMPLETE) {
				return { false, false };
			}

			_job       = Job::NONE;
			_job_state = Job_state::NONE;
			return { true, _job_success };
		}

		bool queue_encrypt_key(char const *src, size_t len)
		{
			if (_job != Job::NONE) {
				return false;
			}

			if (_lock_state != Lock_state::UNLOCKED) {
				return false;
			}

			if (len != _encrypt_key.length) {
				Genode::error(__func__, ": key length mismatch, expected: ",
				              _encrypt_key.length, " got: ", len);
				return false;
			}

			Genode::memcpy(_encrypt_key.value, src, len);

			_job       = Job::ENCRYPT;
			_job_state = Job_state::PENDING;
			return true;
		}

		Complete_request complete_encrypt_key(char *dst, size_t len)
		{
			if (_job != Job::ENCRYPT || _job_state != Job_state::COMPLETE) {
				return { false, false };
			}

			if (len != _encrypt_key.length) {
				Genode::error(__func__, ": key length mismatch, expected: ",
				              _encrypt_key.length, " got: ", len);
				return { true, false };
			}

			Genode::memcpy(dst, _encrypt_key.value, _encrypt_key.length);

			_job       = Job::NONE;
			_job_state = Job_state::NONE;
			return { true, _job_success };
		}

		bool queue_decrypt_key(char const *src, size_t len)
		{
			if (_job != Job::NONE) {
				return false;
			}

			if (_lock_state != Lock_state::UNLOCKED) {
				return false;
			}

			if (len != _decrypt_key.length) {
				Genode::error(__func__, ": key length mismatch, expected: ",
				              _decrypt_key.length, " got: ", len);
				return false;
			}

			Genode::memcpy(_decrypt_key.value, src, len);

			_job       = Job::DECRYPT;
			_job_state = Job_state::PENDING;
			return true;
		}

		Complete_request complete_decrypt_key(char *dst, size_t len)
		{
			if (_job != Job::DECRYPT || _job_state != Job_state::COMPLETE) {
				return { false, false };
			}

			if (len != _decrypt_key.length) {
				Genode::error(__func__, ": key length mismatch, expected: ",
				              _decrypt_key.length, " got: ", len);
				return { true, false };
			}

			Genode::memcpy(dst, _decrypt_key.value, _decrypt_key.length);

			_job       = Job::NONE;
			_job_state = Job_state::NONE;
			return { true, _job_success };
		}

		bool queue_generate_key()
		{
			if (_job_state != Job_state::NONE) {
				return false;
			}

			_job       = Job::GENERATE;
			_job_state = Job_state::PENDING;
			return true;
		}

		Complete_request complete_generate_key(char *dst, size_t len)
		{
			if (_job != Job::GENERATE || _job_state != Job_state::COMPLETE) {
				return { false, false };
			}

			if (len < _generated_key.length) {
				Genode::warning("truncate generated key");
			} else

			if (len > _generated_key.length) {
				len = _generated_key.length;
			}

			Genode::memcpy(dst, _generated_key.value, len);
			Genode::memset(_generated_key.value, 0, sizeof (_generated_key.value));

			_job       = Job::NONE;
			_job_state = Job_state::NONE;
			return { true, _job_success };
		}
};


class Vfs_cbe_trust_anchor::Hashsum_file_system : public Vfs::Single_file_system
{
	private:

		Trust_anchor &_trust_anchor;

		struct Hashsum_handle : Single_vfs_handle
		{
			Trust_anchor &_trust_anchor;

			enum class State { NONE, PENDING_WRITE_ACK, PENDING_READ };
			State _state { State::NONE };

			Hashsum_handle(Directory_service &ds,
			               File_io_service   &fs,
			               Genode::Allocator &alloc,
			               Trust_anchor      &ta)
			:
				Single_vfs_handle { ds, fs, alloc, 0 },
				_trust_anchor     { ta }
			{ }

			Read_result read(char *src, file_size count,
			                 file_size &out_count) override
			{
				if (_state == State::NONE) {
					try {
						bool const ok =
							_trust_anchor.queue_read_last_hash();
						if (!ok) {
							return READ_ERR_IO;
						}
						_state = State::PENDING_READ;
					} catch (...) {
						return READ_ERR_INVALID;
					}

					_trust_anchor.execute();
					return READ_QUEUED;
				} else

				if (_state == State::PENDING_READ) {
					try {
						Trust_anchor::Complete_request const cr =
							_trust_anchor.complete_read_last_hash(src, count);
						if (!cr.valid) {
							_trust_anchor.execute();
							return READ_QUEUED;
						}

						_state = State::NONE;
						out_count = count;
						return cr.success ? READ_OK : READ_ERR_IO;
					} catch (...) {
						return READ_ERR_INVALID;
					}
				} else

				if (_state == State::PENDING_WRITE_ACK) {
					try {
						Trust_anchor::Complete_request const cr =
							_trust_anchor.complete_update_last_hash();
						if (!cr.valid) {
							_trust_anchor.execute();
							return READ_QUEUED;
						}

						_state = State::NONE;
						out_count = count;
						return cr.success ? READ_OK : READ_ERR_IO;
					} catch (...) {
						return READ_ERR_INVALID;
					}
				}

				return READ_ERR_IO;
			}

			Write_result write(char const *src, file_size count,
			                   file_size &out_count) override
			{
				if (_state != State::NONE) {
					return WRITE_ERR_IO;
				}

				try {
					bool const ok =
						_trust_anchor.queue_update_last_hash(src, count);
					if (!ok) {
						return WRITE_ERR_IO;
					}
					_state = State::PENDING_WRITE_ACK;
				} catch (...) {
					return WRITE_ERR_INVALID;
				}

				_trust_anchor.execute();
				out_count = count;
				return WRITE_OK;
			}

			bool read_ready() override { return true; }
		};

	public:

		Hashsum_file_system(Trust_anchor &ta)
		:
			Single_file_system { Node_type::TRANSACTIONAL_FILE, type_name(),
			                     Node_rwx::ro(), Xml_node("<hashsum/>") },
			_trust_anchor      { ta }
		{ }

		static char const *type_name() { return "hashsum"; }

		char const *type() override { return type_name(); }

		/*********************************
		 ** Directory-service interface **
		 *********************************/

		Open_result open(char const  *path, unsigned,
		                 Vfs::Vfs_handle **out_handle,
		                 Genode::Allocator   &alloc) override
		{
			if (!_single_file(path))

				return OPEN_ERR_UNACCESSIBLE;

			try {
				*out_handle =
					new (alloc) Hashsum_handle(*this, *this, alloc,
					                           _trust_anchor);
				return OPEN_OK;
			}
			catch (Genode::Out_of_ram)  { return OPEN_ERR_OUT_OF_RAM; }
			catch (Genode::Out_of_caps) { return OPEN_ERR_OUT_OF_CAPS; }
		}

		Stat_result stat(char const *path, Stat &out) override
		{
			Stat_result result = Single_file_system::stat(path, out);
			return result;
		}

		/********************************
		 ** File I/O service interface **
		 ********************************/

		Ftruncate_result ftruncate(Vfs::Vfs_handle *, file_size) override
		{
			return FTRUNCATE_OK;
		}
};


class Vfs_cbe_trust_anchor::Generate_key_file_system : public Vfs::Single_file_system
{
	private:

		Trust_anchor &_trust_anchor;

		struct Gen_key_handle : Single_vfs_handle
		{
			Trust_anchor &_trust_anchor;

			enum class State { NONE, PENDING };
			State _state { State::NONE, };

			Gen_key_handle(Directory_service &ds,
			               File_io_service   &fs,
			               Genode::Allocator &alloc,
			               Trust_anchor      &ta)
			:
				Single_vfs_handle { ds, fs, alloc, 0 },
				_trust_anchor     { ta }
			{ }

			Read_result read(char *dst, file_size count,
			                 file_size &out_count) override
			{
				if (_state == State::NONE) {

					if (!_trust_anchor.queue_generate_key()) {
						return READ_QUEUED;
					}
					_state = State::PENDING;
				}

				(void)_trust_anchor.execute();

				Trust_anchor::Complete_request const cr =
					_trust_anchor.complete_generate_key(dst, count);
				if (!cr.valid) {
					return READ_QUEUED;
				}

				_state = State::NONE;
				out_count = count;
				return cr.success ? READ_OK : READ_ERR_IO;
			}

			Write_result write(char const *, file_size , file_size &) override
			{
				return WRITE_ERR_IO;
			}

			bool read_ready() override { return true; }
		};

	public:

		Generate_key_file_system(Trust_anchor &ta)
		:
			Single_file_system { Node_type::TRANSACTIONAL_FILE, type_name(),
			                     Node_rwx::ro(), Xml_node("<generate_key/>") },
			_trust_anchor      { ta }
		{ }

		static char const *type_name() { return "generate_key"; }

		char const *type() override { return type_name(); }

		/*********************************
		 ** Directory-service interface **
		 *********************************/

		Open_result open(char const  *path, unsigned,
		                 Vfs::Vfs_handle **out_handle,
		                 Genode::Allocator   &alloc) override
		{
			if (!_single_file(path))

				return OPEN_ERR_UNACCESSIBLE;

			try {
				*out_handle =
					new (alloc) Gen_key_handle(*this, *this, alloc,
					                           _trust_anchor);
				return OPEN_OK;
			}
			catch (Genode::Out_of_ram)  { return OPEN_ERR_OUT_OF_RAM; }
			catch (Genode::Out_of_caps) { return OPEN_ERR_OUT_OF_CAPS; }
		}

		Stat_result stat(char const *path, Stat &out) override
		{
			Stat_result result = Single_file_system::stat(path, out);
			return result;
		}

		/********************************
		 ** File I/O service interface **
		 ********************************/

		Ftruncate_result ftruncate(Vfs::Vfs_handle *, file_size) override
		{
			return FTRUNCATE_OK;
		}
};


class Vfs_cbe_trust_anchor::Encrypt_file_system : public Vfs::Single_file_system
{
	private:

		Trust_anchor &_trust_anchor;

		struct Encrypt_handle : Single_vfs_handle
		{
			Trust_anchor &_trust_anchor;

			enum State { NONE, PENDING };
			State _state;

			Encrypt_handle(Directory_service &ds,
			               File_io_service   &fs,
			               Genode::Allocator &alloc,
			               Trust_anchor      &ta)
			:
				Single_vfs_handle { ds, fs, alloc, 0 },
				_trust_anchor { ta },
				_state        { State::NONE }
			{ }

			Read_result read(char *dst, file_size count,
			                 file_size &out_count) override
			{
				if (_state != State::PENDING) {
					return READ_ERR_IO;
				}

				_trust_anchor.execute();

				try {
					Trust_anchor::Complete_request const cr =
						_trust_anchor.complete_encrypt_key(dst, count);
					if (!cr.valid) {
						return READ_QUEUED;
					}

					_state = State::NONE;

					out_count = count;
					return cr.success ? READ_OK : READ_ERR_IO;
				} catch (...) {
					return READ_ERR_INVALID;
				}

				return READ_ERR_IO;
			}

			Write_result write(char const *src, file_size count,
			                   file_size &out_count) override
			{
				if (_state != State::NONE) {
					return WRITE_ERR_IO;
				}

				try {
					bool const ok =
						_trust_anchor.queue_encrypt_key(src, count);
					if (!ok) {
						return WRITE_ERR_IO;
					}
					_state = State::PENDING;
				} catch (...) {
					return WRITE_ERR_INVALID;
				}

				_trust_anchor.execute();
				out_count = count;
				return WRITE_OK;
			}

			bool read_ready() override { return true; }
		};

	public:

		Encrypt_file_system(Trust_anchor &ta)
		:
			Single_file_system { Node_type::TRANSACTIONAL_FILE, type_name(),
			                     Node_rwx::rw(), Xml_node("<encrypt/>") },
			_trust_anchor { ta }
		{ }

		static char const *type_name() { return "encrypt"; }

		char const *type() override { return type_name(); }

		/*********************************
		 ** Directory-service interface **
		 *********************************/

		Open_result open(char const  *path, unsigned,
		                 Vfs::Vfs_handle **out_handle,
		                 Genode::Allocator   &alloc) override
		{
			if (!_single_file(path))
				return OPEN_ERR_UNACCESSIBLE;

			try {
				*out_handle =
					new (alloc) Encrypt_handle(*this, *this, alloc,
					                           _trust_anchor);
				return OPEN_OK;
			}
			catch (Genode::Out_of_ram)  { return OPEN_ERR_OUT_OF_RAM; }
			catch (Genode::Out_of_caps) { return OPEN_ERR_OUT_OF_CAPS; }
		}

		Stat_result stat(char const *path, Stat &out) override
		{
			Stat_result result = Single_file_system::stat(path, out);
			return result;
		}

		/********************************
		 ** File I/O service interface **
		 ********************************/

		Ftruncate_result ftruncate(Vfs::Vfs_handle *, file_size) override
		{
			return FTRUNCATE_OK;
		}
};


class Vfs_cbe_trust_anchor::Decrypt_file_system : public Vfs::Single_file_system
{
	private:

		Trust_anchor &_trust_anchor;

		struct Decrypt_handle : Single_vfs_handle
		{
			Trust_anchor &_trust_anchor;

			enum State { NONE, PENDING };
			State _state;

			Decrypt_handle(Directory_service &ds,
			               File_io_service   &fs,
			               Genode::Allocator &alloc,
			               Trust_anchor      &ta)
			:
				Single_vfs_handle { ds, fs, alloc, 0 },
				_trust_anchor { ta },
				_state        { State::NONE }
			{ }

			Read_result read(char *dst, file_size count,
			                 file_size &out_count) override
			{
				if (_state != State::PENDING) {
					return READ_ERR_IO;
				}

				_trust_anchor.execute();

				try {
					Trust_anchor::Complete_request const cr =
						_trust_anchor.complete_decrypt_key(dst, count);
					if (!cr.valid) {
						return READ_QUEUED;
					}

					_state = State::NONE;

					out_count = count;
					return cr.success ? READ_OK : READ_ERR_IO;
				} catch (...) {
					return READ_ERR_INVALID;
				}

				return READ_ERR_IO;
			}

			Write_result write(char const *src, file_size count,
			                   file_size &out_count) override
			{
				if (_state != State::NONE) {
					return WRITE_ERR_IO;
				}

				try {
					bool const ok =
						_trust_anchor.queue_decrypt_key(src, count);
					if (!ok) {
						return WRITE_ERR_IO;
					}
					_state = State::PENDING;
				} catch (...) {
					return WRITE_ERR_INVALID;
				}

				_trust_anchor.execute();
				out_count = count;
				return WRITE_OK;
			}

			bool read_ready() override { return true; }
		};

	public:

		Decrypt_file_system(Trust_anchor &ta)
		:
			Single_file_system { Node_type::TRANSACTIONAL_FILE, type_name(),
			                     Node_rwx::rw(), Xml_node("<decrypt/>") },
			_trust_anchor { ta }
		{ }

		static char const *type_name() { return "decrypt"; }

		char const *type() override { return type_name(); }

		/*********************************
		 ** Directory-service interface **
		 *********************************/

		Open_result open(char const  *path, unsigned,
		                 Vfs::Vfs_handle **out_handle,
		                 Genode::Allocator   &alloc) override
		{
			if (!_single_file(path))
				return OPEN_ERR_UNACCESSIBLE;

			try {
				*out_handle =
					new (alloc) Decrypt_handle(*this, *this, alloc,
					                           _trust_anchor);
				return OPEN_OK;
			}
			catch (Genode::Out_of_ram)  { return OPEN_ERR_OUT_OF_RAM; }
			catch (Genode::Out_of_caps) { return OPEN_ERR_OUT_OF_CAPS; }
		}

		Stat_result stat(char const *path, Stat &out) override
		{
			Stat_result result = Single_file_system::stat(path, out);
			return result;
		}

		/********************************
		 ** File I/O service interface **
		 ********************************/

		Ftruncate_result ftruncate(Vfs::Vfs_handle *, file_size) override
		{
			return FTRUNCATE_OK;
		}
};


class Vfs_cbe_trust_anchor::Initialize_file_system : public Vfs::Single_file_system
{
	private:

		Trust_anchor &_trust_anchor;

		struct Initialize_handle : Single_vfs_handle
		{
			Trust_anchor &_trust_anchor;

			enum class State { NONE, PENDING };
			State _state { State::NONE };

			bool _init_pending { false };

			Initialize_handle(Directory_service &ds,
			                  File_io_service   &fs,
			                  Genode::Allocator &alloc,
			                  Trust_anchor      &ta)
			:
				Single_vfs_handle { ds, fs, alloc, 0 },
				_trust_anchor     { ta }
			{ }

			Read_result read(char *, file_size, file_size &) override
			{
				if (_state != State::PENDING) {
					return READ_ERR_INVALID;
				}

				(void)_trust_anchor.execute();

				Trust_anchor::Complete_request const cr =
					_init_pending ? _trust_anchor.complete_queue_unlock()
					              : _trust_anchor.complete_queue_initialize();
				if (!cr.valid) {
					return READ_QUEUED;
				}

				_state        = State::NONE;
				_init_pending = false;

				return cr.success ? READ_OK : READ_ERR_IO;
			}

			Write_result write(char const *src, file_size count,
			                   file_size &out_count) override
			{
				if (_state != State::NONE) {
					return WRITE_ERR_INVALID;
				}

				_init_pending = _trust_anchor.initialized();

				bool const res = _init_pending ? _trust_anchor.queue_unlock(src, count)
				                               : _trust_anchor.queue_initialize(src, count);

				if (!res) {
					return WRITE_ERR_IO;
				}
				_state = State::PENDING;

				out_count = count;
				return WRITE_OK;
			}

			bool read_ready() override { return true; }
		};

	public:

		Initialize_file_system(Trust_anchor &ta)
		:
			Single_file_system { Node_type::TRANSACTIONAL_FILE, type_name(),
			                     Node_rwx::rw(), Xml_node("<initialize/>") },
			_trust_anchor      { ta }
		{ }

		static char const *type_name() { return "initialize"; }

		char const *type() override { return type_name(); }

		/*********************************
		 ** Directory-service interface **
		 *********************************/

		Open_result open(char const  *path, unsigned,
		                 Vfs::Vfs_handle **out_handle,
		                 Genode::Allocator   &alloc) override
		{
			if (!_single_file(path))

				return OPEN_ERR_UNACCESSIBLE;

			try {
				*out_handle =
					new (alloc) Initialize_handle(*this, *this, alloc,
					                              _trust_anchor);
				return OPEN_OK;
			}
			catch (Genode::Out_of_ram)  { return OPEN_ERR_OUT_OF_RAM; }
			catch (Genode::Out_of_caps) { return OPEN_ERR_OUT_OF_CAPS; }
		}

		Stat_result stat(char const *path, Stat &out) override
		{
			Stat_result result = Single_file_system::stat(path, out);
			return result;
		}

		/********************************
		 ** File I/O service interface **
		 ********************************/

		Ftruncate_result ftruncate(Vfs::Vfs_handle *, file_size) override
		{
			return FTRUNCATE_OK;
		}
};


struct Vfs_cbe_trust_anchor::Local_factory : File_system_factory
{
	Trust_anchor _trust_anchor;

	Decrypt_file_system      _decrypt_fs;
	Encrypt_file_system      _encrypt_fs;
	Generate_key_file_system _gen_key_fs;
	Hashsum_file_system      _hash_fs;
	Initialize_file_system   _init_fs;

	using Storage_path = String<256>;
	static Storage_path _storage_path(Xml_node const &node)
	{
		if (!node.has_attribute("storage_dir")) {
			error("mandatory 'storage_dir' attribute missing");
			struct Missing_storage_dir_attribute { };
			throw Missing_storage_dir_attribute();
		}
		return node.attribute_value("storage_dir", Storage_path());
	}

	Local_factory(Vfs::Env &vfs_env, Xml_node config)
	:
		_trust_anchor { vfs_env, _storage_path(config).string() },
		_decrypt_fs   { _trust_anchor },
		_encrypt_fs   { _trust_anchor },
		_gen_key_fs   { _trust_anchor },
		_hash_fs      { _trust_anchor },
		_init_fs      { _trust_anchor }
	{ }

	Vfs::File_system *create(Vfs::Env&, Xml_node node) override
	{
		if (node.has_type(Decrypt_file_system::type_name())) {
			return &_decrypt_fs;
		}

		if (node.has_type(Encrypt_file_system::type_name())) {
			return &_encrypt_fs;
		}

		if (node.has_type(Generate_key_file_system::type_name())) {
			return &_gen_key_fs;
		}

		if (node.has_type(Hashsum_file_system::type_name())) {
			return &_hash_fs;
		}

		if (node.has_type(Initialize_file_system::type_name())) {
			return &_init_fs;
		}

		return nullptr;
	}
};


class Vfs_cbe_trust_anchor::File_system : private Local_factory,
                                          public Vfs::Dir_file_system
{
	private:

		using Config = String<128>;

		static Config _config(Xml_node const &node)
		{
			char buf[Config::capacity()] { };

			Xml_generator xml(buf, sizeof (buf), "dir", [&] () {
				xml.attribute("name", node.attribute_value("name", String<32>("")));

				xml.node("decrypt",      [&] () { });
				xml.node("encrypt",      [&] () { });
				xml.node("generate_key", [&] () { });
				xml.node("hashsum",      [&] () { });
				xml.node("initialize",   [&] () { });
			});

			return Config(Cstring(buf));
		}

	public:

		File_system(Vfs::Env &vfs_env, Genode::Xml_node node)
		:
			Local_factory        { vfs_env, node },
			Vfs::Dir_file_system { vfs_env, Xml_node(_config(node).string()), *this }
		{ }

		~File_system() { }
};


/**************************
 ** VFS plugin interface **
 **************************/

extern "C" Vfs::File_system_factory *vfs_file_system_factory(void)
{
	struct Factory : Vfs::File_system_factory
	{
		Vfs::File_system *create(Vfs::Env &vfs_env,
		                         Genode::Xml_node node) override
		{
			try {
				return new (vfs_env.alloc())
					Vfs_cbe_trust_anchor::File_system(vfs_env, node);

			} catch (...) {
				Genode::error("could not create 'cbe_trust_anchor'");
			}
			return nullptr;
		}
	};

	static Factory factory;
	return &factory;
}
