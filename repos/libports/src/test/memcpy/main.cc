#include <stdio.h>
#include <string.h>

#include <base/attached_ram_dataspace.h>
#include <libc/component.h>
#include <util/string.h>

#include "memcpy.h"

using Genode::log;

__attribute__((aligned(4096)))
size_t memcpy_aligned(void *dst, const void *src, size_t size)
{
	unsigned char *d = (unsigned char *)dst, *s = (unsigned char *)src;

	/* check 32-byte alignment */
	size_t d_align = (size_t)d & 0x3;
	size_t s_align = (size_t)s & 0x3;

	/* only same word-alignments work for the following LDM/STM loop */
	if ((d_align & 0x3) != (s_align & 0x3))
		return size;

	for (; (size > 0) && (s_align > 0) && (s_align < 4);
		  s_align++, *d++ = *s++, size--);

	/* copy 32 byte chunks */
	for (; size >= 32; size -= 32) {
		asm volatile ("ldmia %0!, {r3 - r10} \n\t"
			           "stmia %1!, {r3 - r10} \n\t"
			           : "+r" (s), "+r" (d)
			           :: "r3","r4","r5","r6","r7","r8","r9","r10");
	}
	return size;
}

struct Bytewise_test {

	void start()    { log("start bytewise memcpy");    }
	void finished() { log("finished bytewise memcpy"); }

	void copy(void *dst, const void *src, size_t size) {
		bytewise_memcpy(dst, src, size); }
};

struct Genode_cpy_test {

	void start()    { log("start Genode memcpy");    }
	void finished() { log("finished Genode memcpy"); }

	void copy(void *dst, const void *src, size_t size) {
		memcpy_aligned(dst, src, size); }
};

struct Genode_set_test {

	void start()    { log("start Genode memset");    }
	void finished() { log("finished Genode memset"); }

	void copy(void *dst, const void *, size_t size) {
		Genode::memset(dst, 0, size); }
};

struct Libc_cpy_test {

	void start()    { log("start libc memcpy");    }
	void finished() { log("finished libc memcpy"); }

	void copy(void *dst, const void *src, size_t size) {
		memcpy(dst, src, size); }
};

struct Libc_set_test {

	void start()    { log("start libc memset");    }
	void finished() { log("finished libc memset"); }

	void copy(void *dst, const void *, size_t size) {
		memset(dst, 0, size); }
};

void Libc::Component::construct(Libc::Env &env)
{
	log("Memcpy testsuite started");

//	memcpy_test<Bytewise_test>();
//	memcpy_test<Genode_cpy_test>();
//	memcpy_test<Genode_set_test>();
//	memcpy_test<Libc_cpy_test>();
//	memcpy_test<Libc_set_test>();

	Genode::Attached_ram_dataspace cached_ds1(env.ram(), env.rm(),
	                                          BUF_SIZE+4096*2, Genode::CACHED);
	Genode::Attached_ram_dataspace cached_ds2(env.ram(), env.rm(),
	                                          BUF_SIZE+4096*2, Genode::CACHED);

	memcpy_test<Genode_cpy_test>((void*)((Genode::addr_t)cached_ds1.local_addr<void>()+     0),
	                             (void*)((Genode::addr_t)cached_ds2.local_addr<void>()+4096+0), BUF_SIZE);

	Genode::Attached_ram_dataspace uncached_ds(env.ram(), env.rm(),
	                                           BUF_SIZE, Genode::UNCACHED);

	memcpy_test<Genode_cpy_test>(uncached_ds.local_addr<void>(),
	                             nullptr, BUF_SIZE);
//	memcpy_test<Genode_cpy_test>(nullptr, uncached_ds.local_addr<void>(),
//	                             BUF_SIZE);

	log("Memcpy testsuite finished");
}
