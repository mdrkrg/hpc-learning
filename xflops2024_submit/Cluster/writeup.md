Cluster
===

I'm afraid of algorithm.

## The algorithm

I've read solutions to NOIP 2012 King's Game. The OIers provided a very solid solution, though their code styles are far from satisfactory. To bring their idea to Cluster, here's some small bit of explanation:

Suppose we take two servers in the middle, say `server[i]` and `server[i + 1]`. The maximum risk of all the servers above them is independent of their order, so we can just examine whether we can swap their order to get a lower risk.

```
long long risk_before_swap = max(
    risk_max_prev,
    sum_weight_before - servers[i].capacity,
    sum_weight_before + servers[i].weight - servers[i + 1].capacity
);

long long risk_after_swap = max(
    risk_max_prev,
    sum_weight_before - servers[i + 1].capacity,
    sum_weight_before + servers[i + 1].weight - servers[i].capacity
);
```

We should look closer at the operands of `max`.

The `risk_max_prev` is independent of the swap, so we can ignore it. `sum_weight_before` are the same for the other four, so we can also ignore it.

Let `a_cap = servers[i].capacity, a_wht = servers[i].weight`, `b_cap = servers[i + 1].capacity, b_wth = servers[i + 1].weight`, then `-a_cap < b_wht - a_cap` is guaranteed, it is also true for `a_wht - b_cap > -b_cap`.

If `risk_before_swap == sum_weight_before - a_cap`, then because `-b_cap < a_wht - b_cap < -a_cap < b_wht - b_cap`, no swap is needed. This is also true when `risk_after_swap == sum_weight_before - b_cap`. We only need to swap when `a_wht + a_cap > b_wht + b_cap`, so this is our compare function.

After we sort the array of servers this way, the bottom server will contain the least maximum risk of collapsing.

Great, now parallelize the program.

## Parallelization

The `sort` must be parallelized. I used a merge sort from stackoverflow.

However, the way I implemented it is worse than serial implementation.

The parallelization of the last part of loop is a bit harder, I might or might not come back to that.

