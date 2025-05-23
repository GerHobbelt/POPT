#include <errno.h>

#include "system.h"

int main (int argc, const char **argv) {
    const char *out;
    int newargc, j, f, ret;
    const char **newargv;
    FILE *fp;

    if (argc == 1) {
	printf ("usage: test-popt file_1 file_2 ...\n");
	printf ("you may specify many files\n");
	exit (1);
    }

    for (f = 1; f < argc; f++) {
	fp = fopen (argv[f], "r");
	if (fp == NULL) {
	    printf ("cannot read file %s.  errno=%s\n", argv[f],
		strerror(errno));
	    continue;
	}

	ret = poptConfigFileToString (fp, &out, 0);
	if (ret != 0) {
	    printf ("cannot parse %s. ret=%d\n", argv[f], ret);
	    continue;
	}

	printf ("single string: '%s'\n", out);

	ret = poptParseArgvString (out, &newargc, &newargv);
	if (ret != 0) {
	    printf ("cannot parse %s. ret=%d\n", out, ret);
	    continue;
	}

	printf ("popt array: size=%d\n", newargc);
	for (j = 0; j < newargc; j++)
	    printf ("'%s'\n", newargv[j]);

	printf ("\n");
	free(newargv);
	free((void *)out);
	fclose (fp);
    }
    return 0;
}
