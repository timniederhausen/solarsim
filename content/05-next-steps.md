## Conclusion

- Performance differences rather small - HPX's futures slightly faster
- HPX's released S/R implementation incomplete and plagued by smaller issues

<div class="fragment">

- However: That will change in the next version
- PRs [#6431](https://github.com/STEllAR-GROUP/hpx/pull/6431) and [#6494](https://github.com/STEllAR-GROUP/hpx/pull/6494) (merged Aug 26 and Sep 3) integrate stdexec into HPX

</div>

Notes:
- In total, perf diff relatively small - futures slightly faster
- S/R in released HPX ver. incomplete, small but hard-to-debug issues such as: move-only senders when the proposal allows copying.
- But if you try and copy HPX senders you get unhelpful error messages about missing template specializations that do not point at the root cause.
- Now, all of that will change though: the next version of HPX will integrate with stdexec.
- Sadly, that work has been performed after this project was already done. We've got two PR that were recently merged into HPX.
- That add support for using a bundled stdexec with HPX's schedulers.
- For this presentation, I've re-run the benchmarks with the current HPX master branch and stdexec enabled.
---

### Comparison to HPX's master branch

<object width="55%" data="content/strong_scaling_master.svg" type="image/svg+xml">
</object>

Notes:
Now, what c. w. see here: Again, until 8 threads stdexec performs better
We can also see that HPX futures again perform better after 8 threads.
The big difference is that the stdexec based S/R in the HPX master branch perform better.
Also notable is that for stdexec in HPX we see this enormous spike at 128 threads.
I could consistently reproduce that. Given that the same code works fine on the released ver and produces better results for 128 threads there...
---

<object width="55%" data="content/weak_scaling_master.svg" type="image/svg+xml">
</object>

Notes:
