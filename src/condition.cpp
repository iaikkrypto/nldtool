#include "condition.h"

const Condition<1>::Cond2Char Condition<1>::COND2CHAR;
const Condition<1>::Char2Cond Condition<1>::CHAR2COND;
Condition<1>::Cond2Char::Cond2Char() {
  static const char chars[] = "#0u3n5x71-ABCDE?";
  for (int i = 0; i < 16; i++) cond2char[i] = chars[i];
}
Condition<1>::Char2Cond::Char2Cond() {
  for (int i = 0; i < 256; i++) char2cond[i] = 255;
  for (int i = 0; i < 16; i++)
    char2cond[static_cast<uint8_t>(Condition<1>::COND2CHAR(i))] = i;
}

const int Condition<1>::NUMPAIRS[16] = {0, 1, 1, 2, 1, 2, 2, 3,
                                        1, 2, 2, 3, 2, 3, 3, 4};
const int Condition<1>::PAIRS[16][4] = {{},
                                        {0x00},
                                        {0x01},
                                        {0x00, 0x01},
                                        {0x02},
                                        {0x00, 0x02},
                                        {0x01, 0x02},
                                        {0x00, 0x01, 0x02},
                                        {0x03},
                                        {0x00, 0x03},
                                        {0x01, 0x03},
                                        {0x00, 0x01, 0x03},
                                        {0x02, 0x03},
                                        {0x00, 0x02, 0x03},
                                        {0x01, 0x02, 0x03},
                                        {0x00, 0x01, 0x02, 0x03}};

const int Condition<1>::PAIRS4[16][4] = {{},
                                         {0x00},
                                         {0x01},
                                         {0x00, 0x01},
                                         {0x10},
                                         {0x00, 0x10},
                                         {0x01, 0x10},
                                         {0x00, 0x01, 0x10},
                                         {0x11},
                                         {0x00, 0x11},
                                         {0x01, 0x11},
                                         {0x00, 0x01, 0x11},
                                         {0x10, 0x11},
                                         {0x00, 0x10, 0x11},
                                         {0x01, 0x10, 0x11},
                                         {0x00, 0x01, 0x10, 0x11}};

// static const int onebit_to_twobit_cond[][] = { { 0, 1, 4, 5 }, { 2, 3, 6, 7
// }, { 8, 9, 12, 13 }, { 10, 11, 14, 15 } }; static const int
// twobit_to_one_bit[] = { 0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 3, 3 };

void Condition<2>::MergeBit0(Condition<1> cond) {
  uint16_t bitmask[4] = {0xfafa, 0xf5f5, 0xafaf, 0x5f5f};
  for (int i = 0; i < 4; i++)
    if (((cond >> i) & 1) == 0) condition_ &= bitmask[i];
}

void Condition<2>::MergeBit1(Condition<1> cond) {
  uint16_t bitmask[4] = {0xffcc, 0xff33, 0xccff, 0x33ff};
  for (int i = 0; i < 4; i++)
    if (((cond >> i) & 1) == 0) condition_ &= bitmask[i];
}

// 11,1n,n1,nn,1u,10,nu,n0,u1,un,01,0n,uu,u0,0u,00
// 3,2,3,2,1,0,1,0,3,2,3,2,1,0,1,0
// x,x,3,2,x,x,1,0,x,x,3,2,x,x,1,0
Condition<1> Condition<2>::GetBit0() {
  uint16_t help = condition_;
  help |= help >> 2;
  help |= help >> 8;
  uint64_t bc1 = (help & 0x3) | ((help >> 2) & 0xc);
  return Condition<1>(bc1);
}

Condition<1> Condition<2>::GetBit1() {
  uint64_t bc1 = 0;
  uint16_t help = 0;
  help = (condition_ >> 4) | condition_;
  help &= 0x0f0f;
  help |= help >> 4;
  help |= help >> 1;
  bc1 |= 1 & help;
  bc1 |= 2 & (help >> 1);
  bc1 |= 4 & (help >> 2);
  bc1 |= 8 & (help >> 3);
  return Condition<1>(bc1);
}

void Condition<3>::MergeBit0(Condition<1> cond) {
  const uint64_t bitmask[4] = {0xffaaffaaffaaffaaull, 0xff55ff55ff55ff55ull,
                               0xaaffaaffaaffaaffull, 0x55ff55ff55ff55ffull};
  for (int i = 0; i < 4; i++)
    if (((cond >> i) & 1) == 0) condition_ &= bitmask[i];
}

