struct bit_file {
	FILE *file;
	unsigned char mask;
	int rack;
};

struct bit_file *open_input_bit_file(char *name);
struct bit_file *open_output_bit_file(char *name);
void output_bit(struct bit_file *bit_file, int bit);
void output_bits(struct bit_file *bit_file, unsigned long code, int count);
int input_bit(struct bit_file *bit_file);
unsigned long input_bits(struct bit_file *bit_file, int bit_count);
void close_input_bit_file(struct bit_file *bit_file);
void close_output_bit_file(struct bit_file *bit_file);
void file_print_binary(FILE * file, unsigned int code, int bits);
