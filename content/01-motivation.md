<img class="r-stretch" src="content/server_cpus.jpg">
<small>from: <a href="https://www.servethehome.com/amd-and-intel-2p-server-core-count-growth-2010-2022">https://www.servethehome.com/amd-and-intel-2p-server-core-count-growth-2010-2022/</a></small>

Notes:
- What we've seen in the recent years is that CPU clock speeds remain roughly the same, peaking at around 4Ghz for consumer desktop or normal server hardware.
- The number of CPU cores however is steadily increasing.
- Because of this it becomes more and more important to develop parallelizible software.
- How can we do that in C++?
---

<div class="r-stack">
<object class="fragment current-visible" width="100%" data="content/cpp_parallelism.svg" type="image/svg+xml">
</object>
<object class="fragment current-visible" width="100%" data="content/cpp_parallelism_p2300.svg" type="image/svg+xml">
</object>
<object class="fragment current-visible" width="100%" data="content/cpp_parallelism_thirdparty.svg" type="image/svg+xml">
</object>
<object class="fragment current-visible" width="100%" data="content/cpp_parallelism_full.svg" type="image/svg+xml">
</object>
</div>

Notes:
- We have the C++ StdLib that has support for threads, mutexes and other primitives since C++11
- Since C++17 we've got the parallel algorithms. They allow you to easily parallelize some of C++'s standard algorithms.
- However, both suffer from various issues: The basics are too low-level
- for example if you want to run your work on a thread pool, you'd have to implement that yourself.
- The parallel algorithms are not flexible enough: You cannot specify on which threads they run, or even the number of threads.

- That's why there's been multiple proposals to add a more generic and flexible way for parallel and async. exec to the standard:
- One of these proposals, P2300, called Senders / Receivers is now part of the next standard's draft.

- To fill the gap left by the standard, multiple third-party libraries have been developed over time.
- Our focus here lies on HPX: It has a rich feature set, ranging from shared memory parallelization to distributed computing.
- The design of its API is similar to the StdLib's, the difference being HPX offering various interfaces that haven't been standardized.
- Now, HPX also has its own version of P2300, so the question is:

- Motivation: How parallelization mechanisms from P2300 compare to the older future-based implementation in HPX?
  (in terms of: overhead / scalability, code quality / ease of use)
- How does it compare to S/R's ref impl?
- Short teaser: Small project developed for this, with strong/weak scaling benchmarks as a result
