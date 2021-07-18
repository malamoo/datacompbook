#include <stdio.h>
#include <stdlib.h>
#include "bitio.h"
#include "errhand.h"

struct bit_file *open_input_bit_file(char *name)
{
	struct bit_file *bit_file;

	bit_file = calloc(1, sizeof(struct bit_file));
	if (bit_file == NULL)
		return bit_file;
	bit_file->mask = 0x80;
	bit_file->rack = 0;
	bit_file->file = fopen(name, "rb");
	return bit_file;
}

void close_input_bit_file(struct bit_file *bit_file)
{
	fclose(bit_file->file);
	free(bit_file);
}

struct bit_file *open_output_bit_file(char *name)
{
	struct bit_file *bit_file;

	bit_file = calloc(1, sizeof(struct bit_file));
	if (bit_file == NULL)
		return bit_file;
	bit_file->mask = 0x80;
	bit_file->rack = 0;
	bit_file->file = fopen(name, "wb");
	return bit_file;
}

void close_output_bit_file(struct bit_file *bit_file)
{
	if (bit_file->mask != 0x80) {
		if (putc(bit_file->rack, bit_file->file) != bit_file->rack) {
			printf("Error: failed to write bit_file\n");
			exit(EXIT_FAILURE);
		}
	}
	fclose(bit_file->file);
	free(bit_file);
}

void output_bit(struct bit_file *bit_file, int bit)
{
	if (bit)
		bit_file->rack |= bit_file->mask;
	bit_file->mask >>= 1;
	if (bit_file->mask == 0) {
		if (putc(bit_file->rack, bit_file->file) != bit_file->rack)
			fatal_error("Fatal error in output_bit\n");
		bit_file->rack = 0;
		bit_file->mask = 0x80;
	}
}

void output_bits(struct bit_file *bit_file, unsigned long code, int count)
{
	unsigned long mask;

	mask = 1 << (count - 1);
	while (mask != 0) {
		if (mask & code)
			bit_file->rack |= bit_file->mask;
		bit_file->mask >>= 1;
		if (bit_file->mask == 0) {
			if (putc(bit_file->rack, bit_file->file) !=
			    bit_file->rack)
				fatal_error
				    ("Fatal error in output_bits\n");
			bit_file->rack = 0;
			bit_file->mask = 0x80;
		}
		mask >>= 1;
	}
}

int input_bit(struct bit_file *bit_file)
{
	int value;

	if (bit_file->mask == 0x80) {
		bit_file->rack = getc(bit_file->file);
		if (bit_file->rack == EOF)
			fatal_error("Fatal error in output_bits\n");
	}
	value = bit_file->rack & bit_file->mask;
	bit_file->mask >>= 1;
	if (bit_file->mask == 0)
		bit_file->mask = 0x80;
	return value ? 1 : 0;
}

unsigned long input_bits(struct bit_file *bit_file, int bit_count)
{
	unsigned long mask;
	unsigned long return_value;

	mask = 1 << (bit_count - 1);
	return_value = 0;
	while (mask != 0) {
		if (bit_file->mask == 0x80) {
			bit_file->rack = getc(bit_file->file);
			if (bit_file->rack == EOF)
				fatal_error
				    ("Fatal error in intput_bits\n");
		}
		if (bit_file->rack & bit_file->mask)
			return_value |= mask;
		mask >>= 1;
		bit_file->mask >>= 1;
		if (bit_file->mask == 0)
			bit_file->mask = 0x80;
	}
	return return_value;
}

void file_print_binary(FILE * file, unsigned int code, int bits)
{
	unsigned int mask;

	mask = 1 << (bits - 1);
	while (mask != 0) {
		if (code & mask)
			fputc('1', file);
		else
			fputc('0', file);
		mask >>= 1;
	}
}
