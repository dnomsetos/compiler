# Comparison of Memory Resources (Valgrind Metrics)

This document summarizes performance counters for three C++ memory resource configurations:
- `default resource`
- `unsynchronized_pool_resource`
- `monotonic_buffer_resource`

---

## Instruction and Data References

| Metric | Default | Unsynchronized Pool | Monotonic Buffer |
|--------|--------:|--------------------:|------------------:|
| I refs | 270,742,150 | 263,809,928 | 256,651,224 |
| D refs | 120,654,476 | 117,432,675 | 115,578,686 |
| LL refs | 5,045,101 | 4,973,145 | 4,960,269 |

---

## Instruction Cache Performance

| Metric | Default | Unsynchronized Pool | Monotonic Buffer |
|--------|--------:|--------------------:|------------------:|
| I1 misses | 2,439,256 | 2,353,247 | 2,365,599 |
| LLi misses | 123,590 | 133,024 | 133,064 |
| I1 miss rate | 0.90% | 0.89% | 0.92% |
| LLi miss rate | 0.05% | 0.05% | 0.05% |

---

## Data Cache Performance

| Metric | Default | Unsynchronized Pool | Monotonic Buffer |
|--------|--------:|--------------------:|------------------:|
| D1 misses | 2,605,845 | 2,619,898 | 2,594,670 |
| LLd misses | 229,102 | 245,827 | 247,550 |
| D1 miss rate | 2.2% | 2.2% | 2.2% |
| LLd miss rate | 0.2% | 0.2% | 0.2% |

---

## Branch Prediction

| Metric | Default | Unsynchronized Pool | Monotonic Buffer |
|--------|--------:|--------------------:|------------------:|
| Branches | 45,878,246 | 44,954,434 | 43,814,823 |
| Mispredicts | 3,571,048 | 3,626,399 | 3,564,479 |
| Mispred rate | 7.8% | 8.1% | 8.1% |

---

## Summary

- Cache miss rates are broadly similar across all allocators.
- `unsynchronized_pool_resource` shows slightly higher branch misprediction.
- `monotonic_buffer_resource` reduces instruction/data references but does not significantly improve cache behavior.
- Differences are marginal; performance impact is likely workload-dependent.
