# rulesmap
This is an implementation of a rule engine that uses a trie to achieve a near constant time lookup regardless of the number of rules provided.

Features:
1- Wildcards using * for zero or more chars and ? for a single character match
2- When conflicting rules are specified using wildcards, the order of rule creation is used to resolve the conflict. Optionally, a priority can be added.
3- Support for any alphabet

Missing Features (aka TODO):
1- Compression: Limits the number of nodes and reduces memory consumption
2- No recursion: For even faster lookup time
3- Contiguous memory allocation for lookup keys. There are many ways to arrange the keys in a contiguous memory, perhaps implement all of them and let the user
chose an allocator.
