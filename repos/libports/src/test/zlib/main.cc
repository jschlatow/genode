#include <zlib.h>
#include <base/printf.h>

int main()
{

	/* test 1: segfault */
	unsigned char buffer[1024*1024];
	gzFile gzfile = gzopen("test.txt", "rb");
	int size = gzread(gzfile, buffer, sizeof(buffer));
	gzclose(gzfile);
	/* end of test 1 */

	/* test 2: grow buffer until segfault */
//	for (int i=1; i < 1024; i++) {
//		unsigned char buffer[i*1024];
//
//		gzFile gzfile = gzopen("test.txt", "rb");
//		gzread(gzfile, buffer, sizeof(buffer));
//		gzclose(gzfile);
//
//		PINF("gzread successful with buffer size %d", sizeof(buffer));
//	}
   /* end of test 2 */

	return 0;
}
