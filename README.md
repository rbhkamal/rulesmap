# rulesmap
When trying to matching a substring against a very large list of strings, may approaches can be taken (like binary search or rolling hash... etc). But all of these methods break down when matching against wild card patterns. This is where rulesmap comes in. It provides near constant time lookup regardless of the number of patters that you are matching. It accomplishes this via a modified Trie. The implementation currently is rough and is just a proof of concept but it is fully functional.

# Features:

- Near constant lookup time of any number of rules
- Wildcards support that can be easily extended. Currently * matches zero or more chars and ? for a single character.
- Priority. When conflicting rules are specified using wildcards, the order of rule creation is used to resolve the conflict. Optionally, a priority can be added.
- Supports the entire Unicode standard.

# TODO:
- Compression: Limits the number of nodes and reduces memory consumption
- No recursion: For even faster lookup time
- Eliminate STL and implement a contiguous memory allocation for lookup keys. There are many ways to arrange the keys in a contiguous memory, perhaps implement all of them and let the user chose an allocator.
