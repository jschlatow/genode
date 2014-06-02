/*
 * \brief  Support to link libraries statically supposed to be dynamic
 * \author Alexander Boettcher
 * \date   2014-05-13
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <util/string.h>

/* VirtualBox includes */
#include <iprt/err.h>
#include <iprt/ldr.h>
#include <VBox/hgcmsvc.h>

extern "C" {

DECLCALLBACK(DECLEXPORT(int)) VBoxHGCMSvcLoad (VBOXHGCMSVCFNTABLE *ptable);


static struct shared {
	const char * name;
	const char * symbol;
	void * func;
} shared[] = { "/VBoxSharedFolders", VBOX_HGCM_SVCLOAD_NAME, (void *)VBoxHGCMSvcLoad };


int RTLdrLoad(const char *pszFilename, PRTLDRMOD phLdrMod)
{
	for (unsigned i = 0; i < sizeof(shared) / sizeof(shared[0]); i++) {
		if (Genode::strcmp(shared[i].name, pszFilename))
			continue;

		*phLdrMod = reinterpret_cast<RTLDRMOD>(&shared[i]);
		return VINF_SUCCESS;
	}

	PERR("shared library '%s' not supported", pszFilename);
	return VERR_NOT_SUPPORTED;
}


int RTLdrGetSymbol(RTLDRMOD hLdrMod, const char *pszSymbol, void **ppvValue) 
{
	
	struct shared * library = reinterpret_cast<struct shared *>(hLdrMod);

	if (!(shared <= library &&
	      library < shared + sizeof(shared) / sizeof(shared[0]))) {

		PERR("shared library handle %p unknown - symbol looked for '%s'",
		     hLdrMod, pszSymbol); 

		return VERR_NOT_SUPPORTED;
	}

	if (Genode::strcmp(pszSymbol, library->symbol)) {
		PERR("shared library '%s' does not provide symbol '%s'",
		     library->name, pszSymbol);

		return VERR_NOT_SUPPORTED;
	}

	*ppvValue = library->func;

	return VINF_SUCCESS;
}

}
