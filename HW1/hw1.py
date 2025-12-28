import sys
from array import array

# -------- fast byte parser for non-negative ints --------
data = sys.stdin.buffer.read()
n = len(data)
i = 0

def next_int():
    global i
    # skip non-digits
    while i < n and data[i] <= 32:
        i += 1
    if i >= n:
        return None
    x = 0
    while i < n:
        c = data[i]
        if 48 <= c <= 57:  # '0'..'9'
            x = x * 10 + (c - 48)
            i += 1
        else:
            break
    # skip trailing sep (optional)
    while i < n and data[i] > 32:
        i += 1
    return x

# -------- read header --------
N = next_int()
K = next_int()
L = next_int()
S = next_int()

W = N + 1
SZ = W * W

# 2D prefix sums (row-major, 1D)
PF = array('I', [0]) * SZ  # forests == 1
PR = array('I', [0]) * SZ  # rocks   == 2

# helper to avoid attribute lookups in loops
PF_buf, PR_buf = PF, PR
Wloc = W

# -------- build prefix tables on the fly --------
for r in range(1, N + 1):
    base = r * Wloc
    base_up = (r - 1) * Wloc
    for c in range(1, N + 1):
        v = next_int()
        f = 1 if v == 1 else 0
        rck = 1 if v == 2 else 0
        idx = base + c
        PF_buf[idx] = f + PF_buf[base_up + c] + PF_buf[idx - 1] - PF_buf[base_up + c - 1]
        PR_buf[idx] = rck + PR_buf[base_up + c] + PR_buf[idx - 1] - PR_buf[base_up + c - 1]

# -------- rectangle sum via prefix table (1-based, inclusive) --------
def rect_sum(P, x1, y1, x2, y2):
    Wl = Wloc
    return (P[x2 * Wl + y2]
            - P[(x1 - 1) * Wl + y2]
            - P[x2 * Wl + (y1 - 1)]
            + P[(x1 - 1) * Wl + (y1 - 1)])

C = K - 2 * L  # central square size (>= 1)
best = 0

# -------- enumerate all KxK placements --------
for i_top in range(1, N - K + 2):
    i_bot = i_top + K - 1
    ic1 = i_top + L
    ic2 = ic1 + C - 1

    for j_left in range(1, N - K + 2):
        j_right = j_left + K - 1

        forests = rect_sum(PF, i_top, j_left, i_bot, j_right)

        jc1 = j_left + L
        jc2 = jc1 + C - 1
        rocks = rect_sum(PR, ic1, jc1, ic2, jc2)

        if rocks >= S and forests > best:
            best = forests

print(best)
