#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <math.h>


typedef unsigned int uint32;
typedef unsigned long long uint64;

/* Shift-right (used in SHA-256, SHA-384, and SHA-512): */
#define R(b,x)    ((x) >> (b))
/* 32-bit Rotate-right (used in SHA-256): */
#define S32(b,x)  (((x) >> (b)) | ((x) << (32 - (b))))
/* 64-bit Rotate-right (used in SHA-384 and SHA-512): */
#define S64(b,x)  (((x) >> (b)) | ((x) << (64 - (b))))

/* Two of six logical functions used in SHA-256, SHA-384, and SHA-512: */
#define Ch(x,y,z) (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z)  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/* Four of six logical functions used in SHA-256: */
#define Sigma0_256(x) (S32(2,  (x)) ^ S32(13, (x)) ^ S32(22, (x)))
#define Sigma1_256(x) (S32(6,  (x)) ^ S32(11, (x)) ^ S32(25, (x)))
#define sigma0_256(x) (S32(7,  (x)) ^ S32(18, (x)) ^ R(3 ,   (x)))
#define sigma1_256(x) (S32(17, (x)) ^ S32(19, (x)) ^ R(10,   (x)))

/* Four of six logical functions used in SHA-384 and SHA-512: */
#define Sigma0_512(x) (S64(28, (x)) ^ S64(34, (x)) ^ S64(39, (x)))
#define Sigma1_512(x) (S64(14, (x)) ^ S64(18, (x)) ^ S64(41, (x)))
#define sigma0_512(x) (S64( 1, (x)) ^ S64( 8, (x)) ^ R( 7,   (x)))
#define sigma1_512(x) (S64(19, (x)) ^ S64(61, (x)) ^ R( 6,   (x)))



const static uint32 K256[64] = {
  0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
  0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
  0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
  0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
  0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
  0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
  0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
  0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
  0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
  0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
  0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
  0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
  0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
  0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
  0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
  0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

/* Hash constant words K for SHA-384 and SHA-512: */
const static uint64 K512[80] = {
  0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL,
  0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
  0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
  0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
  0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
  0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
  0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL,
  0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
  0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
  0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
  0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL,
  0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
  0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL,
  0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
  0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
  0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
  0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL,
  0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
  0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL,
  0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
  0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
  0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
  0xd192e819d6ef5218ULL, 0xd69906245565a910ULL,
  0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
  0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
  0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
  0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
  0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
  0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL,
  0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
  0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL,
  0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
  0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
  0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
  0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
  0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
  0x28db77f523047d84ULL, 0x32caab7b40c72493ULL,
  0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
  0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
  0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};


void SHA256(uint32 state[][8], uint32 W256[]) {
  uint32 a, b, c, d, e, f, g, h, s0, s1;
  uint32 T1, T2;
  int j;

  /* Initialize registers with the prev. intermediate value */
  a = state[0][0];
  b = state[0][1];
  c = state[0][2];
  d = state[0][3];
  e = state[0][4];
  f = state[0][5];
  g = state[0][6];
  h = state[0][7];

  j = 0;
  do {
    T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + W256[j];
    T2 = Sigma0_256(a) + Maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;
    j++;
    state[j][0] = a;
    state[j][1] = b;
    state[j][2] = c;
    state[j][3] = d;
    state[j][4] = e;
    state[j][5] = f;
    state[j][6] = g;
    state[j][7] = h;
  } while (j < 16);

  do {
    /* Part of the message block expansion: */
    s0 = W256[(j-15)];
    s0 = sigma0_256(s0);
    s1 = W256[(j-2)];
    s1 = sigma1_256(s1);

    /* Apply the SHA-256 compression function to update a..h */
    T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] +
         (W256[j] = s1 + W256[(j-7)] + s0 + W256[(j-16)]);
    T2 = Sigma0_256(a) + Maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;
    j++;
    state[j][0] = a;
    state[j][1] = b;
    state[j][2] = c;
    state[j][3] = d;
    state[j][4] = e;
    state[j][5] = f;
    state[j][6] = g;
    state[j][7] = h;
  } while (j < 64);

  /* Compute the current intermediate hash value */
  state[65][0] += a;
  state[65][1] += b;
  state[65][2] += c;
  state[65][3] += d;
  state[65][4] += e;
  state[65][5] += f;
  state[65][6] += g;
  state[65][7] += h;

}

