#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_SYMBOLS 128
#define MAX_NODES (2 * MAX_SYMBOLS - 1)

struct node {
        unsigned int weight;
        struct node *left;
        struct node *right;
        struct symbol *symbol;
};

struct symbol {
        unsigned int code;
        unsigned int count;
        char ascii;
};

void printtree(const struct node *np, unsigned int indent);
void assigncodes(struct node *np, unsigned int code);
int comparenodes(const void *a, const void *b);

int main(void)
{
        int i;
        int j;
        int c;
        FILE *fp;
        struct symbol *symbols;
        struct node *nodes;

        fp = fopen("./src/source.txt", "r");
        if (fp == NULL) {
                printf("Error: failed to open file\n");
                exit(EXIT_FAILURE);
        }

        nodes = calloc(MAX_NODES, sizeof(struct node));
        symbols = calloc(MAX_SYMBOLS, sizeof(struct symbol));

        while ((c = fgetc(fp)) != EOF) {
                if (symbols[c].ascii == '\0')
                        symbols[c].ascii = (char)c;
                symbols[c].count++;
        }

        j = 0;
        for (i = 0; i < MAX_SYMBOLS; i++) {
                if (symbols[i].count > 0) {
                        nodes[j].symbol = &symbols[i];
                        nodes[j].weight = symbols[i].count;
                        j++;
                }
        }

        qsort(nodes, j, sizeof(struct node), comparenodes);

        i = 0;
        while (i + 1 < j) {
                nodes[j].weight = nodes[i].weight + nodes[i + 1].weight;
                nodes[j].left = &nodes[i];
                nodes[j].right = &nodes[i + 1];
                j++;
                i += 2;
                qsort(nodes + i, j - i, sizeof(struct node), comparenodes);
        }

        assigncodes(&nodes[j - 1], 0);
        printtree(&nodes[j - 1], 0);

        free(symbols);
        free(nodes);
        fclose(fp);

        return 0;
}

void assigncodes(struct node *np, unsigned int code)
{
        struct symbol *sp;

        if (np == NULL)
                return;

        sp = np->symbol;
        if (sp != NULL) {
                sp->code = code;
        }
        assigncodes(np->left, (code << 1) | 0);
        assigncodes(np->right, (code << 1) | 1);
}

void printtree(const struct node *np, unsigned int indent)
{
        if (np == NULL)
                return;

        printtree(np->right, indent + 8);
        printf("%*d", indent, np->weight);
        if (np->symbol != NULL) {
                printf("'%c'[%d]\n", np->symbol->ascii, np->symbol->code);
        } else {
                printf("\n");
        }
        printtree(np->left, indent + 8);
}

int comparenodes(const void *a, const void *b)
{
        const struct node *npa = (const struct node *) a;
        const struct node *npb = (const struct node *) b;

        return (npa->weight > npb->weight) - (npa->weight < npb->weight);
}