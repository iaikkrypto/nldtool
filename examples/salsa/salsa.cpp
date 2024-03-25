#include "salsa/salsa.h"

#include "bitslice_step.h"
#include "carry_step.h"
#include "functions.h"

void Salsa::AddToOptions(cxxopts::Options& options) {}

Salsa::Salsa(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()) {
  for (int i = 0; i < num_rounds_ + 1; i++) {
    for (int c = 0; c < 16; c++) {
      // A2[i][c] = AddConditionWord("A2", i, -1, -1, HORIZONTALWORD);
      // B2[i][c] = AddConditionWord("X2", i, -1, -1, HORIZONTALWORD);
      // A[i][c] = AddReferenceToConditionWord(A2[i][c], "A", i, 2 * i*4 + c/4,
      // c%4, MAINWORD,1); B[i][c] = AddReferenceToConditionWord(B2[i][c], "X",
      // i, 2 * i*4 + 4 + c/4, c%4,SUBWORD,1);

      A[i][c] = AddConditionWord("A", i, 2 * i * 4 + c / 4, c % 4);
      B[i][c] = AddConditionWord("X", i, 2 * i * 4 + 4 + c / 4, c % 4, SUBWORD);
    }
  }

  for (int i = 1; i < num_rounds_ + 1; i++) {
    if ((i & 1) == 0) {
      // (z0, z1, z2, z3) = quarterround(y0, y1, y2, y3)
      Add(new CarryStep<ADD<2>>(word_size_, A[i - 1][0], A[i - 1][3],
                                B[i - 1][1]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][1],
                                   B[i - 1][1]->Rotl(7), A[i][1]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][1], A[i - 1][0], B[i - 1][2]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][2],
                                   B[i - 1][2]->Rotl(9), A[i][2]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][2], A[i][1], B[i - 1][3]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][3],
                                   B[i - 1][3]->Rotl(13), A[i][3]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][3], A[i][2], B[i - 1][0]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][0],
                                   B[i - 1][0]->Rotl(18), A[i][0]));

      // (z5, z6, z7, z4) = quarterround(y5, y6, y7, y4)
      Add(new CarryStep<ADD<2>>(word_size_, A[i - 1][5], A[i - 1][4],
                                B[i - 1][6]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][6],
                                   B[i - 1][6]->Rotl(7), A[i][6]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][6], A[i - 1][5], B[i - 1][7]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][7],
                                   B[i - 1][7]->Rotl(9), A[i][7]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][7], A[i][6], B[i - 1][4]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][4],
                                   B[i - 1][4]->Rotl(13), A[i][4]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][4], A[i][7], B[i - 1][5]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][5],
                                   B[i - 1][5]->Rotl(18), A[i][5]));

      //(z10, z11, z8, z9) = quarterround(y10, y11, y8, y9)
      Add(new CarryStep<ADD<2>>(word_size_, A[i - 1][10], A[i - 1][9],
                                B[i - 1][11]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][11],
                                   B[i - 1][11]->Rotl(7), A[i][11]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][11], A[i - 1][10],
                                B[i - 1][8]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][8],
                                   B[i - 1][8]->Rotl(9), A[i][8]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][8], A[i][11], B[i - 1][9]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][9],
                                   B[i - 1][9]->Rotl(13), A[i][9]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][9], A[i][8], B[i - 1][10]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][10],
                                   B[i - 1][10]->Rotl(18), A[i][10]));

      //(z15, z12, z13, z14) = quarterround(y15, y12, y13, y14)
      Add(new CarryStep<ADD<2>>(word_size_, A[i - 1][15], A[i - 1][14],
                                B[i - 1][12]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][12],
                                   B[i - 1][12]->Rotl(7), A[i][12]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][12], A[i - 1][15],
                                B[i - 1][13]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][13],
                                   B[i - 1][13]->Rotl(9), A[i][13]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][13], A[i][12], B[i - 1][14]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][14],
                                   B[i - 1][14]->Rotl(13), A[i][14]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][14], A[i][13], B[i - 1][15]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][15],
                                   B[i - 1][15]->Rotl(18), A[i][15]));

    } else {
      //(y0, y4, y8, y12) = quarterround(x0, x4, x8, x12)
      Add(new CarryStep<ADD<2>>(word_size_, A[i - 1][0], A[i - 1][12],
                                B[i - 1][4]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][4],
                                   B[i - 1][4]->Rotl(7), A[i][4]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][4], A[i - 1][0], B[i - 1][8]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][8],
                                   B[i - 1][8]->Rotl(9), A[i][8]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][8], A[i][4], B[i - 1][12]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][12],
                                   B[i - 1][12]->Rotl(13), A[i][12]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][12], A[i][8], B[i - 1][0]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][0],
                                   B[i - 1][0]->Rotl(18), A[i][0]));

      //(y5, y9, y13, y1) = quarterround(x5, x9, x13, x1)
      Add(new CarryStep<ADD<2>>(word_size_, A[i - 1][5], A[i - 1][1],
                                B[i - 1][9]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][9],
                                   B[i - 1][9]->Rotl(7), A[i][9]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][9], A[i - 1][5],
                                B[i - 1][13]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][13],
                                   B[i - 1][13]->Rotl(9), A[i][13]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][13], A[i][9], B[i - 1][1]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][1],
                                   B[i - 1][1]->Rotl(13), A[i][1]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][1], A[i][13], B[i - 1][5]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][5],
                                   B[i - 1][5]->Rotl(18), A[i][5]));

      //(y10, y14, y2, y6) = quarterround(x10, x14, x2, x6)
      Add(new CarryStep<ADD<2>>(word_size_, A[i - 1][10], A[i - 1][6],
                                B[i - 1][14]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][14],
                                   B[i - 1][14]->Rotl(7), A[i][14]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][14], A[i - 1][10],
                                B[i - 1][2]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][2],
                                   B[i - 1][2]->Rotl(9), A[i][2]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][2], A[i][14], B[i - 1][6]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][6],
                                   B[i - 1][6]->Rotl(13), A[i][6]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][6], A[i][2], B[i - 1][10]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][10],
                                   B[i - 1][10]->Rotl(18), A[i][10]));

      //(y15, y3, y7, y11) = quarterround(x15, x3, x7, x11)
      Add(new CarryStep<ADD<2>>(word_size_, A[i - 1][15], A[i - 1][11],
                                B[i - 1][3]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][3],
                                   B[i - 1][3]->Rotl(7), A[i][3]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][3], A[i - 1][15],
                                B[i - 1][7]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][7],
                                   B[i - 1][7]->Rotl(9), A[i][7]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][7], A[i][3], B[i - 1][11]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][11],
                                   B[i - 1][11]->Rotl(13), A[i][11]));
      Add(new CarryStep<ADD<2>>(word_size_, A[i][11], A[i][7], B[i - 1][15]));
      Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][15],
                                   B[i - 1][15]->Rotl(18), A[i][15]));
    }
  }
}
