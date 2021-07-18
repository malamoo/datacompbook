#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitio.h"
#include "errhand.h"
#include "main.h"

void usage_exit(char *prog_name);
long file_size(char *name);
void print_ratios(char *input, char *output);

int main(int argc, char **argv)
{
	FILE *input;
	struct bit_file *output;

	if (argc < 3)
		usage_exit(argv[0]);
	input = fopen(argv[1], "rb");
	if (input == NULL)
		fatal_error("Error opening %s for input\n", argv[1]);
	output = open_output_bit_file(argv[2]);
	if (output == NULL)
		fatal_error("Error opening %s for output\n", argv[2]);
	printf("\nCompressing %s to %s\n", argv[1], argv[2]);
	printf("Using %s\n", compression_name);
	compress_file(input, output, argc - 3, argv + 3);
	close_output_bit_file(output);
	fclose(input);
	print_ratios(argv[1], argv[2]);
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

long file_size(char *name)
{
	long eof_ftell;
	FILE *file;

	file = fopen(name, "r");
	if (file == NULL)
		return 0;
	fseek(file, 0, SEEK_END);
	eof_ftell = ftell(file);
	fclose(file);
	return eof_ftell;
}

void print_ratios(char *input, char *output)
{
	long input_size;
	long output_size;
	int ratio;

	input_size = file_size(input);
	if (input_size == 0)
		input_size = 1;
	output_size = file_size(output);
	if (output_size == 0)
		output_size = 1;
	ratio = 100 - (output_size * 100 / input_size);
	printf("\nInput bytes: %ld\n", input_size);
	printf("Output bytes: %ld\n", output_size);
	printf("Compression ratio: %d%%\n", ratio);
}