void SHA512(uint64 state[][8], uint64 W512[]) {
  uint64 a, b, c, d, e, f, g, h, s0, s1;
  uint64 T1, T2;
  int j;

  /* Initialize registers with the prev. intermediate value */
  a = state[0][0];
  b = state[0][1];
  c = state[0][2];
  d = state[0][3];
  e = state[0][4];
  f = state[0][5];
  g = state[0][6];
  h = state[0][7];

  j = 0;
  do {
    T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + W512[j];
    T2 = Sigma0_512(a) + Maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;
    j++;
    state[j][0] = a;
    state[j][1] = b;
    state[j][2] = c;
    state[j][3] = d;
    state[j][4] = e;
    state[j][5] = f;
    state[j][6] = g;
    state[j][7] = h;
  } while (j < 16);

  do {
    /* Part of the message block expansion: */
    s0 = W512[(j-15)];
    s0 = sigma0_512(s0);
    s1 = W512[(j-2)];
    s1 = sigma1_512(s1);

    /* Apply the SHA-256 compression function to update a..h */
    T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] +
         (W512[j] = s1 + W512[(j-7)] + s0 + W512[(j-16)]);
    T2 = Sigma0_512(a) + Maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;
    j++;
    state[j][0] = a;
    state[j][1] = b;
    state[j][2] = c;
    state[j][3] = d;
    state[j][4] = e;
    state[j][5] = f;
    state[j][6] = g;
    state[j][7] = h;
  } while (j < 80);

  /* Compute the current intermediate hash value */
  state[81][0] += a;
  state[81][1] += b;
  state[81][2] += c;
  state[81][3] += d;
  state[81][4] += e;
  state[81][5] += f;
  state[81][6] += g;
  state[81][7] += h;

}


void printWord(uint32 a, uint32 b) {
  uint32 c= a^b;
  for (int i=0; i<32;i++) {
    if ((c & 0x80000000UL) == 0) {
      printf("%d", (a>>31) & 1);
    } else {
      if ((a & 0x80000000UL) == 0) {
        printf("n");
      } else {
        printf("u");
      }
    }
    a <<=1;
    c <<=1;
  }
}

void printWord(uint64 a, uint64 b) {
  uint64 c= a^b;
  for (int i=0; i<64;i++) {
    if ((c & 0x8000000000000000UL) == 0) {
      printf("%d", (a>>63) & 1);
    } else {
      if ((a & 0x8000000000000000UL) == 0) {
        printf("n");
      } else {
        printf("u");
      }
    }
    a <<=1;
    c <<=1;
  }
}

void testSHA256() {
  uint32 W1[64] = {0};
  uint32 W2[64] = {0};

  uint32 S1[66][8];
  uint32 S2[66][8];

  for (int i=0;i<16;i++) {
    W1[i] = random();
    W2[i] = random();
  }

  for (int i=0;i<8;i++) {
    S1[0][i] = random();
    S2[0][i] = random();
  }

  SHA256(S1,W1);
  SHA256(S2,W2);

  for (int i=0;i<4;i++) {
    printf("A: ");
    printWord(S1[0][3-i],S2[0][3-i]);
    printf(" | E: ");
    printWord(S1[0][7-i],S2[0][7-i]);
    printf(" | \n");
  }

  for (int i=1;i<=64;i++) {
    printf("A: ");
    printWord(S1[i][0],S2[i][0]);
    printf(" | E: ");
    printWord(S1[i][4],S2[i][4]);
    printf(" | W: ");
    printWord(W1[i-1],W2[i-1]);
    printf(" \n");
  }
}


void testSHA512() {
  uint64 W1[80] = {0};
  uint64 W2[80] = {0};

  uint64 S1[82][8];
  uint64 S2[82][8];

  for (int i=0;i<16;i++) {
    W1[i] = random();
    W1[i] <<= 32;
    W1[i] ^= random();
    W2[i] = random();
    W2[i] <<= 32;
    W2[i] ^= random();
  }

  for (int i=0;i<8;i++) {
    S1[0][i] = random();
    S1[0][i] <<= 32;
    S1[0][i] ^= random();
    S2[0][i] = random();
    S2[0][i] <<= 32;
    S2[0][i] ^= random();
  }

  SHA512(S1,W1);
  SHA512(S2,W2);

  for (int i=0;i<4;i++) {
    printf("A: ");
    printWord(S1[0][3-i],S2[0][3-i]);
    printf(" | E: ");
    printWord(S1[0][7-i],S2[0][7-i]);
    printf(" | \n");
  }

  for (int i=1;i<=64;i++) {
    printf("A: ");
    printWord(S1[i][0],S2[i][0]);
    printf(" | E: ");
    printWord(S1[i][4],S2[i][4]);
    printf(" | W: ");
    printWord(W1[i-1],W2[i-1]);
    printf(" \n");
  }
}

