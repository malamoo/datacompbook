#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bitio.h"
#include "errhand.h"
#include "main.h"

struct tree_node {
	unsigned int count;
	unsigned int saved_count;
	int child_0;
	int child_1;
};

struct code {
	unsigned int code;
	int code_bits;
};

#define END_OF_STREAM 256

void count_bytes(FILE * input, unsigned long *long_counts);
void scale_counts(unsigned long *long_counts, struct tree_node *nodes);
int build_tree(struct tree_node *nodes);
void convert_tree_to_code(struct tree_node *nodes, struct code *codes,
			  unsigned int code_so_far, int bits, int node);
void output_counts(struct bit_file *output, struct tree_node *nodes);
void input_counts(struct bit_file *input, struct tree_node *nodes);
void print_model(struct tree_node *nodes, struct code *codes);
void compress_data(FILE * input, struct bit_file *output,
		   struct code *codes);
void expand_data(struct bit_file *input, FILE * output,
		 struct tree_node *nodes, int root_node);
void print_char(int c);

char *compression_name = "static order 0 model with Huffman coding";
char *usage =
    "infile outfile [-d]\n\n Specifying -d will dump the modelling data\n";

void compress_file(FILE * input, struct bit_file *output, int argc,
		   char **argv)
{
	unsigned long *counts;
	struct tree_node *nodes;
	struct code *codes;
	int root_node;

	counts = calloc(256, sizeof(unsigned long));
	if (counts == NULL)
		fatal_error("Error allocating counts array\n");
	nodes = calloc(514, sizeof(struct tree_node));
	if (nodes == NULL)
		fatal_error("Error allocating nodes array\n");
	codes = calloc(257, sizeof(struct code));
	if (codes == NULL)
		fatal_error("Error allocating codes array\n");
	count_bytes(input, counts);
	scale_counts(counts, nodes);
	output_counts(output, nodes);
	root_node = build_tree(nodes);
	convert_tree_to_code(nodes, codes, 0, 0, root_node);
	if (argc > 0 && strcmp(argv[0], "-d") == 0)
		print_model(nodes, codes);
	compress_data(input, output, codes);
	free(counts);
	free(nodes);
	free(codes);
}

void expand_file(struct bit_file *input, FILE * output, int argc,
		 char **argv)
{
	struct tree_node *nodes;
	int root_node;

	nodes = calloc(514, sizeof(struct tree_node));
	if (nodes == NULL)
		fatal_error("Error allocating nodes array\n");
	input_counts(input, nodes);
	root_node = build_tree(nodes);
	if (argc > 0 && strcmp(argv[0], "-d") == 0)
		print_model(nodes, 0);
	expand_data(input, output, nodes, root_node);
	free(nodes);
}

void output_counts(struct bit_file *output, struct tree_node *nodes)
{
	int first;
	int last;
	int next;
	int i;

	first = 0;
	while (first < 255 && nodes[first].count == 0)
		first++;
	for (; first < 256; first = next) {
		last = first + 1;
		for (;;) {
			for (; last < 256; last++)
				if (nodes[last].count == 0)
					break;
			last--;
			for (next = last + 1; next < 256; next++)
				if (nodes[next].count != 0)
					break;
			if (next > 255)
				break;
			if ((next - last) > 3)
				break;
			last = next;
		}
		if (putc(first, output->file) != first)
			fatal_error("Error writing byte counts\n");
		if (putc(last, output->file) != last)
			fatal_error("Error writing byte counts\n");
		for (i = first; i <= last; i++) {
			if (putc(nodes[i].count, output->file) !=
			    nodes[i].count)
				fatal_error("Error writing byte counts\n");
		}
	}
	if (putc(0, output->file) != 0)
		fatal_error("Error writing byte counts\n");
}

void input_counts(struct bit_file *input, struct tree_node *nodes)
{
	int first;
	int last;
	int i;
	int c;

	for (i = 0; i < 256; i++)
		nodes[i].count = 0;
	if ((first = getc(input->file)) == EOF)
		fatal_error("Error reading byte counts\n");
	if ((last = getc(input->file)) == EOF)
		fatal_error("Error reading byte counts\n");
	for (;;) {
		for (i = first; i <= last; i++)
			if ((c = getc(input->file)) == EOF)
				fatal_error("Error reading byte counts\n");
			else
				nodes[i].count = c;
		if ((first = getc(input->file)) == EOF)
			fatal_error("Error reading byte counts\n");
		if (first == 0)
			break;
		if ((last = getc(input->file)) == EOF)
			fatal_error("Error reading byte counts\n");
	}
	nodes[END_OF_STREAM].count = 1;
}

