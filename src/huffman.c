#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_SYMBOLS 128
#define MAX_NODES (2 * MAX_SYMBOLS - 1)

struct node {
        unsigned int weight;
        struct node *left;
        struct node *right;
        char ascii;
};

struct coding {
        unsigned int code;
        unsigned int len;
};

void printtree(const struct node *np, const struct coding *cds,
               unsigned int indent);
void assigncodes(const struct node *np, struct coding *cds, unsigned int code,
                 unsigned int len);
int comparenodes(const void *a, const void *b);
void compress(const struct coding *cds, FILE *infp, FILE *compfp);
void decompress(const struct node *rootp, FILE *compfp, FILE *decompfp);

int main(void)
{
        int i;
        int j;
        int c;
        FILE *infp;
        FILE *compfp;
        FILE *decompfp;
        struct node *ns;
        struct coding *cds;

        infp = fopen("./src/source.txt", "r");
        if (infp == NULL) {
                printf("Error: failed to open input file\n");
                exit(EXIT_FAILURE);
        }

        ns = calloc(MAX_NODES, sizeof(struct node));
        cds = calloc(MAX_SYMBOLS, sizeof(struct coding));

        while ((c = fgetc(infp)) != EOF) {
                if (ns[c].ascii == '\0')
                        ns[c].ascii = (char)c;
                ns[c].weight++;
        }

        fclose(infp);

        qsort(ns, MAX_SYMBOLS, sizeof(struct node), comparenodes);

        i = 0;
        j = MAX_SYMBOLS;
        while (i + 1 < j) {
                if (ns[i].weight == 0) {
                        i++;
                        continue;
                }
                ns[j].weight = ns[i].weight + ns[i + 1].weight;
                ns[j].left = &ns[i];
                ns[j].right = &ns[i + 1];
                j++;
                i += 2;
                qsort(ns + i, j - i, sizeof(struct node), comparenodes);
        }

        assigncodes(&ns[j - 1], cds, 0, 0);
//        printtree(&ns[j - 1], cds, 0);

        infp = fopen("./src/source.txt", "r");
        if (infp == NULL) {
                printf("Error: failed to open input file\n");
                exit(EXIT_FAILURE);
        }

        compfp = fopen("./src/compressed.txt", "w");
        if (compfp == NULL) {
                printf("Error: failed to open output file\n");
                exit(EXIT_FAILURE);
        }

        compress(cds, infp, compfp);

        fclose(compfp);

        compfp = fopen("./src/compressed.txt", "r");
        if (compfp == NULL) {
                printf("Error: failed to open output file\n");
                exit(EXIT_FAILURE);
        }

        decompfp = fopen("./src/decompressed.txt", "w");
        if (decompfp == NULL) {
                printf("Error: failed to open output file\n");
                exit(EXIT_FAILURE);
        }

        decompress(&ns[j - 1], compfp, decompfp);

        free(ns);
        free(cds);
        fclose(infp);
        fclose(compfp);
        fclose(decompfp);

        return 0;
}

void decompress(const struct node *rootp, FILE *compfp, FILE *decompfp)
{
        int c;
        unsigned char buffer;
        unsigned char mask;
        const struct node *np;

        mask = 0x80;
        np = rootp;
        while ((c = fgetc(compfp)) != EOF) {
                buffer = c;
                while (mask != 0x0) {
                        if ((mask & buffer) != 0)
                                np = np->right;
                        else
                                np = np->left;
                        if (np->ascii != '\0') {
                                fwrite(&np->ascii, sizeof(unsigned char), 1,
                                       decompfp);
                                np = rootp;
                        }
                        mask >>= 1;
                }
                mask = 0x80;
        }

        printf("\n");
}

void compress(const struct coding *cds, FILE *infp, FILE *compfp)
{
        unsigned int code;
        unsigned int len;
        unsigned int offset;
        unsigned int align;
        unsigned char *bp;
        unsigned char buffer;
        unsigned char mask;
        unsigned char bit;
        unsigned int pos;
        int i;
        int c;

        pos = 0;
        buffer = 0x0;
        while ((c = fgetc(infp)) != EOF) {
                code = cds[c].code;
                len = cds[c].len;

                offset = len / 8;
                align = 8 - (len % 8);

                code <<= align;
                bp = (unsigned char *)&code + offset;
                mask = 0x80;
                for (i = 0; i < len; i++) {
                        bit = mask & *bp;
                        bit >>= pos;
                        buffer |= bit;
                        pos++;
                        code <<= 1;
                        if (pos > 7) {
                                fwrite(&buffer, sizeof(unsigned char), 1,
                                       compfp);
                                buffer = 0x0;
                                pos = 0;
                        }
                }
        }

        fwrite(&buffer, sizeof(unsigned char), 1, compfp);
}

void assigncodes(const struct node *np, struct coding *cds, unsigned int code,
                 unsigned int len)
{
        if (np == NULL) {
                return;
        }

        if (np->ascii != '\0') {
                cds[np->ascii].code = code;
                cds[np->ascii].len = len;
        }

        len++;
        assigncodes(np->left, cds, (code << 1), len);
        assigncodes(np->right, cds, (code << 1) | 0x1, len);
}

void printtree(const struct node *np, const struct coding *cds,
               unsigned int indent)
{
        if (np == NULL)
                return;

        printtree(np->right, cds, indent + 8);
        printf("%*d", indent, np->weight);
        if (np->ascii != '\0') {
                printf("'%c'[0x%x] {%d}\n", np->ascii, cds[np->ascii].code,
                       cds[np->ascii].len);
        } else {
                printf("\n");
        }
        printtree(np->left, cds, indent + 8);
}

int comparenodes(const void *a, const void *b)
{
        const struct node *npa;
        const struct node *npb;

        npa = (const struct node *)a;
        npb = (const struct node *)b;

        return (npa->weight > npb->weight) - (npa->weight < npb->weight);
}