# nldtool: Automated search for nonlinear differential characteristics in cryptographic algorithms

The **nldtool** implements a dedicated guess-and-determine algorithm to search for differential characteristics and confirming input pairs.
It has been used in numerous research papers to find collisions and other cryptanalytic results for various hash functions, particularly SHA-2 [[1]](#references).
For details, we refer to the [introduction](#introduction) below.
A non-exhaustive list of related papers on [hash function results](#hash-function-results) and other [related work](#related-work) is included at the bottom.
For information on the team and contributors, see the `AUTHORS` file.


## Quick Start Guide

Clone the submodules, build the nldtool and run some basic tests:

    git submodule update --init
    cmake . && cmake --build . && ctest

Find a differential characteristic and colliding message pair for MD4 withing a minute:

    ./nldtool -i examples/md4/eurocryptWangLFCY05/start.xml -R 963821092 -E

Start the search for a 27-round differential characteristic in SHA-256:

    ./nldtool -i examples/sha2/asiacryptMendelNS11/27_256_coll_start.xml


## Table of Contents

* [Quick Start Guide](#quick-start-guide)
* [Table of Contents](#table-of-contents)
* [Building](#building)
  - [Single-Configuration Generators (e.g. Unix Makefiles)](#single-configuration-generators-eg-unix-makefiles)
  - [Multi-Configuration Generators (e.g. Visual Studio)](#multi-configuration-generators-eg-visual-studio)
  - [Build Options](#build-options)
* [How to Run](#how-to-run)
  - [Introduction](#introduction)
  - [Example: MD4 search](#example-md4-search)
  - [A sample run](#a-sample-run)
  - [Interpreting the results](#interpreting-the-results)
  - [References](#references)
* [Adding a new crypto function description](#adding-a-new-crypto-function-description)
* [Adding a custom bitslice function](#adding-a-custom-bitslice-function)
* [Adding a linear function](#adding-a-linear-function)
  - [Linear functions](#linear-functions)
  - [Example of the linear layer in Ascon](#example-of-the-linear-layer-in-ascon)
  - [References](#references-1)
* [Implementing an S-Box](#implementing-an-s-box)
  - [S-Box as a Lookup Table](#s-box-as-a-lookup-table)
  - [S-Box as a Bitsliced Function](#s-box-as-a-bitsliced-function)
* [Adding Testvectors to a Crypto Function](#adding-testvectors-to-a-crypto-function)
* [Using two-bit conditions for ARX Constructions](#using-two-bit-conditions-for-arx-constructions)
  - [Application to Skein](#application-to-skein)
  - [References](#references-2)
* [Using the search tool to find collisions](#using-the-search-tool-to-find-collisions)
  - [References](#references-3)
* [Search Options](#search-options)
  - [\<search\> Element](#search-element)
  - [\<phase\> Element](#phase-element)
  - [\<setting\> Element](#setting-element)
  - [\<mask\> Element](#mask-element)
  - [\<guess\> Element](#guess-element)
* [Lookahead and Backtracking Strategies](#lookahead-and-backtracking-strategies)
  - [Lookahead Strategies](#lookahead-strategies)
  - [Backtracking Strategies](#backtracking-strategies)
* [Adding a Custom Callback to a Crypto Function](#adding-a-custom-callback-to-a-crypto-function)
* [Enabling Probability Computation](#enabling-probability-computation)
  - [Special Case: MD5](#special-case-md5)
* [Hash Function Results](#hash-function-results)
  - [SHA-2](#sha-2)
  - [SHA-1](#sha-1)
  - [Keccak](#keccak)
  - [RIPEMD-128/160](#ripemd-128160)
  - [SM3](#sm3)
  - [HAS-160](#has-160)
  - [Others](#others)
* [Related Work](#related-work)
  - [SHA-1 Tool](#sha-1-tool)
  - [ARX-Tools](#arx-tools)
  - [Probability Computation](#probability-computation)


## Building

To build the tool, the following dependencies are required:
* cmake 3.5+
* g++ 4.9.0+ or clang 3.4+ or MSVC 2015+
* cxxopts (provided as a git submodule)
* tinyxml2 (provided as a git submodule)

Prior to building, initialize and update the git submodules:

```bash
git submodule update --init
```

Alternatively, download the tagged versions manually and unpack
into the cxxopts and tinyxml2 folders.


### Single-Configuration Generators (e.g. Unix Makefiles)

```bash
cmake .
cmake --build .
ctest
```


### Multi-Configuration Generators (e.g. Visual Studio)

```bash
cmake .
cmake --build . --config Release
ctest -C Release
```


### Build Options

Configure to build only specific crypto algorithms:
```bash
cmake . -DNLDTOOL_CRYPTO="sha1;sha2"
```

To improve the performance (by about 10%) add the `-march=native` flag:
```bash
cmake . -DCMAKE_CXX_FLAGS=-march=native
```

Configure the 'Debug' build for single-configuration generators:
```bash
cmake . -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

Configure the 'Debug' build for multi-configuration generators:
```bash
cmake .
cmake --build . --config Debug
```


## How to Run

### Introduction

**nldtool** is an automated, heuristic search tool for nonlinear differential characteristics in cryptographic functions (hash functions and ciphers). The basic search strategy is a bitwise guess-and-determine approach, summarized as follows by Mendel et al. [[1]](#references):
>In general, our search techniques can be divided into three parts: decision, deduction and backtracking. 
>Note that the same separation is done in many other fields, like SAT solvers. The first aspect of our search strategy is the decision,
>where we decide which bit is chosen and which condition is imposed at its position. In the deduction part we compute the propagation of the imposed 
>condition and check for contradictions. If a contradiction occurs we need to backtrack and undo decisions, which is the third part of the search strategy.

Restrictions on bits are described with *Generalized Conditions*, based on the 2006 paper by De Cannière and Rechberger [[2]](#references). These conditions  describe the possible values for a pair of bits, and are named according to the following table.

| (Xi,Xi*) |     ?    |     -    |     x    |     0    |     u    |     n    |     1    |     #    |     3    |     5    |     7    |     A    |     B    |     C    |     D    |     E    |
|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|
|   (0,0)  | &#x2713; | &#x2713; |     -    | &#x2713; |     -    |     -    |     -    |     -    | &#x2713; | &#x2713; | &#x2713; |     -    | &#x2713; |     -    | &#x2713; |     -    |
|   (1,0)  | &#x2713; |     -    | &#x2713; |     -    | &#x2713; |     -    |     -    |     -    | &#x2713; |     -    | &#x2713; | &#x2713; | &#x2713; |     -    |     -    | &#x2713; |
|   (0,1)  | &#x2713; |     -    | &#x2713; |     -    |     -    | &#x2713; |     -    |     -    |     -    | &#x2713; | &#x2713; |     -    |     -    | &#x2713; | &#x2713; | &#x2713; |
|   (1,1)  | &#x2713; | &#x2713; |     -    |     -    |     -    |     -    | &#x2713; |     -    |     -    |     -    |     -    | &#x2713; | &#x2713; | &#x2713; | &#x2713; | &#x2713; |

These shorthand notations are very helpful in producing a compact and clear output. They allow to describe the entire search process, from a mostly undetermined starting point (`?`) to a differential characteristic (`-`, `x`) and finally a confirming message pair (`0`, `1`, `u`, `n`).

To start a search, you need to provide an XML configuration file containing search options and a starting point for the search.
Example XML configuration files for different ciphers can be found in `examples/*/chars/*.xml`.


### Example: MD4 search

In the following section, we will look at a basic search for the MD4 hash function. We will use the following XML configuration file.

> examples/md4/eurocryptWangLFCY05/start.xml
>```xml
><config>
><options>
>  <option name="f"  value="md4"/>
>  <option name="n"  value="48"/>
>  <option name="w"  value="32"/>
></options>
><comment>Xiaoyun Wang, Xuejia Lai, Dengguo Feng, Hui Chen, Xiuyuan Yu: Cryptanalysis of the Hash Functions MD4 and RIPEMD. EUROCRYPT >2005</comment>
><char value="
> -4 A: 01100111010001010010001100000001
> -3 A: 00010000001100100101010001110110
> -2 A: 10011000101110101101110011111110
> -1 A: 11101111110011011010101110001001
>  0 A: -------------------------------- W: --------------------------------
>  1 A: ???????????????????????????????? W: ????????????????????????????????
>  2 A: ???????????????????????????????? W: ????????????????????????????????
>  3 A: ???????????????????????????????? W: --------------------------------
>  4 A: ???????????????????????????????? W: --------------------------------
>  5 A: ???????????????????????????????? W: --------------------------------
>  6 A: ???????????????????????????????? W: --------------------------------
>  7 A: ???????????????????????????????? W: --------------------------------
>  8 A: ???????????????????????????????? W: --------------------------------
>  9 A: ???????????????????????????????? W: --------------------------------
> 10 A: ???????????????????????????????? W: --------------------------------
> 11 A: ???????????????????????????????? W: --------------------------------
> 12 A: ???????????????????????????????? W: ????????????????????????????????
> 13 A: ???????????????????????????????? W: --------------------------------
> 14 A: ???????????????????????????????? W: --------------------------------
> 15 A: ???????????????????????????????? W: --------------------------------
> 16 A: ????????????????????????????????
> 17 A: ????????????????????????????????
> 18 A: ????????????????????????????????
> 19 A: ????????????????????????????????
> 20 A: ????????????????????????????????
> 21 A: --------------------------------
> 22 A: --------------------------------
> 23 A: --------------------------------
> 24 A: --------------------------------
> 25 A: --------------------------------
> 26 A: --------------------------------
> 27 A: --------------------------------
> 28 A: --------------------------------
> 29 A: --------------------------------
> 30 A: --------------------------------
> 31 A: --------------------------------
> 32 A: --------------------------------
> 33 A: --------------------------------
> 34 A: --------------------------------
> 35 A: x???????????????????????????????
> 36 A: ????????????????????????????????
> 37 A: --------------------------------
> 38 A: --------------------------------
> 39 A: --------------------------------
> 40 A: --------------------------------
> 41 A: --------------------------------
> 42 A: --------------------------------
> 43 A: --------------------------------
> 44 A: --------------------------------
> 45 A: --------------------------------
> 46 A: --------------------------------
> 47 A: --------------------------------
>    "/>
><search reseed="-1" credits="1000">
>  <phase twobit_complete="1">
>    <setting prob="1">
>      <mask word="A" rounds="32-47"/>
>      <guess condition="?" choice_prob="1"/>
>      <guess condition="x" choice_prob="0.000001"/>
>    </setting>
>  </phase>
>  <phase twobit_complete="1">
>    <setting prob="1">
>      <mask word="A"/>
>      <mask word="W"/>
>      <guess condition="?" choice_prob="1"/>
>      <guess condition="x" choice_prob="0.000001"/>
>    </setting>
>  </phase>
>  <phase twobit_complete="1">
>    <setting prob="1" ordered_guesses="1">
>      <mask word="W"/>
>      <guess condition="-" choice_prob="0.5"/>
>    </setting>
>  </phase>
></search>
></config>
>```

A short explanation of the XML configuration:
* `<options>`: These options contain the name of the crypto function, the number of rounds, and the word size.
* `<char>`: The starting point for the characteristic. This one is taken from Wang et al. [[3]](#references)
  + The names of the words `A` (part of internal state) and `W` (message input) are defined in the C++ implemenation of the crypto function 
(**examples/md4/md4.cpp**).
* `<search>`: The search configuration, split into multiple phases.

A more detailed explanation of the XML file and its options can be found in  [Using the Search Tool to Find Collisions](#using-the-search-tool-to-find-collisions) and [Search Options](#search-options).


### A sample run

To start the search, just pass the XML configuration file as a option with the option `-i` or `--input-file`. We additionally use the options `-C 0` or `--print-char 0` to supress the continuous output of the current state and `-E` or `--end-when-found` to end the search once a result has been found. (You can show all available command line options with `--help`.)

```bash
./nldtool -i examples/md4/eurocryptWangLFCY05/start.xml -C 0 -E
```
The tool then checks the starting characteristic for validity and starts the search. The phases of the search are executed in order, only beginning the next phase once every bit matching the phases masks is determined. For MD4, the tool finds a solution in a few seconds. The ouput of our sample run is found below.
```
Info: Search started...
Info: seed: 3070187328 time: 2 iterations/sec: 3346 iterations: 6693 stack_size: 225 contr: 416 restarts: 1 minfree: 11 absminfree: 11 credits: 584 phase: 1 found: 0 smax: 255 complete: 3/3 0/0
Info: seed: 3070187328 time: 3 iterations/sec: 4869 iterations: 14607 stack_size: 202 contr: 1000 restarts: 1 minfree: 11 absminfree: 11 credits: 0 phase: 1 found: 0 smax: 255 complete: 5/5 0/0
Info: seed: 3070187328 time: 4 iterations/sec: 6031 iterations: 9517 stack_size: 526 contr: 1000 restarts: 2 minfree: 0 absminfree: 0 credits: 0 phase: 2 found: 0 smax: 542 complete: 1/1 1/1 0/0
Info: seed: 3070187328 time: 6 iterations/sec: 5335 iterations: 7887 stack_size: 320 contr: 436 restarts: 3 minfree: 38 absminfree: 0 credits: 564 phase: 1 found: 0 smax: 357 complete: 4/4 0/0
Info: seed: 3070187328 time: 8 iterations/sec: 5003 iterations: 15901 stack_size: 337 contr: 955 restarts: 3 minfree: 38 absminfree: 0 credits: 45 phase: 1 found: 0 smax: 364 complete: 4/4 0/0
Info: seed: 3070187328 time: 10 iterations/sec: 4910 iterations: 8291 stack_size: 223 contr: 567 restarts: 4 minfree: 18 absminfree: 0 credits: 433 phase: 1 found: 0 smax: 265 complete: 5/5 0/0
found new characteristic
 -4 A: 01100111010001010010001100000001                                    
 -3 A: 00010000001100100101010001110110                                    
 -2 A: 10011000101110101101110011111110                                    
 -1 A: 11101111110011011010101110001001                                    
  0 A: 00101010110000101001011010010100 W: 10000101010110000101001011010011
  1 A: 0100111110010000011111001n111000 W: u0100101011101000000000110011001
  2 A: 100001010010111011000u0nu0001100 W: n10n1110000010000011000101001001
  3 A: 01000000100010100001101110100000 W: 00100011111001100000010111110000
  4 A: 10000101011001000010nun110101110 W: 10010110110011111000100110001001
  5 A: 1101100000101n1n10n1010110100001 W: 11110100000101010001011011010011
  6 A: 1nu0101n0u001n001100111100101000 W: 11011111010001100111010011101101
  7 A: 00010100001000n11unnn1n010000101 W: 01101010110110011110000100111110
  8 A: 111011111101nnn01101000111000011 W: 00110000100010110001111101101010
  9 A: 111011un0nnnn0001111011101110101 W: 00001111101010001001110110100100
 10 A: 110u0100nu1110001101011n11011100 W: 00110011001011111110111000110001
 11 A: 0u1n1010100100111101000011111010 W: 01110110011111001011000101110110
 12 A: 100n01000u11u1101001110110111111 W: 001111001010111n0000101000010111
 13 A: 11010110111100110011101110110001 W: 00100101100110100001110000001000
 14 A: 00000001010nu0011001110111000101 W: 00110111101101000111100001011100
 15 A: 000011001010111u1010111001010101 W: 00000110101111111011011011100000
 16 A: n10010u0001010010101000000000011                                    
 17 A: 00001101110110110110001100011010                                    
 18 A: 000n0101001100011011011100110010                                    
 19 A: 00110100101000101111011000100011                                    
 20 A: n11u1110100111100001001100111010                                    
 21 A: 00100100101101010101011100010010                                    
 22 A: 00100110010010110100001101001000                                    
 23 A: 11011011110110111101101101101011                                    
 24 A: 01110000001000001000101100110010                                    
 25 A: 00011001010000100010000001011010                                    
 26 A: 10000000011011010001100000011010                                    
 27 A: 01011010101011110100111111001110                                    
 28 A: 00110111110000001001100010101000                                    
 29 A: 00110001011100100111011101111110                                    
 30 A: 10011101010001100010111100001010                                    
 31 A: 01110111111011100011111000100110                                    
 32 A: 00111110011010011110101101110000                                    
 33 A: 00110010111110011100101101001011                                    
 34 A: 01101110000100101000100011110011                                    
 35 A: n1101110010100110100001011111100                                    
 36 A: n1110000001000000100110011110001                                    
 37 A: 11001010010110000011011010001010                                    
 38 A: 11110001000100000100010010000010                                    
 39 A: 11110010111110010011000000100100                                    
 40 A: 01110000111110111110001010111010                                    
 41 A: 11011010101010111101011101110111                                    
 42 A: 01000010011001101111110101100101                                    
 43 A: 00000000001110101011011111010010                                    
 44 A: 11100101100110111001000001011100                                    
 45 A: 10010100100111101111001011001111                                    
 46 A: 11010100111111000010110001101010                                    
 47 A: 11010100101001100000110111100110  
```


### Interpreting the results

The status line that is printed periodically contains the following information:
* __seed__: The seed used for the RNG (can be set manually with `-R` or `--random-seed`)
* __time__: Elapsed time in seconds.
* __iterations/sec__: How many guesses are done per second.
* __iterations__: Total number of guesses in this run.
* __stack_size__: Current size of the backtracking stack
* __contr__: Total number of contradictions found in this run.
* __restarts__: How many times the search has restarted.
* __minfree__: The minimum number of undefined bits in this run. 
* __absminfree__: The minimum number of undefined bits in all runs. 
* __credits__: Current, amount of credits, restarts search when reaching 0.
* __phase__: Current search phase (from the configuration) 
* __found__: Number of found characteristics.
* __smax__: Maximum size of the backtracking stack for this run.
* __complete__: For phases that reached the end, how many of them were valid when considering conditions over two bits.

The characteristic that is found in our example is composed of generalized conditions [[2]](#references). For our example there are only four different 
conditions: 
* `0`: both messages have the bit at this position set to 0
* `1`: both messages have the bit at this position set to 1
* `n`: message A has the bit at this position set to 0, while message B has the bit at that position set to 1
* `u`: message A has the bit at this position set to 1, while message B has the bit at that position set to 0

If you are running the tool in a terminal with color support, the differences in the characteristic are colored, making them easy to spot.
More information about the search and the structure of the crypto function in the tool can be found in [Adding a new crypto function description](#adding-a-new-crypto-function-description) and [Using the Search Tool to Find Collisions](#using-the-search-tool-to-find-collisions).


### References

[1] Florian Mendel, Tomislav Nad, Martin Schläffer: Finding SHA-2 Characteristics: Searching through a Minefield of Contradictions. ASIACRYPT 2011 https://doi.org/10.1007/978-3-642-25385-0_16
[2] Christophe De Cannière, Christian Rechberger: Finding SHA-1 Characteristics: General Results and Applications. ASIACRYPT 2006 https://doi.org/10.1007/11935230_1
[3] Xiaoyun Wang, Xuejia Lai, Dengguo Feng, Hui Chen, Xiuyuan Yu: Cryptanalysis of the Hash Functions MD4 and RIPEMD. EUROCRYPT 2005 https://doi.org/10.1007/11426639_1


## Adding a new crypto function description

We now demonstrate how to add a new crypto function for the example of MD4 ([RFC 1320](https://tools.ietf.org/html/rfc1320))

The directory structure for the new crypto function should follow the existing
convention. To ease the addition of new crypto functions, a bash script is provided
which creates all neccessary skeleton files for a given name.

```bash
./create_template.sh md4
```

This will create the following directory/file structure:

```
examples
 |---md4
      |---chars
      |    |-----(startpoints for search)
      |
      |---testvectors
      |    |-----(automated tests)
      |
      |---md4.h
      |---md4.cpp
      |---sources.cmake
```

>examples/md4/sources.cmake:
>```cmake
>set(CRYPTO_FILES
>  examples/md4/md4.h
>  examples/md4/md4.cpp
>)
>set(MAP_OPTION_TO_CLASSNAME
>  md4:Md4
>)
>```

We need to add the header and source files to the CRYPTO_FILES variable and
set the class and command line option name of the crypto function such that
the crypto_factory.cpp file can create our crypto function by its name.

>examples/md4/md4.h:
>```cpp
>#ifndef MD4_H_
>#define MD4_H_
>
>#include "crypto.h"
>#include "condition_word.h"
>
>class Md4 : public Crypto
>{
>...
>```

All specific crypto functions have to inherit from the class "Crypto".

>examples/md4/md4.h: (cont.)
>```cpp
>...
>public:
>  static const uint32 K[3];
>  static const int S[12];
>  static const int P[48];
>
>  static void AddToOption(cxxopts::Options& options);
>
>  Md4(cxxopts::Options& options);
>...
>```

We define members to hold the constants used by MD4 (the constant `K`, the rotation `S`,
and the permutation of the input words `P`). Furthermore, we define a static method and
constructor which pass the command line options to the crypto function.

>examples/md4/md4.h: (cont.)
>```cpp
>...
>protected:
>  int num_rounds_;
>  ConditionWordPtr W[16];
>  ConditionWordPtr tA[48 + 4];
>  ConditionWordPtr* A = &tA[4];
>  ConditionWordPtr F[48];
>};
>
>#endif // MD4_H_
>```

Finally we define the `ConditionWordPtr` containers that hold the actual processed data.
In the case of MD4 we have 16 message words, the 4 state words A,B,C,D but since only
one of these words is actually updated per round, we can use an easier description
using only A and previous values of A. To make addressing more intuitive, we also
define `A` as `&tA[4]` so we can use `A[-4]` in the algorithm.
Finally we need the `ConditionWordPtr` F to store the output of the combiner functions in
MD4.

>examples/md4/md4.cpp:
>
>```cpp
>#include "md4/md4.h"
>
>#include "bitslice_step.h"
>#include "functions.h"
>
>const uint32 Md4::K[3] = {0x00000000, 0x5a827999, 0x6ed9eba1};
>
>const int Md4::S[12] = {3, 7, 11, 19, 3, 5, 9, 13, 3, 9, 11, 15};
>
>const int Md4::P[48] = {
>    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
>    0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15,
>    0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15,
>};
>
>Md4::Md4(cxxopts::Options& options)
>    : Crypto(options["word-size"].as<int>()),
>      num_rounds_(options["num-rounds"].as<int>()) {
>...
>```

In the source file we initialize the constants according to their values from the
MD4 specification. We call the super constructor with the used word size.

>examples/md4/md4.cpp: (cont.)
>```cpp
>...
>  for (int i = -4; i < 0; i++)
>    A[i] = AddConditionWord("A", i, 4 + i, 0);
>  for (int i = 0; i < std::min(16, num_rounds_); i++)
>    W[i] = AddConditionWord("W", i, 4 + i * 2 + 1, 1);
>...
>```

The first rounds of A and the message words are initialized. Each `ConditionWord`
has at least the following options:
* the name (should be unique within each round)
* the round number it belongs to
* the row for print output
* the column for print output

>examples/md4/md4.cpp: (cont.)
>```cpp
>...
>  for (int i = 0; i < num_rounds_; i++) {
>    F[i] = AddConditionWord("F", i, 4 + i * 2 + 0, 1, SUBWORD);
>    A[i] = AddConditionWord("A", i, 4 + i * 2 + 1, 0);
>...
>```

For each round of MD4, we initialize the two needed `ConditionWord`containers. Note that `F` is
of the type `SUBWORD`, which is a flag that excludes F from normal print output.

>examples/md4/md4.cpp: (cont.)
>```cpp
>...
>    if (i < 16)
>      Add(new BitsliceStep<IF>(word_size_, A[i - 1], A[i - 2], A[i - 3], F[i]));
>    else if (i < 32)
>      Add(new BitsliceStep<MAJ>(word_size_, A[i - 1], A[i - 2], A[i - 3], F[i]));
>    else
>      Add(new BitsliceStep<XOR3>(word_size_, A[i - 1], A[i - 2], A[i - 3], F[i]));
>...
>```

Now we use a `BitsliceStep` with one of the predefined functions `IF`, `MAJ` and `XOR3`
and save the result in `F[i]`. The first option is the word size `word_size_`, the rest of the
options are forwarded to the function. (e.g. `XOR3` is an xor with 3 inputs and 1 output)

>examples/md4/md4.cpp: (cont.)
>```cpp
>...
>    int m = P[i];
>    ConditionWordPtr k(new ConditionWord(K[i / 16]));
>    Add(new CarryStep<ADD4>(word_size_, A[i - 4], F[i], W[m], k, A[i]->Rotr(S[(i / 16) * 4 + i % 4])));
>  }
>}
>```

Finally the 4 words are added together according to the MD4 algorithm. The constant P is
looked up for the current round and we build a constant `ConditionWord` for the constant `k`.
Then we use a `CarryStep` for `ADD4`, which is a special case of a `BitsliceStep` where
the necessary carry words for the addition are added automatically. We can also see the
rotation is performed by saving the result in a rotated reference to `A[i]`. The `Rotr/Rotl`
methods allow a `ConditionWord` to be accessed in a rotated way, without the need for a
temporary copy in between.

The description of MD4 is now complete and can be used to find collisions with the
search tool. Be sure to add your crypto to the `CRYPTO` variable in the local.cmake file. 


## Adding a custom bitslice function

Although some of the most used bitslice functions (like `ADD`, `XOR`) are alredy implemented in the tool, many crypto function definitions 
require the implementation of custom functions for their algorithm.

In this section we will break down the necessary parts of a bitsliced function for the example of the majority function with 3 inputs.

```cpp
class MAJ: public F {
public:
  static const int IN = 3;
  static const int OUT = 1;
  static const int NUM = IN + OUT;
  static constexpr char NAME[] = "MAJ";
  template<class T> static inline void f(T x[NUM]) {
    T& a = x[0];
    T& b = x[1];
    T& c = x[2];
    T& r = x[3];
    r = (a & b) ^ (b & c) ^ (c & a);
  }
};
```

All functions need to inherif from the base function `F`, and need to set their number of inputs `IN`, number of outputs `OUT` and total number of variables `NUM`. 

The function itself has to have the structure `template<class T> static inline void f(T x[NUM])`. The option `x` is an array of all variables, with the inputs starting at index `0`, and the outputs starting at index `IN` (in this case 3). The necessary computations are then performed with the inputs and written to the output variables. 

Various examples for more complicated functions can be found in the existing crypto function implementations.


## Adding a linear function

### Linear functions

**nldtool** also provides the possibility to model linear operations in a cipher with a larger number of input and output bits. The general use of these linear functions in our tool is summarized by Eichlseder et al. [[1]](#references) as follows.

>The  constraints  defined  by  generalized  conditions  and  the  function *f* can  also
>be  expressed  as  a  system  of  equations  involving  the  input  and  output  bits  as
>variables. Propagating conditions then corresponds to manipulating
>this system in order to bring it to a more useful form. If all involved equations
>are linear, methods like elementary row and column operations can be applied to
>simplify the system. Unfortunately, it is not possible to translate all generalized
>conditions  into  linear  equations.  Furthermore,  crypto  functions  contain  nonlinear
>building blocks such as modular additions or other nonlinear Boolean functions
>which cannot be expressed as linear equations.
>However, large parts of cryptographic algorithms consist of affine (linear) functions. 
>Furthermore, most generalized conditions are linear (only **7**,**B**,**D**, and **E** are
>nonlinear). 

The linear equation system resulting from a linear function can be solved very efficiently and leads to a better
propagation of conditions through the state of the crypto function.

### Example of the linear layer in Ascon

The linear layer uses an xor of rotated copies of each word for horizontal diffusion within each word, with different rotation values for each word [[2]](#references).

These rotations and xors are linear operations, meaning we can utilize the `LinearStep` of the search tool. Below you can find the implementation of the
linear layer as a template taking the 3 rotation values.

> examples/ascon/ascon.cpp
>```cpp
>...
>////////////////////////////// class Sigma //////////////////////////////
>
>template<int R0, int R1, int R2>
>class Ascon::Sigma: public F {
>public:
>  static const int IN = 1;
>  static const int OUT = 1;
>  static const int NUM = IN + OUT;
>  static constexpr char NAME[] = "SIGMA";
>  static void f(int word_size, uint64 x[IN + OUT]) {
>    x[1] = nldtool::Rotr(x[0], R0, word_size) ^ nldtool::Rotr(x[0], R1, word_size) ^ nldtool::Rotr(x[0], R2, word_size);
>  }
>};
>...
>```

This function can then be used in the description of the cipher. Note that we use `LinearStep` instead of `BitsliceStep`
when adding the steps. This means the step will be calculated with a linear equation system instead of exhaustive search over
all options like the `BitsliceStep`. 

> examples/ascon/ascon.cpp
>```cpp
>...
>    step = Add(new LinearStep<Ascon::Sigma<0, RotVal[0], RotVal[1]> >(word_size_, B[i-1][0], A[i][0]));
>    A[i][0]->SetStepToComputeProbability(step);
>    step = Add(new LinearStep<Ascon::Sigma<0, RotVal[2], RotVal[3]> >(word_size_, B[i-1][1], A[i][1]));
>    A[i][1]->SetStepToComputeProbability(step);
>    step = Add(new LinearStep<Ascon::Sigma<0, RotVal[4], RotVal[5]> >(word_size_, B[i-1][2], A[i][2]));
>    A[i][2]->SetStepToComputeProbability(step);
>    step = Add(new LinearStep<Ascon::Sigma<0, RotVal[6], RotVal[7]> >(word_size_, B[i-1][3], A[i][3]));
>    A[i][3]->SetStepToComputeProbability(step);
>    step = Add(new LinearStep<Ascon::Sigma<0, RotVal[8], RotVal[9]> >(word_size_, B[i-1][4], A[i][4]));
>    A[i][4]->SetStepToComputeProbability(step);
>...
>```


### References

[1] Maria Eichlseder, Florian Mendel, Tomislav Nad, Vincent Rijmen, Martin Schläffer: Linear Propagation in Efficient Guess-and-Determine Attacks: WCC 2013 http://www.selmer.uib.no/WCC2013/pdfs/Eichlseder.pdf
[2] http://ascon.iaik.tugraz.at/specification.html


## Implementing an S-Box

### S-Box as a Lookup Table

Most Sbox definitions are given as a lookup table and there is an easy way to use this definition available in the tool.
In the following example, the 5-bit S-Box of Ascon is implemented using a lookup table.

> examples/ascon/ascon.h
>
>```cpp
>class Ascon: public Crypto {
>...
>public:
>  static constexpr uint8_t LUT[32] = {4, 11, 31, 20, 26, 21, 9, 2, 27, 5, 8, 18, 29, 3, 6, 28, 30, 19, 7, 14, 0, 13, 17, 24, 16, 12, 1, 25, 22, 10, 15, 23};
>  class Sbox;
>...
>}
>```

> examples/ascon/ascon.cpp
>
>```cpp
>...
>  step = Add(new BitsliceStep<SBOX<5, 5, LUT>>(
>      word_size_, A[i - 1][0], A[i - 1][1], A[i - 1][2], A[i - 1][3],
>      A[i - 1][4], B[i - 1][0], B[i - 1][1], B[i - 1][2], B[i - 1][3],
>      B[i - 1][4]));
>...
>}
>```


### S-Box as a Bitsliced Function

If a bitsliced definition of the S-Box is available, you can also use this to implement the S-Box.
The following example shows the same S-Box for Ascon, but now as a bitsliced definition.

>examples/ascon/ascon.cpp
>
>```cpp
>class Ascon::Sbox: public F {
>public:
>  static const int IN = 5;
>  static const int OUT = 5;
>  static const int NUM = IN + OUT;
>  static constexpr char NAME[] = "SBOX";
>  template<class T> static inline void f(T x[IN + OUT]) {
>    T r0 = x[0];
>    T r1 = x[1];
>    T r2 = x[2];
>    T r3 = x[3];
>    T r4 = x[4];
>    T t0, t1, t2, t3, t4;
>    r0 ^= r4;    r4 ^= r3;    r2 ^= r1;
>    t0  = r0;    t1  = r1;    t2  = r2;    t3  = r3;    t4  = r4;
>    t0 =! t0;    t1 =! t1;    t2 =! t2;    t3 =! t3;    t4 =! t4;
>    t0 &= r1;    t1 &= r2;    t2 &= r3;    t3 &= r4;    t4 &= r0;
>    r0 ^= t1;    r1 ^= t2;    r2 ^= t3;    r3 ^= t4;    r4 ^= t0;
>    r1 ^= r0;    r0 ^= r4;    r3 ^= r2;    r2 =! r2;
>    x[5 + 0] = r0;
>    x[5 + 1] = r1;
>    x[5 + 2] = r2;
>    x[5 + 3] = r3;
>    x[5 + 4] = r4;
>  }
>};
>```


## Adding Testvectors to a Crypto Function

To enable easy automated testing, all crypto functions should contain one or more testvectors to ensure the basic functionality of the crypto function is correct.
These testvectors are essentially search configuration files without the `<search>` element. When placed in the __examples/*/testvectors__ folder, the test is automatically added to the `make test` target.

The following example testvector is for md4. The exact value of the bits thoughout the crypto computation is taken from a seperate reference implementation.

> examples/md4/testvectors/md4-test1.xml
>```xml
><config>
><options>
>  <option name="f" value="md4"/>
>  <option name="n" value="48"/>
>  <option name="w" value="32"/>
></options>
><char value="
>A: 01101100100001001100001110011111
>A: 01010100010000111011110010011111
>A: 00101000000111101110100011010100
>A: 01111100100101110101101110011101
>A: 11010000110110000000100110010110 W: 01000101001111110101000011111101
>A: 10001111110101111101010000001001 W: 01000110010001010000100100110101
>A: 11100001100011000100101000010000 W: 00101001001011010011110100100001
>A: 10111001100100001010100010001000 W: 00000110101001010111101000001111
>A: 00110111111000010110110001101111 W: 01101110010111001100011111110110
>A: 01111110011010001101110111001111 W: 00011101100110001101001110011010
>A: 01011110101010111000010101111101 W: 00010110001011110001111100010001
>A: 11001101010011001100110010100111 W: 01100000100110110110001111010010
>A: 10010001011011100001010100100111 W: 00111100001000111100000011001000
>A: 00011101010101111011101011011000 W: 01100010000001000100110100100111
>A: 00010100111101100000010001110000 W: 01011110000010001100010100011100
>A: 00000101000001000100101001100010 W: 00100110101000010110001010100010
>A: 10111100110000110100100100000000 W: 01101001110100101010001100000001
>A: 11010101011110110100000011001000 W: 01110000000111101110111100111001
>A: 10000000110111100010000100010010 W: 01111001000100101100110100110010
>A: 00001001111000001011011001000001 W: 01010101011010001000111011011010
>A: 11110011111110011001111010110110
>A: 00001010011001110000110100100100
>A: 11001011111100110010111001000110
>A: 00011100001000000011001101000101
>A: 11111001001000101000011001000100
>A: 01110100100100000001001101101011
>A: 00110100000100001001011000001001
>A: 01000110010011000000101101011010
>A: 10000111000100101000001001001111
>A: 01101010010001011100110000011101
>A: 11000000101111100011101001100101
>A: 11111011100011100101101101111110
>A: 10010110010001101000001110100110
>A: 11111110001110001001110111011110
>A: 11100001011001010011110001111111
>A: 11000000001111011111010000111011
>A: 01001110000000001010111011110001
>A: 00011101011000011111100000110001
>A: 11000100100110001000101010001111
>A: 00101111100101100001100001110010
>A: 11100011101110100001001111111110
>A: 11110010010100111110001111100101
>A: 00001011111011010101010001000001
>A: 10111010110011111001100011000011
>A: 11100010010100011100000111011110
>A: 01001010010101000001001000101101
>A: 01010010111101010110010101011001
>A: 10010101001000111100101001011100
>A: 00101010100111110010010110110111
>A: 00110001110101100100010110011010
>A: 10101010111110011110110110000110
>A: 01101001010000010000010110001011
>    "/>
></config>
>```

When calling `make test`the tool checks the crypto function by setting all the bits to the given value and checking for a contradiction.
Additionally, just defining the input and the output, as well as the message is sufficient to test a crypto function. An easy way to generate test files is 
to use any testvectors provided by the standard/reference documents.


## Using two-bit conditions for ARX Constructions

Leurent et al. [[1]](references) showed that for crypto functions based on ARX constructions (like Blake, Skein), considering conditions on two consecutive bits can improve the serach for differential characteristics

>We extend the generalized characteristics of de Cannière and Rechberger by introducing constraints involving several consecutive bits of a variable, 
>instead of considering bits one by one. We show that constraints on 2 consecutive bits can completely capture the modular difference, and we
>introduce reduced sets of constraints on 1.5 and 2.5 consecutive bits. This is motivated by the analysis of modular addition, but since these constraints are
>still local, they interact well with bitwise Boolean operations and rotations, and we can use them to study pure ARX as well as SHA-like constructions.

### Application to Skein

For the SHA-3 finalist Skein, **examples/skein/skein.cpp** contains improvements made by Leurent et al.

> examples/skein/skein.cpp
>```cpp
>...
>		// mix & permutate
>		Add(new BitsliceStep<ADD2B>(word_size_-1, e[d][0], e[d][1], c[4]->Shl(1), v[d+1][0], c[4], c[4]->Shr(1)));
>		Add(new BitsliceStep<XOR2B>(word_size_-1, e[d][1]->Rotl(Rdj[d%8][0]), v[d+1][0], v[d+1][3]));
>
>		Add(new BitsliceStep<ADD2B>(word_size_-1, e[d][2], e[d][3], c[5]->Shl(1), v[d+1][2], c[5], c[5]->Shr(1)));
>		Add(new BitsliceStep<XOR2B>(word_size_-1, e[d][3]->Rotl(Rdj[d%8][1]), v[d+1][2], v[d+1][1]));
>...
>```

We can see that this version of **skein** essentially performs the same steps, but is using **ADD2B** instead of **ADD2** and **XOR2B**
instead of **XOR**. These functions are the two-bit versions of their respective counterparts. Using these functions when dealing with ARX constructions can 
improve the propagation of conditons and lead to better search results.

### References
[1] Gaëtan Leurent: Analysis of Differential Attacks in ARX Constructions. ASIACRYPT 2012 https://doi.org/10.1007/978-3-642-34961-4_15
[2] Gaëtan Leurent: Construction of Differential Characteristics in ARX Designs Application to Skein. CRYPTO 2013 https://doi.org/10.1007/978-3-642-40041-4_14


## Using the search tool to find collisions

To use the search tool you need a XML configuration file.

>examples/md4/chars/md4_empty.xml
>```xml
><config>
>    <options>
>        <option name="f"  value="md4"/>
>        <option name="n"  value="48"/>
>        <option name="w"  value="32"/>
>    </options>
>    <char value=""/>
></config>
>```

The three options are required and should be set to the following values:
* `f`: the name of the crypto function
* `n`: the number of rounds
* `w`: the word size (in bits)

The `char` is the description of the characteristic. If you run the tool with an empty
`value` argument, it will output a completely undefined characteristic, which you can
copy to the configuration file and then modify to your needs.

```bash
./nldtool -i examples/md4/chars/md4_empty.xml
```

Various configuration files can already be found for most existing crypto functions.
An example follows with a characteristic taken from Sasaki et al. [[1]](#references).

>examples/md4/chars/48rounds_m0m2m4m8m12.xml
>```xml
><config>
> <options>
>   <option name="f"  value="md4"/>
>   <option name="n"  value="48"/>
>   <option name="w"  value="32"/>
> </options>
> <comment>Yu Sasaki, Lei Wang, Kazuo Ohta, Noboru Kunihiro: New Message Difference for MD4. FSE 2007</comment>
> <char value="
>  -4 A: 01100111010001010010001100000001
>  -3 A: 00010000001100100101010001110110
>  -2 A: 10011000101110101101110011111110
>  -1 A: 11101111110011011010101110001001
>   0 A: ???????????????????????????????? W: ????????????????????????????????
>   1 A: ???????????????????????????????? W: --------------------------------
>   2 A: ???????????????????????????????? W: ????????????????????????????????
>   3 A: ???????????????????????????????? W: --------------------------------
>   4 A: ???????????????????????????????? W: ????????????????????????????????
>   5 A: ???????????????????????????????? W: --------------------------------
>   6 A: ???????????????????????????????? W: --------------------------------
>   7 A: ???????????????????????????????? W: --------------------------------
>   8 A: ???????????????????????????????? W: ????????????????????????????????
>   9 A: ???????????????????????????????? W: --------------------------------
>  10 A: ???????????????????????????????? W: --------------------------------
>  11 A: ???????????????????????????????? W: --------------------------------
>  12 A: ???????????????????????????????? W: ????????????????????????????????
>  13 A: ???????????????????????????????? W: --------------------------------
>  14 A: ???????????????????????????????? W: --------------------------------
>  15 A: ???????????????????????????????? W: --------------------------------
>  16 A: ????????????????????????????????
>  17 A: ????????????????????????????????
>  18 A: ????????????????????????????????
>  19 A: ????????????????????????????????
>  20 A: ????????????????????????????????
>  21 A: --------------------------------
>  22 A: --------------------------------
>  23 A: --------------------------------
>  24 A: --------------------------------
>  25 A: --------------------------------
>  26 A: --------------------------------
>  27 A: --------------------------------
>  28 A: --------------------------------
>  29 A: --------------------------------
>  30 A: --------------------------------
>  31 A: --------------------------------
>  32 A: x???????????????????????????????
>  33 A: --------------------------------
>  34 A: --------------------------------
>  35 A: --------------------------------
>  36 A: --------------------------------
>  37 A: --------------------------------
>  38 A: --------------------------------
>  39 A: --------------------------------
>  40 A: --------------------------------
>  41 A: --------------------------------
>  42 A: --------------------------------
>  43 A: --------------------------------
>  44 A: --------------------------------
>  45 A: --------------------------------
>  46 A: --------------------------------
>  47 A: --------------------------------
>     "/>
> <search credits="10000">
>   <phase twobit_complete="1">
>     <setting prob="1">
>       <mask word="A" rounds="32-47"/>
>       <guess condition="?" choice_prob="1"/>
>       <guess condition="x"  choice_prob="0.000001"/>
>     </setting>
>   </phase>
>   <phase twobit_complete="1">
>     <setting prob="1">
>       <mask word="A"/>
>       <mask word="W"/>
>       <guess condition="?" choice_prob="1"/>
>       <guess condition="x"  choice_prob="0.000001"/>
>     </setting>
>   </phase>
>   <phase twobit_complete="1">
>     <setting prob="1" ordered_guesses="1">
>       <mask word="W"/>
>       <guess condition="-" choice_prob="0.5"/>
>     </setting>
>   </phase>
> </search>
></config>
>```

The actual search is configured via the `<search>` element.
There are several phases in the search to better control our desired results.
* Phase 1: We guess `?` and `x` bits of `A` in rounds 32-47 first to allow us to achive
few differences in these rounds.
* Phase 2: Then the `?` and `x` are determined for the rest of the `A` and `W` words.
* Phase 3: Finally the `-` bits are determined to produce an instance of a colliding message.

A `<phase>` has at least one `<setting>` element, containing one or more `<mask>` and `<guess>`
elements. A `<mask>` element is selecting the words that are to be guessed. Its `word`
attribute is a regular expression which matches the name of the `ConditionWord` used
in the description. A `<guess>` element is determining which bits are guessed. The
`condition` attribute is used to select bits based on their current constraints.
The `choice_prob` is the distribution of the two possible choices of the guess. In
this example the `choice_prob` for `?` guesses is `1`, because we always prefer the
`-` guess (no difference) over the `x` guess (difference), because it leads to a
sparser characteristic. 

Starting the tool with the above configuration file should find a collision
within a few seconds on a modern PC.

```bash
./nldtool -i examples/md4/chars/48rounds_m0m2m4m8m12.xml -E
```


### References

[1]: Yu Sasaki, Lei Wang, Kazuo Ohta, Noboru Kunihiro: New Message Difference for MD4. FSE 2007, https://doi.org/10.1007/978-3-540-74619-5_21


## Search Options

The search tool is configurable to a great extend with a XML configuration file. In the following sections we will describe the various options that can be used in the configuration of the search.

---

### \<search\> Element

Attributes:
* __reseed__: time in seconds after which the RNGs are reseeded (default: disabled)
* __credits__: number of allowed contradictions before the search is restarted (required, e.g. `1000`)
* __callback__: name of the callback function that is called when the search starts or restarts (default: none)

Child Elements:
* __\<phase\>__: at least one search phase is required, phases will be executed in order of their apperance

---

### \<phase\> Element

Attributes:
* __twobit_complete__: should the twobit conditions be checked for completeness at the end of the phase (`0` or `1`, default: `0`)
* __backtrack__: the backtracking strategy that is used for this phase (default: `choice`)
  + `choice`: jump back to last choice on the stack.
  + `10perc` : jump back 10% of the current stack size.
* __lookahead__: the lookahead strategy that is used for this phase (default: `none`)
   + `none`: don't perform any lookahead
   + `mcbranch`: look at a number of branches and choose the one with the lowest number of free bits.
   + `probbranch`: look at a number of branches and choose the one with the highest probability
* __branch__: the number of branches for the  `*branch` lookahead strategies (default: `0`)
* __callback__: name of the callback function that is called when the phase is completed (default: none)

Child Elements:
* __\<setting\>__: at least one setting is required, settings will be selected based on their probability

---

### \<setting\> Element

Attributes:
* __prob__: probability of this setting being selected in the current phase: (required, e.g. `0.9`)
* __ordered_guesses__: if activated, bits are not selected at random, but instead in order of apperance in the crypto function description (`0` or `1`, default: `0`)

Child Elements:
* __\<mask\>__ : at least one mask is required
* __\<guess\>__: at least one guess is required

---

### \<mask\> Element

Attributes:
* __word__: regular expression identifying the words to be guessed (required, e.g. `"A[0-5]"`)
* __rounds__: single number or range of numbers selecting the rounds that should be guessed for the given word/s (default: all rounds, e.g. `10` or `3-7`)
* __twobit_threshold__: threshold for mask to only include bits with a twobit degree higher than the threshold (default: none, e.g. `2`)

---

### \<guess\> Element

Attributes:
* __condition__: type of condition to make a guess (required, e.g. `?`,  `x`)
* __choice_prob__: probability for the first of the two choice-options; e.g. for the choice `?` and its two options  `-` and `x`, how likely is `-` (required, e.g. `0.5`) (if the probability is not `0` or `1`, the other choice is placed on the stack for backtracking)


## Lookahead and Backtracking Strategies

### Lookahead Strategies

A lookahead strategy can increase the performance of the search.  The implemented lookahead strategy chooses  a number of random guess positions and evaluates them based on some metric. The evaluation metrics can be the number of free bits after the guess, or the overall probability of the characteristic.

For most searches, a lookahead with 8 random guess positions and the number of free bits as a metric  (`lookahead="mcbranch" branch="8"`) is a good lookahead strategy. For an example, see the SHA-256 XML configuration file **examples/sha2/chars/27-t10.xml** . You can compare the frequency of found characteristics for this configuration to the same configuration when setting the option `branch="0"`.

> `branch="8"`
>```
>Info: ... time: 120 ... found: 18 ...
>```

>`branch="0"`
>```
>Info: ... time: 120 ... found: 2 ...
>```


### Backtracking Strategies

For most searches the default backtracking strategy of going back to the last choice is sufficient to get a good search. However, for crypto functions with a very large state like Keccak, it can be more benifitial to jump back a fixed amount of guesses or a percentage of the current stack size. This prevents the search from getting stuck in very deep, but impossible search branches for a long time. However, the choice of the correct backtracking strategy should be made on a case by case basis.

An example of the `10perc` backtracking strategy can be found in the Keccack XML configuration file **examples/keccak/chars/r4_w64_n512.xml**.


## Adding a Custom Callback to a Crypto Function

There are several callback functions that get executed at various points during the search.

The `found` function is called when the search is finished and has found a result. Additionally, one can specify a function that is called when the search is 
started or restarted (for example to further initialize the crypto) and each phase can have a function that is called when the phase is finished. (see [Search Options](#search-options))

The callback function has the following definition:
```cpp
bool Crypto::Callback(std::string function, Characteristic& characteristic, std::mt19937& rng, LogFile& logfile);
```

The `function` option is the string that identifies the function to be called, `characteristic` is the current state of the search, `rng`is the current state of the
random number generator, and `logfile` is the logfile for output.

As an example, for Ascon, there is a custom callback for `found` that calculates the number of active sboxes of the characteristic and only outputs it if it is lower than the last one.

> examples/ascon/ascon.cpp
>```cpp
>...
>bool Ascon::Callback(std::string function, Characteristic& characteristic, std::mt19937& rng, LogFile& logfile) {
>  if (function.empty())
>    return true;
>  else if (function.compare("found") == 0) {
>    uint64 cost =0;
>    uint64 trunc_diff[20];
>    uint64 trunc_cost[20];
>    for (int r=0; r<rounds_;r++) {
>        trunc_diff[r] = characteristic.GetConditionWordMask([](BitCondition bc) { return bc == BitCondition("1"); },
>                                                            characteristic.GetCrypto()->GetConditionWordIndex("S", r));
>        trunc_cost[r] = nldtool::HammingWeight(trunc_diff[r]);
>        cost += trunc_cost[r];
>    }
>    if(cost < best_char_) {
>      logfile << "found new characteristic" << std::endl;
>      logfile << "cost: " << cost << std::endl;
>      characteristic.WriteCharacteristic(logfile);
>      best_char_ = cost;
>    }
>    return true;
>  }
>
>  return true;
>}
>```


## Enabling Probability Computation

We can also enable the computation of the probability of the characteristic during the search.

For that we need to define the step that is responsible for the probability of each `ConditionWord`.
The example code for MD4 follows:

> examples/md4/md4.cpp
>```cpp
>...
>Md4::Md4(cxxopts::Options& options)
>    : Crypto(options["word-size"].as<int>()),
>      num_rounds_(options["num-rounds"].as<int>()) {
>  Step* step = 0;
>
>  for (int i = -4; i < 0; i++)
>    A[i] = AddConditionWord("A", i, 4 + i, 0);
>  for (int i = 0; i < std::min(16, num_rounds_); i++)
>    W[i] = AddConditionWord("W", i, 4 + i * 2 + 1, 1);
>
>  for (int i = 0; i < num_rounds_; i++) {
>    F[i] = AddConditionWord("F", i, 4 + i * 2 + 0, 1, SUBWORD);
>    A[i] = AddConditionWord("A", i, 4 + i * 2 + 1, 0);
>
>    if (i < 16)
>      step = Add(new BitsliceStep<IF>(word_size_, A[i - 1], A[i - 2], A[i - 3], F[i]));
>    else if (i < 32)
>      step = Add(new BitsliceStep<MAJ>(word_size_, A[i - 1], A[i - 2], A[i - 3], F[i]));
>    else
>      step = Add(new BitsliceStep<XOR3>(word_size_, A[i - 1], A[i - 2], A[i - 3], F[i]));
>    F[i]->SetStepToComputeProbability(step);
>
>    int m = P[i];
>    ConditionWordPtr k(new ConditionWord(K[i / 16]));
>    step = Add(new CarryStep<ADD4>(word_size_, A[i - 4], F[i], W[m], k, A[i]->Rotr(S[(i / 16) * 4 + i % 4])));
>    step->SetProbabilityMethod(CYCLICGRAPH);
>    A[i]->SetStepToComputeProbability(step);
>  }
>}
>...
>```

We can see that 2 lines were added calling `SetStepToComputeProbability` for each `ConditionWord` that is part of the crypto function description. Also, for every function that has some internal carry state (like `ADD4` in the case of MD4), we set the method to calculate the probability to `CYCLICGRAPH` to improve accuracy. This is all that is required to compute the probability.

To enable the output of the probability during the search, we also need to enable the corresponding option in the print configuration file.

> print_config.xml
>```xml
>...
>    <flag name="probability" value="1"/>
>...
>```

The output prints the probability for each round, as well as the overall probability at the top right. Keep in mind that the probability is only displayed for steps with enabled output, so if you want to see the probability for `SUBWORD` steps, you need to enable them in the print configuration file (`<flag name="sub" value="1"/>`).


### Special Case: MD5

In the case of the hash function MD5, we have a slight problem with the description of the modular additions. Before the last value is added, the result of the previous additions is rotated by a round-dependant value `s`. This means we need to split the addition in two parts and in this process we lose some information about the propagation of conditions on the carry bits.

> examples/md5/md5.cpp
>```cpp
>...
>Md5::Md5(cxxopts::Options& options)
>    : Crypto(options["word-size"].as<int>()),
>      num_rounds_(options["num-rounds"].as<int>()),
>      A(tA + H) {
>  for (int i = -H; i < 0; i++)
>    A[i] = AddConditionWord("A", i, H + i, 0);
>  for (int i = 0; i < std::min(16, num_rounds_); i++)
>    W[i] = AddConditionWord("W", i, 4 + i * L + 1, 1);
>
>  Step* step;
>  for (int i = 0; i < num_rounds_; i++) {
>    int m;
>    if (i < 16)
>      m = i;
>    else if (i < 32)
>      m = (5 * i + 1) % 16;
>    else if (i < 48)
>      m = (3 * i + 5) % 16;
>    else /* if (i < 64) */
>      m = (7 * i) % 16;
>
>    B[i] = AddConditionWord("B", i, 4 + i * L + 0, 0, SUBWORD);
>    F[i] = AddConditionWord("F", i, 4 + i * L + 0, 1, SUBWORD);
>    A[i] = AddConditionWord("A", i, 4 + i * L + 1, 0);
>    ConditionWordPtr k(new ConditionWord(K[i]));
>    ConditionWordPtr c3o = AddConditionWord("Co", i, 4 + i * L + 3, 0, CARRYWORD, 3);
>
>    ConditionWordPtr c3i = c3o->Rotl(1)->SetZero(0, 1, 2)->SetZero(S[i], 2, 0);
>
>    if (i < 16)
>      step = Add(new BitsliceStep<IF>(word_size_, A[i - 1], A[i - 2], A[i - 3], F[i]));  // IF(X,Y,Z)
>    else if (i < 32)
>      step = Add(new BitsliceStep<IF>(word_size_, A[i - 3], A[i - 1], A[i - 2], F[i]));  // IF(Z,X,Y)
>    else if (i < 48)
>      step = Add(new BitsliceStep<XOR3>(word_size_, A[i - 1], A[i - 2], A[i - 3], F[i]));
>    else /* if (i < 64) */
>      step = Add(new BitsliceStep<ONX>(word_size_, A[i - 1], A[i - 2], A[i - 3], F[i]));
>    step->SetProbabilityMethod(BITSLICE);
>    F[i]->SetStepToComputeProbability(step);
>
>    ConditionWordPtr cb = AddReferenceToConditionWord(c3o->Rotr(S[i]), "CB", i, 4 + i * L + 2, 1, CARRYWORD, 2, 0);
>    ConditionWordPtr ca = AddReferenceToConditionWord(c3o, "CA", i, 4 + i * L + 2, 0, CARRYWORD, 1, 2);
>    Add(new BitsliceStep<ADD4>(word_size_, A[i - 4], F[i], W[m], k, cb->Shl(1), B[i], cb));
>    Add(new BitsliceStep<ADD2>(word_size_, B[i]->Rotl(S[i]), A[i - 1], ca->Shl(1), A[i], ca));
>  }
>}
>...
>```

To improve the propagation of this round, a function `ADD4ADD2` was defined for MD5 and added to the hash function description, essentially removing a
lot of false positives that would only be recognized late in the search process.

>examples/md5/md5.cpp
>```cpp
>...
>class Md5::ADD4ADD2: public F {
>public:
>  static const int IN = 6;
>  static const int OUT = 3;
>  static const int NUM = IN + OUT;
>  static const int STATE_IN = 5;
>  static const int STATE_OUT = 8;
>  static const int STATE_SIZE = 64;
>  static constexpr int SYM[16] = { 0, 0, 0, 0, 1, 2, 3, 4, 5 };
>  static constexpr int SIZE[16] = { 1, 1, 1, 1, 1, 3, 1, 1, 3 };
>  static constexpr char NAME[] = "ADD4ADD2";
>  template<class T> static inline void f(T x[NUM]) {
>    const int ci = IN - 1;
>    const int sb = IN;
>    const int sa = IN + 1;
>    const int co = IN + 2;
>    x[sb] = x[0] + x[1] + x[2] + x[3] + (x[ci] % 4);
>    x[co] = x[sb] >> 1;
>    x[sb] &= 1;
>    x[sa] = x[sb] + x[4] + (x[ci] / 4);
>    x[co] |= (x[sa] >> 1) * 4;
>    x[sa] &= 1;
>  }
>};
>...
>```


## Hash Function Results

### SHA-2

Florian Mendel, Tomislav Nad, Martin Schläffer: Finding SHA-2 Characteristics: Searching through a Minefield of Contradictions. ASIACRYPT 2011 https://doi.org/10.1007/978-3-642-25385-0_16

Florian Mendel, Tomislav Nad, Martin Schläffer: Improving Local Collisions: New Attacks on Reduced SHA-256. EUROCRYPT 2013 https://doi.org/10.1007/978-3-642-38348-9_16

Maria Eichlseder, Florian Mendel, Tomislav Nad, Vincent Rijmen, Martin Schläffer: Linear Propagation in Efficient Guess-and-Determine Attacks: WCC 2013 http://www.selmer.uib.no/WCC2013/pdfs/Eichlseder.pdf

Maria Eichlseder, Florian Mendel, Martin Schläffer: Branching Heuristics in Differential Collision Search with Applications to SHA-512. FSE 2014 https://doi.org/10.1007/978-3-662-46706-0_24

Christoph Dobraunig, Maria Eichlseder, Florian Mendel: Analysis of SHA-512/224 and SHA-512/256. ASIACRYPT 2015 https://doi.org/10.1007/978-3-662-48800-3_25


### SHA-1

Ange Albertini, Jean-Philippe Aumasson, Maria Eichlseder, Florian Mendel, Martin Schläffer: Malicious Hashing: Eve's Variant of SHA-1. SAC 2014 https://doi.org/10.1007/978-3-319-13051-4_1


### Keccak

Stefan Kölbl, Florian Mendel, Tomislav Nad, Martin Schläffer: Differential Cryptanalysis of Keccak Variants. IMA 2013 https://doi.org/10.1007/978-3-642-45239-0_9


### RIPEMD-128/160

Florian Mendel, Tomislav Nad, Martin Schläffer: Collision Attacks on the Reduced Dual-Stream Hash Function RIPEMD-128. FSE 2012 https://doi.org/10.1007/978-3-642-34047-5_14

Florian Mendel, Thomas Peyrin, Martin Schläffer, Lei Wang, Shuang Wu: Improved Cryptanalysis of Reduced RIPEMD-160. ASIACRYPT 2013 https://doi.org/10.1007/978-3-642-42045-0_25

Florian Mendel, Tomislav Nad, Stefan Scherz, Martin Schläffer: Differential Attacks on Reduced RIPEMD-160. ISC 2012 https://doi.org/10.1007/978-3-642-33383-5_2


### SM3

Florian Mendel, Tomislav Nad, Martin Schläffer: Finding Collisions for Round-Reduced SM3. CT-RSA 2013 https://doi.org/10.1007/978-3-642-36095-4_12


### HAS-160

Florian Mendel, Tomislav Nad, Martin Schläffer: Cryptanalysis of Round-Reduced HAS-160. ICISC 2011 https://doi.org/10.1007/978-3-642-31912-9_3


### Others

Christoph Dobraunig, Florian Mendel, Martin Schläffer: Differential Cryptanalysis of SipHash. SAC 2014 https://doi.org/10.1007/978-3-319-13051-4_10

Christoph Dobraunig, Maria Eichlseder, Florian Mendel: Forgery Attacks on Round-Reduced ICEPOLE-128. SAC 2015 https://doi.org/10.1007/978-3-319-31301-6_27

Christoph Dobraunig, Maria Eichlseder, Florian Mendel, Martin Schläffer: Cryptanalysis of Ascon. CT-RSA 2015 https://doi.org/10.1007/978-3-319-16715-2_20


## Related Work


### SHA-1 Tool

Christophe De Cannière, Christian Rechberger: Finding SHA-1 Characteristics: General Results and Applications. ASIACRYPT 2006 https://doi.org/10.1007/11935230_1

Christophe De Cannière, Florian Mendel, Christian Rechberger: Collisions for 70-round SHA-1: On the Full Cost of Collision Search. SAC 2007 https://doi.org/10.1007/978-3-540-77360-3_4


### ARX-Tools

Gaëtan Leurent: Analysis of Differential Attacks in ARX Constructions. ASIACRYPT 2012 https://doi.org/10.1007/978-3-642-34961-4_15

Gaëtan Leurent: Construction of Differential Characteristics in ARX Designs Application to Skein. CRYPTO 2013 https://doi.org/10.1007/978-3-642-40041-4_14


### Probability Computation

Nicky Mouha, Vesselin Velichkov, Christophe De Cannière, Bart Preneel: The Differential Analysis of S-Functions. SAC 2010 https://doi.org/10.1007/978-3-642-19574-7_3

Vesselin Velichkov, Nicky Mouha, Christophe De Cannière, Bart Preneel: The Additive Differential Probability of ARX. FSE 2011 https://doi.org/10.1007/978-3-642-21702-9_20