void count_bytes(FILE * input, unsigned long *counts)
{
	long input_marker;
	int c;

	input_marker = ftell(input);
	while ((c = getc(input)) != EOF)
		counts[c]++;
	fseek(input, input_marker, SEEK_SET);
}

void scale_counts(unsigned long *counts, struct tree_node *nodes)
{
	unsigned long max_count;
	int i;

	max_count = 0;
	for (i = 0; i < 256; i++)
		if (counts[i] > max_count)
			max_count = counts[i];
	if (max_count == 0) {
		counts[0] = 1;
		max_count = 1;
	}
	max_count = max_count / 255;
	max_count = max_count + 1;
	for (i = 0; i < 256; i++) {
		nodes[i].count = counts[i] / max_count;
		if (nodes[i].count == 0 && counts[i] != 0)
			nodes[i].count = 1;
	}
	nodes[END_OF_STREAM].count = 1;
}

int build_tree(struct tree_node *nodes)
{
	int next_free;
	int i;
	int min_1;
	int min_2;

	nodes[513].count = 0xffff;
	for (next_free = END_OF_STREAM + 1;; next_free++) {
		min_1 = 513;
		min_2 = 513;
		for (i = 0; i < next_free; i++) {
			if (nodes[i].count != 0) {
				if (nodes[i].count < nodes[min_1].count) {
					min_2 = min_1;
					min_1 = i;
				} else if (nodes[i].count <
					   nodes[min_2].count) {
					min_2 = i;
				}
			}
		}
		if (min_2 == 513)
			break;
		nodes[next_free].count =
		    nodes[min_1].count + nodes[min_2].count;
		nodes[min_1].saved_count = nodes[min_1].count;
		nodes[min_1].count = 0;
		nodes[min_2].saved_count = nodes[min_2].count;
		nodes[min_2].count = 0;
		nodes[next_free].child_0 = min_1;
		nodes[next_free].child_1 = min_2;
	}
	next_free--;
	nodes[next_free].saved_count = nodes[next_free].count;
	return next_free;
}

void convert_tree_to_code(struct tree_node *nodes, struct code *codes,
			  unsigned int code_so_far, int bits, int node)
{
	if (node <= END_OF_STREAM) {
		codes[node].code = code_so_far;
		codes[node].code_bits = bits;
		return;
	}
	code_so_far <<= 1;
	bits++;
	convert_tree_to_code(nodes, codes, code_so_far, bits,
			     nodes[node].child_0);
	convert_tree_to_code(nodes, codes, code_so_far | 1, bits,
			     nodes[node].child_1);
}

void print_model(struct tree_node *nodes, struct code *codes)
{
	int i;

	for (i = 0; i < 513; i++) {
		if (nodes[i].saved_count != 0) {
			printf("node=");
			print_char(i);
			printf(" count=%3d", nodes[i].saved_count);
			printf(" child_0=");
			print_char(nodes[i].child_0);
			printf(" child_1=");
			print_char(nodes[i].child_1);
			if (codes && i <= END_OF_STREAM) {
				printf(" Huffman code=");
				file_print_binary(stdout, codes[i].code,
						  codes[i].code_bits);
			}
			printf("\n");
		}
	}
}

void print_char(int c)
{
	if (c >= 0x20 && c < 127)
		printf("'%c'", c);
	else
		printf("%3d", c);
}

void compress_data(FILE * input, struct bit_file *output,
		   struct code *codes)
{
	int c;

	while ((c = getc(input)) != EOF)
		output_bits(output, codes[c].code, codes[c].code_bits);
	output_bits(output, codes[END_OF_STREAM].code,
		    codes[END_OF_STREAM].code_bits);
}

void expand_data(struct bit_file *input, FILE * output,
		 struct tree_node *nodes, int root_node)
{
	int node;

	for (;;) {
		node = root_node;
		do {
			if (input_bit(input))
				node = nodes[node].child_1;
			else
				node = nodes[node].child_0;
		} while (node > END_OF_STREAM);
		if (node == END_OF_STREAM)
			break;
		if ((putc(node, output)) != node)
			fatal_error
			    ("Error trying to write byte to output\n");
	}
}
