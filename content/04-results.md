## Results

- Implementation as `sender` adaptor &rarr; lots of boilerplate code
- Future chaining is simple, but offers fewer customization points.
- HPX's S/R implementation has small (but hard to debug) incompatibilities with the proposal

Notes:
- First the results on code quality and ease of use:
  Writing correct sender adaptors requires lots of boilerplate code.
  (e.g. Adapters must have multiple overloads according to the proposal)
- For future-chaining: Customization points must be part of the API if futures are used (e.g. ExecutionPolicy overloads).
  With S/R this can happen when connecting to a receiver that has these properties set.
- Now I have to worry about how to realize customization points, instead of having a standard framework for that - the recv's env.
---

- HPX v1.10.0 on ipvs-epyc1 (2x AMD EPYC 7742 - 128 cores, 2TB DDR4 RAM)
- Dataset: scenario1_state_vectors.csv (19054 celestial bodies) using Barnes-Hut

<object width="55%" data="content/strong_scaling.svg" type="image/svg+xml">
</object>

Notes:
- Now on to the benchmarks, I'll focus on the Barnes-Hut versions.
- All Benchm. done with curr. released HPX on ipvs-epyc1. Dataset from IPVS.
- The graphs show the mean execution times over 15 iterations, with error bars, that are extremely small, representing stddev.
- What we can see: stdexec senders offer better perf in the beginning, their simple threadpool seems to outperform HPX there.
- HPX futures slightly better later on (8+ threads)
- HPX S/R remains consistently behind
---

<object width="55%" data="content/weak_scaling.svg" type="image/svg+xml">
</object>

Notes:
- Dataset is scaled so even if we add more threads, the work per-thread remains the same
- same as above actually
- $(n * log(n)) / threads == (n_0 * log(n_0))$
- $n = n_0 * threads log(n_0) / W(n_0 * threads log(n_0))$
