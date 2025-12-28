#include <stdio.h>
#include <stdlib.h>

#define BUFSZ (1<<20)

static unsigned char ibuf[BUFSZ];
static size_t ipos = 0, ilen = 0;

static inline int refill() {
    ipos = 0;
    ilen = fread(ibuf, 1, BUFSZ, stdin);
    return (int)ilen;
}

static inline int next_int() {
    int c;
    // fetch first byte
    if (ipos >= ilen && !refill()) return 0;
    c = ibuf[ipos++];
    // skip non-digits (inputs here are non-negative)
    while ((c < '0' || c > '9')) {
        if (ipos >= ilen && !refill()) return 0;
        c = ibuf[ipos++];
    }
    int x = 0;
    while (c >= '0' && c <= '9') {
        x = x * 10 + (c - '0');
        if (ipos >= ilen && !refill()) break;
        c = ibuf[ipos++];
    }
    return x;
}

int main(void) {
    int N = next_int();
    int K = next_int();
    int L = next_int();
    int S = next_int();
    if (N <= 0) return 0;

    const int W = N + 1;
    const size_t SZ = (size_t)W * (size_t)W;

    // allocate and zero-initialise prefix tables
    int *PF = (int*)calloc(SZ, sizeof(int)); // forests (value == 1)
    int *PR = (int*)calloc(SZ, sizeof(int)); // rocks   (value == 2)
    if (!PF || !PR) {
        fprintf(stderr, "Allocation failed\n");
        free(PF); free(PR);
        return 1;
    }

    #define IDX(i,j) ((size_t)(i) * (size_t)W + (size_t)(j))

    // Build prefix sums while reading input
    for (int i = 1; i <= N; ++i) {
        for (int j = 1; j <= N; ++j) {
            int v = next_int();
            int f = (v == 1);
            int r = (v == 2);
            PF[IDX(i,j)] = f + PF[IDX(i-1,j)] + PF[IDX(i,j-1)] - PF[IDX(i-1,j-1)];
            PR[IDX(i,j)] = r + PR[IDX(i-1,j)] + PR[IDX(i,j-1)] - PR[IDX(i-1,j-1)];
        }
    }

    // helper: sum over [x1..x2] x [y1..y2], 1-based, inclusive
    #define RECTSUM(P,x1,y1,x2,y2) ( \
        (P)[IDX((x2),(y2))] - (P)[IDX((x1)-1,(y2))] - \
        (P)[IDX((x2),(y1)-1)] + (P)[IDX((x1)-1,(y1)-1)] )

    const int C = K - 2 * L; // central square size (>=1 guaranteed)
    int best = 0;

    // enumerate all KxK placements by top-left corner (i,j)
    for (int i = 1; i + K - 1 <= N; ++i) {
        int i2  = i + K - 1;
        int ic1 = i + L;
        int ic2 = ic1 + C - 1;
        for (int j = 1; j + K - 1 <= N; ++j) {
            int j2  = j + K - 1;

            // forests in whole KxK
            int forests = RECTSUM(PF, i, j, i2, j2);

            // rocks in central CxC (offset by L from each side)
            int jc1 = j + L;
            int jc2 = jc1 + C - 1;
            int rocks = RECTSUM(PR, ic1, jc1, ic2, jc2);

            if (rocks >= S && forests > best) best = forests;
        }
    }

    printf("%d\n", best);

    free(PF);
    free(PR);
    return 0;
}