void Condition<3>::MergeBit1(Condition<1> cond) {
  const uint64_t bitmask[4] = {0xffffccccffffccccull, 0xffff3333ffff3333ull,
                               0xccccffffccccffffull, 0x3333ffff3333ffffull};
  for (int i = 0; i < 4; i++)
    if (((cond >> i) & 1) == 0) condition_ &= bitmask[i];
}

void Condition<3>::MergeBit2(Condition<1> cond) {
  const uint64_t bitmask[4] = {0xfffffffff0f0f0f0ull, 0xffffffff0f0f0f0full,
                               0xf0f0f0f0ffffffffull, 0x0f0f0f0fffffffffull};
  for (int i = 0; i < 4; i++)
    if (((cond >> i) & 1) == 0) condition_ &= bitmask[i];
}

void Condition<3>::MergeBits01(Condition<2> cond) {
  const uint64_t bitmask[16] = {
      0xffffffeeffffffeeull, 0xffffffddffffffddull, 0xffffffbbffffffbbull,
      0xffffff77ffffff77ull, 0xffffeeffffffeeffull, 0xffffddffffffddffull,
      0xffffbbffffffbbffull, 0xffff77ffffff77ffull, 0xffeeffffffeeffffull,
      0xffddffffffddffffull, 0xffbbffffffbbffffull, 0xff77ffffff77ffffull,
      0xeeffffffeeffffffull, 0xddffffffddffffffull, 0xbbffffffbbffffffull,
      0x77ffffff77ffffffull};
  for (int i = 0; i < 16; i++)
    if (((cond >> i) & 1) == 0) condition_ &= bitmask[i];
}

void Condition<3>::MergeBits12(Condition<2> cond) {
  const uint64_t bitmask[16] = {
      0xfffffffffffffcfcull, 0xfffffffffffff3f3ull, 0xffffffffffffcfcfull,
      0xffffffffffff3f3full, 0xfffffffffcfcffffull, 0xfffffffff3f3ffffull,
      0xffffffffcfcfffffull, 0xffffffff3f3fffffull, 0xfffffcfcffffffffull,
      0xfffff3f3ffffffffull, 0xffffcfcfffffffffull, 0xffff3f3fffffffffull,
      0xfcfcffffffffffffull, 0xf3f3ffffffffffffull, 0xcfcfffffffffffffull,
      0x3f3fffffffffffffull};
  for (int i = 0; i < 16; i++)
    if (((cond >> i) & 1) == 0) condition_ &= bitmask[i];
}

Condition<1> Condition<3>::GetBit0() {
  uint64_t bc1 = 0;
  uint64_t help = condition_;
  help |= help >> 2;
  help |= help >> 4;
  help |= help >> 16;
  help |= help >> 32;
  bc1 = (help & 0x3) | ((help >> 6) & 0xc);
  return Condition<1>(bc1);
}

Condition<1> Condition<3>::GetBit1() {
  uint64_t bc1 = 0;
  uint64_t help = condition_;
  help |= help >> 1;
  help |= help >> 4;
  help |= help >> 8;
  help |= help >> 32;
  help = (help & 0x10001ull) | ((help & 0x40004ull) >> 1);
  bc1 = (help & 0x3) | ((help >> 14) & 0xc);
  return Condition<1>(bc1);
}

Condition<1> Condition<3>::GetBit2() {
  uint64_t bc1 = 0;
  uint64_t help = condition_;
  help |= help >> 1;
  help |= help >> 2;
  help |= help >> 8;
  help |= help >> 16;
  help = (help & 0x100000001ull) | ((help & 0x1000000010ull) >> 3);
  bc1 = (help & 0x3) | ((help >> 30) & 0xc);
  return Condition<1>(bc1);
}

Condition<2> Condition<3>::GetBits01() {
  uint64_t bc2 = 0;
  uint64_t help = condition_;
  help |= help >> 4;
  help |= help >> 32;
  help = (help & 0xf000full) | ((help & 0xf000f00ull) >> 4);
  bc2 = (help & 0xffull) | ((help & 0xff0000ull) >> 8);
  return Condition<2>(bc2);
}

Condition<2> Condition<3>::GetBits12() {
  uint64_t bc2 = 0;
  uint64_t help = condition_;
  help |= help >> 1;
  help |= help >> 8;
  help = (help & 0x1000100010001ull) | ((help & 0x54005400540054ull) >> 1);
  help = (help & 0x3000300030003ull) | ((help & 0x28002800280028ull) >> 1);
  help = (help & 0x7000700070007ull) | ((help & 0x10001000100010ull) >> 1);
  help = (help & 0xf0000000full) | ((help & 0xf0000000f0000ull) >> 12);
  bc2 = (help & 0xffull) | ((help >> 24) & 0xff00);
  return Condition<2>(bc2);
}
