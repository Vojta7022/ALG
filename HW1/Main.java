import java.io.*;

public class Main {
    // Fast byte-based scanner (non-negative ints are enough here)
    static final class FastScanner {
        private final InputStream in;
        private final byte[] buffer = new byte[1 << 16];
        private int ptr = 0, len = 0;

        FastScanner(InputStream is) { this.in = is; }

        private int read() throws IOException {
            if (ptr >= len) {
                len = in.read(buffer);
                ptr = 0;
                if (len <= 0) return -1;
            }
            return buffer[ptr++];
        }

        int nextInt() throws IOException {
            int c, x = 0;
            do { c = read(); } while (c <= ' ');          // skip whitespace
            // numbers are non-negative by problem statement
            while (c > ' ') {
                x = x * 10 + (c - '0');
                c = read();
            }
            return x;
        }
    }

    public static void main(String[] args) throws Exception {
        FastScanner fs = new FastScanner(System.in);

        int N = fs.nextInt();
        int K = fs.nextInt();
        int L = fs.nextInt();
        int S = fs.nextInt();

        final int W = N + 1;
        final int SZ = W * W; // <= ~9,006,001 for N=3000

        // Flattened (N+1) x (N+1) prefix tables; 0-initialized.
        int[] PF = new int[SZ]; // forests (value == 1)
        int[] PR = new int[SZ]; // rocks   (value == 2)

        // Row-major index
        final java.util.function.IntBinaryOperator IDX = (i, j) -> i * W + j;

        // Build prefix sums while reading the grid
        for (int i = 1; i <= N; ++i) {
            for (int j = 1; j <= N; ++j) {
                int v = fs.nextInt();
                int f = (v == 1) ? 1 : 0;
                int r = (v == 2) ? 1 : 0;

                int ij   = IDX.applyAsInt(i, j);
                int imj  = IDX.applyAsInt(i - 1, j);
                int ijm  = IDX.applyAsInt(i, j - 1);
                int imjm = IDX.applyAsInt(i - 1, j - 1);

                PF[ij] = f + PF[imj] + PF[ijm] - PF[imjm];
                PR[ij] = r + PR[imj] + PR[ijm] - PR[imjm];
            }
        }

        // Provide small inlined helpers for speed
        final class Rect {
            int sum(int[] P, int x1, int y1, int x2, int y2) {
                int a = P[IDX.applyAsInt(x2, y2)];
                int b = P[IDX.applyAsInt(x1 - 1, y2)];
                int c = P[IDX.applyAsInt(x2, y1 - 1)];
                int d = P[IDX.applyAsInt(x1 - 1, y1 - 1)];
                return a - b - c + d;
            }
        }
        Rect R = new Rect();

        final int C = K - 2 * L; // guaranteed >= 1
        int best = 0;

        // Enumerate all KxK placements by top-left (i,j)
        for (int i = 1; i + K - 1 <= N; ++i) {
            int i2  = i + K - 1;
            int ic1 = i + L;
            int ic2 = ic1 + C - 1;

            for (int j = 1; j + K - 1 <= N; ++j) {
                int j2 = j + K - 1;

                // forests in the whole KxK
                int forests = R.sum(PF, i, j, i2, j2);

                // rocks in central CxC (shifted by L)
                int jc1 = j + L;
                int jc2 = jc1 + C - 1;
                int rocks = R.sum(PR, ic1, jc1, ic2, jc2);

                if (rocks >= S && forests > best) best = forests;
            }
        }

        System.out.println(best);
    }
}
