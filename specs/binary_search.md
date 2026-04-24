# Binary Search / Lower Bound / Upper Bound Spec
## Base spec
### Goal:
A set of algorithms that perform partition searching. They find the first index where a monotonic predicate flips from true -> false.

They each return a `bool`, given `i`. `f(i) -> bool`

Where:
```
f(0)...f(k-1) = true
f(k)...f(n-1) = false
```

## lower_bound:
Searches for the first element in the partitioned range `[first, last)` which is **not** ordered before `value`.
`f(i) = comp(a[i], target)`

## upper_bound:
Searches for the first element in the partitioned range `[first, last)` which is ordered after `value`.
`f(i) = !comp(target, a[i]`