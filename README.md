# information-compressor
> A file compressor using [Huffman Coding](https://en.wikipedia.org/wiki/Huffman_coding)

![GitHub last commit (branch)](https://img.shields.io/github/last-commit/0xKilty/information-compressor/main)

### Metrics
Using the `lorem` file
```bash
$ information-compressor -c ./data/lorem
Compressing ./data/lorem into ./data/lorem.dat
Entropy:           4.2378
Avg. Code Length:  6.89744
Original:          ./data/lorem       2088 bytes
New:               ./data/lorem.dat   1165 bytes
Percentage Change: -44.205%
Time Elapsed:      0.467 ms
```
The entropy metric comes from the [Entropy Equation](https://en.wikipedia.org/wiki/Entropy_(information_theory))
```math
H(X) = -\sum_{i=1}^{n}p(x_i)\log_2p(x_i)
```
The resulting average code length is 6.89744 but the theorhetical limit for average code length is 4.2378, the result from the entropy equation.
