// Xoroshiro1024++
// Quelle: http://prng.di.unimi.it/xoroshiro1024plusplus.c 
// http://prng.di.unimi.it/ 

 
/*  Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#include <stdint.h>
#include <string.h>

/* This is xoroshiro1024++ 1.0, one of our all-purpose, rock-solid,
   large-state generators. It is extremely fast and it passes all
   tests we are aware of. Its state however is too large--in general, 
	the xoshiro256 family should be preferred.

   For generating just floating-point numbers, xoroshiro1024* is even
   faster (but it has a very mild bias, see notes in the comments).

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s. 
   
   *splitmix64:
   uint64_t x; /* The state can be seeded with any value. 
   uint64_t next() {
      uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
	  z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	  z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	  return z ^ (z >> 31);
   }
   
   *alternative:
   s[0] = millis();  // say 
   for( int i = 1; i < 16; ++i ) {
      s[i] = UINT64_C(6364136223846793005) * s[i-1] + UINT64_C(1442695040888963407);
   }
   

   */


static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}


static int p;
static uint64_t s[16];

uint64_t seed; 



uint64_t splitmix64() {
      uint64_t z = (seed += UINT64_C(0x9E3779B97F4A7C15));
	  z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	  z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	  return z ^ (z >> 31);
}



//***************************************************************************
// PRNG randomize

uint64_t next(void) {
	const int q = p;
	const uint64_t s0 = s[p = (p + 1) & 15];
	uint64_t s15 = s[q];
	
	const uint64_t result = rotl(s0 + s15, 23) + s15;

	s15 ^= s0;
	s[q] = rotl(s0, 25) ^ s15 ^ (s15 << 27);
	s[p] = rotl(s15, 36);

	return result;
}

//***************************************************************************
// PRNG randomize, alias

uint64_t randXs(void) {
	return next(void);
}


//***************************************************************************
// PRNG seed

uint64_t srandXs {
   seed = millis(); // say
   s[0] = seed;
   for( int i = 1; i < 16; ++i ) {
      s[i] = UINT64_C(6364136223846793005) * s[i-1] + UINT64_C(1442695040888963407);
   }
}



//***************************************************************************
// PRNG jump randomize

/* This is the jump function for the generator. It is equivalent
   to 2^512 calls to next(); it can be used to generate 2^512
   non-overlapping subsequences for parallel computations. */

void jump() {
	static const uint64_t JUMP[] = { 0x931197d8e3177f17,
		0xb59422e0b9138c5f, 0xf06a6afb49d668bb, 0xacb8a6412c8a1401,
		0x12304ec85f0b3468, 0xb7dfe7079209891e, 0x405b7eec77d9eb14,
		0x34ead68280c44e4a, 0xe0e4ba3e0ac9e366, 0x8f46eda8348905b7,
		0x328bf4dbad90d6ff, 0xc8fd6fb31c9effc3, 0xe899d452d4b67652,
		0x45f387286ade3205, 0x03864f454a8920bd, 0xa68fa28725b1b384 };

	uint64_t t[sizeof s / sizeof *s];
	memset(t, 0, sizeof t);
	for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
		for(int b = 0; b < 64; b++) {
			if (JUMP[i] & UINT64_C(1) << b)
				for(int j = 0; j < sizeof s / sizeof *s; j++)
					t[j] ^= s[(j + p) & sizeof s / sizeof *s - 1];
			next();
		}

	for(int i = 0; i < sizeof s / sizeof *s; i++) {
		s[(i + p) & sizeof s / sizeof *s - 1] = t[i];
	}
}

/* This is the long-jump function for the generator. It is equivalent to
   2^768 calls to next(); it can be used to generate 2^256 starting points,
   from each of which jump() will generate 2^256 non-overlapping
   subsequences for parallel distributed computations. */

void long_jump(void) {
	static const uint64_t LONG_JUMP[] = { 0x7374156360bbf00f,
		0x4630c2efa3b3c1f6, 0x6654183a892786b1, 0x94f7bfcbfb0f1661,
		0x27d8243d3d13eb2d, 0x9701730f3dfb300f, 0x2f293baae6f604ad,
		0xa661831cb60cd8b6, 0x68280c77d9fe008c, 0x50554160f5ba9459,
		0x2fc20b17ec7b2a9a, 0x49189bbdc8ec9f8f, 0x92a65bca41852cc1,
		0xf46820dd0509c12a, 0x52b00c35fbf92185, 0x1e5b3b7f589e03c1 };

	uint64_t t[sizeof s / sizeof *s];
	memset(t, 0, sizeof t);
	for(int i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
		for(int b = 0; b < 64; b++) {
			if (LONG_JUMP[i] & UINT64_C(1) << b)
				for(int j = 0; j < sizeof s / sizeof *s; j++)
					t[j] ^= s[(j + p) & sizeof s / sizeof *s - 1];
			next();
		}

	for(int i = 0; i < sizeof s / sizeof *s; i++) {
		s[(i + p) & sizeof s / sizeof *s - 1] = t[i];
	}
}