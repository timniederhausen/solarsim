## Application: Solar System simulation

<img class="r-stretch" src="content/galaxy_crash.jpg">

Notes:
- I implemented a parallelizable problem using these different interfaces.
- An N-body simulation for solar system collision.
- This simulation consists of a time integration method, in this case the velocity verlet, and a method for calculating a body's acceleration:
---

##### naive Acceleration
$$
\begin{equation*}
\vec{a_i} = \sum_{i \ne j} G m_j \frac{\vec{r_j} - \vec{r_i}}{\left( \lvert\lvert\vec{r_j} - \vec{r_i}\rvert\rvert + \epsilon^2\right)^{\frac{3}{2}}} \\
\end{equation*}
$$

<div class="fragment">

##### Barnes-Hut
<object width="50%" data="content/barnes_hut.svg" type="image/svg+xml">
</object>
</div>

Notes:
- The first version uses a naive method, where we calculate each body's acc. based on the forces all other bodies exert on it => RT n to the power of 2
- The second version uses Barnes-Hut (tree-based algo) where we group distant bodies into virtual bodies => O(n log(n)).
---

## Architecture overview
<object width="100%" data="content/solarsim_architecture_en.svg" type="image/svg+xml">
</object>

Notes:
- How can this be parallelized: Here's a birds-eye view of a single parallel simulation step.
- The time integration algorithm consists of 2 sub-steps between which the acceleration is calculated.
- These substeps can be easily parallelized with a fork/join approach. During both substeps there are no deps between bodies.
- Parallelization for the acceleration calculation follows the same pattern.
- We calculate partial results in parallel and then merge them sequentially in the end.
