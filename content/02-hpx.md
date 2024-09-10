## Senders / Receivers (HPX)
![](content/stdexec_simple.svg) <!-- .element: width="100%" -->

Notes:
- Core concept: we have 'senders' that receive input values, perform some work on them, send output values futher along.
- senders can be chained together, when you are ready to run said chain, connect it to a single receiver.
- From an appdev's perspective, you mainly use `sender` APIs provided by the standard or other libraries.
- Writing a parallel application requires you to decompose it into a sequence / chain of senders that you then use with a receiver.
- Now, what's the purpose of the receiver?
---

## Senders / Receivers (HPX) - cont'
![](content/stdexec_full.svg) <!-- .element: width="100%" -->

Notes:
- Some of the flexibility of S/R comes from the fact that senders can be entirely decoupled from how or where they will be run.
- Such things all come from the Environment, which is only available once a 'sender' is connected to a 'receiver'.
- Allocator, default scheduler etc. are all part of the Environment.
- So in the end we can write our sender chains without having to know about schedulers or allocators.
- But how do we build such a sender chain?
---

## `sender` Algorithms
<!-- https://stackoverflow.com/questions/8230777/how-do-i-set-the-table-cell-widths-to-minimum-except-last-column -->
<table class="noborder">
<tr><td style="width: 1%">

- Sender Factories:
</td><td>

```cpp
auto s1 = ex::just(1.0f, "hello", std::vector<int>{});
auto s2 = ex::on(my_thread_pool_scheduler);
```
</td></tr>
<tr><td style="width: 1%">

- Sender Adaptors:
</td><td class="fragment">

```cpp
auto s3 = s2 | ex::then([] { return do_work(...); });
auto s4 = s3 | ex::transfer(my_other_scheduler);
```

```cpp
auto s5_reusable = ex::split(s4);
auto s5_1 = s5_reusable | ex::bulk(N, do_one_work_iteration);
auto s5_2 = s5_reusable | ex::let_value([] { return ex::just(3.0f); })
auto s6 = ex::when_all(s5_1, s5_2);
```
<!-- .element: class="fragment" -->

</td></tr>
<tr><td style="width: 1%">

- Consumers:
</td><td class="fragment">

```cpp
ex::start_detached(s1);
auto [res] = this_thread::sync_wait(s4).value();
```
</td></tr>
</table>

Notes:
- The proposal introduces three different types of sender algorithms:
- factory = initial element of a sender-chain
- e.g. just() where we just send a bunch of values, or on where we explicitly define the sched. for the next senders in the chain
- adaptors make up the middle of our sender chain.
- then() allows us to pass a function that recvs all prev sent values as params, the return values are sent further to the next snd.
- NEXT: other adaptors include split() that allow us to split our execution into multiple paths.
Unlike other senders, the sender returned by split can be used multiple times.
- We have bulk() for simple parallelization for a known number of iterations and let_value() for unwrapping other sender chains.
- when_all() acts as the counterpart to split, combining all sent values into a tuple.
- at the end, there's always a snd consumer: these connect the sender chain to a (possibly internal) receiver and launch the op
- e.g. start_detached when there's no computed result or we don't care about it
- or tt::sync_wait if we want to block the current thread waiting for the result
- where the actual...
