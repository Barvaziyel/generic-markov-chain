# generic-markov-chain-and-tweet-generator
Generic Markov Chain, used for generating pseudorandom sequences from input, and an example program that uses it.

## tweets_generator usage
Run the program with the following arguments:
```
<Seed> <No. of tweets to generate> <Path to input txt file> <Optional - Number of words to read from input txt file>
```
For example, with the following arguments the program will read `420` words from `tweets.txt` and then print `42` randomly generated tweets using the seed  `3`:
```
tweets_generator.exe 3 42 tweets.txt 420
```

Example input txt file included.
