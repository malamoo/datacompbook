#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitio.h"
#include "errhand.h"
#include "main.h"

void usage_exit(char *prog_name);

int main(int argc, char **argv)
{
	FILE *output;
	struct bit_file *input;

	if (argc < 3)
		usage_exit(argv[0]);
	input = open_input_bit_file(argv[1]);
	if (input == NULL)
		fatal_error("Error opening %s for input\n", argv[1]);
	output = fopen(argv[2], "wb");
	if (output == NULL)
		fatal_error("Error opening %s for output\n", argv[2]);
	printf("\nExpanding %s to %s\n", argv[1], argv[2]);
	printf("Using %s\n", compression_name);
	expand_file(input, output, argc - 3, argv + 3);
	close_input_bit_file(input);
	fclose(output);
	printf("\n");
	return 0;
}

void usage_exit(char *prog_name)
{
	char *short_name;
	char *extension;

	short_name = strrchr(prog_name, '\\');
	if (short_name == NULL)
		short_name = strrchr(prog_name, '/');
	if (short_name != NULL)
		short_name++;
	else
		short_name = prog_name;
	extension = strrchr(short_name, '.');
	if (extension != NULL)
		*extension = '\0';
	printf("\nUsage: %s %s\n", short_name, usage);
	exit(EXIT_FAILURE);
}