template <int M, int N>
int rank(unsigned int matrix[M][N])
{
  int r = 0;

  for (int m = 0; m < M; ++m)
    for (int n = 0; n < N; ++n)
      {
  const int x = matrix[m][n];

  if (x)
    {
      const int mask = x & ~(x - 1);

      for (int i = m + 1; i < M; ++i)
        if (matrix[i][n] & mask)
    for (int j = n; j < N; ++j)
      matrix[i][j] ^= matrix[m][j];

      ++r;

      break;
    }
      }

  return r;
}


void testSigma() {

  unsigned int matrix[32][2];

  for (int i=0;i<32;i++) {
    matrix[i][0] = sigma0_256(1ul<<i);
    for (int j=0;j<32;j++) {
      printf("%d, ",(matrix[i][0] >> j) & 1ul);
    }
    printf("\n");
  }

  //printf("rank=%d",rank<32,2>(matrix));

}

unsigned int HammingWeight(unsigned int v) {
  unsigned int c; // c accumulates the total bits set in v

  // option 3, for at most 32-bit values in v:
  c =  ((v & 0xfff) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;
  c += (((v & 0xfff000) >> 12) * 0x1001001001001ULL & 0x84210842108421ULL) %
       0x1f;
  c += ((v >> 24) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;

  return c;
}

void printchar(unsigned int mask, int r) {

  for (int i=-4;i<0;i++) {
    std::cout << i << " A: " << std::setfill('-') << std::setw(32) << "-" << std::setfill(' ') << std::setw(1);
    std::cout << " E: " << std::setfill('-') << std::setw(32) << "-" << std::setfill(' ') << std::setw(1) << std::endl;
  }

  for (int i=0;i<r-4;i++) {
    std::cout << std::setw(2) << i  << " A: " << std::setfill('?') << std::setw(32) << "?" << std::setfill(' ') << std::setw(1);
    std::cout << " E: " << std::setfill('?') << std::setw(32) << "?" << std::setfill(' ') << std::setw(1);
    if ((mask >> i) & 1) {
      std::cout << " W: " << std::setfill('?') << std::setw(32) << "?" << std::setfill(' ') << std::setw(1) << std::endl;
    } else {
      std::cout << " W: " << std::setfill('-') << std::setw(32) << "-" << std::setfill(' ') << std::setw(1) << std::endl;
    }
  }

  for (int i=r-4;i<r;i++) {
    std::cout << std::setw(2) << i << " A: " << std::setfill('-') << std::setw(32) << "-" << std::setfill(' ') << std::setw(1);
    std::cout << " E: " << std::setfill('-') << std::setw(32) << "-" << std::setfill(' ') << std::setw(1);
    if ((mask >> i) & 1) {
      std::cout << " W: " << std::setfill('?') << std::setw(32) << "?" << std::setfill(' ') << std::setw(1) << std::endl;
    } else {
      std::cout << " W: " << std::setfill('-') << std::setw(32) << "-" << std::setfill(' ') << std::setw(1) << std::endl;
    }
  }

}

int main() {

  srandom(time(0));
  int start =0;

  int max = 0;
  int max_index = 0;
  int tmp;

//  printchar(0x120e6e00,40);
//  return 1;

  int x = 5;

  for (int t=1; t< 1 << 16; t++) {
    for (int l=0;l< 1<<x; l++) {
      tmp=0;
      int cond =0;
      int min =64;
      int tW[128] = {0};
      int *W = &tW[64];
      for (int j=0;j<16;j++) {
        W[j] =0;
        W[j] = (t >> j) & 1;
        tmp |= ((t >> j) & 1) << j;
      }
      for (int j=16; j<64;j++) {
        int hw = W[j-16] + W[j-15] + W[j-7] + W[j-2];
        if (j<(16+x)) {
          if (hw > 1) {
            W[j] = ((l >> (16-j)) & 1);
            if (W[j] == 0) cond ++;
          } else W[j] =hw;
          if (W[j] ==1) tmp |= 1 << j;
        } else if (hw==1) {
          W[j] = 1;
          min = std::min(j,min);
          //printf("%d %d %d %08x\n",t,l,min,t);
          break;
        } else {
          if (hw>1) cond++;
        }
      }

      for (int j=15; j>=-32;j--) {
        int hw = W[j] + W[j-15] + W[j-7] + W[j-2];
        if (hw==1) {
          W[j-16] = 1;
          //printf("%d %d %d %08x\n\n",t,l,min,t);
          break;
        } else {
          W[j-16] =0;
          tmp<<=1;
          min++;
          if (hw>1) cond++;
        }
      }


      //if (min >= max) {
      if (min >= 38) {
        max = min;
        max_index=t;
        std::cout << max << ": " << std::hex << tmp << std::dec << " " << cond << std::endl;

      }
    }
  }




}
