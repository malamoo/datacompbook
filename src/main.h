void compress_file(FILE * input, struct bit_file *output, int argc,
		   char **argv);
void expand_file(struct bit_file *input, FILE * output, int argc,
		 char **argv);

extern char *usage;
extern char *compression_name;
