#include "turtle.hpp"
#include "gtest/gtest.h"

// scusa, mi annoiavo
// (probabile poi devo sistemarlo mettendo operatori << e == alla matrice) 

// da 1 a 29 parametri, sta macro ti ritorna il numero di parametri passati
// alla macro, è un azzigogolo gigantesco, lascia fare
// https://groups.google.com/g/comp.std.c/c/d-6Mj5Lko_s?pli=1
// ocio, sta macro non funziona per 0 parametri
#define ARG_COUNT(...) EXTRACT_30TH_IN          \
    (__VA_ARGS__,                               \
     29,28,27,26,25,24,23,22,21,20,             \
     19,18,17,16,15,14,13,12,11,10,             \
     9,8,7,6,5,4,3,2,1)
#define EXTRACT_30TH_IN(_01, _02, _03, _04, _05, _06, _07, _08, _09, _10, \
                        _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                        _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
                        ...) _30

#define FA(...) std::array<float, ARG_COUNT(__VA_ARGS__)>{ __VA_ARGS__ }
#define FA3C(...) static_cast<std::array<float, 3>>( __VA_ARGS__ )


#define OUT_VEC3(A) "{" << A[0] << ", " << A[1] << ", " << A[2]<< "}"

#define ASS_VEC3EQ_IND(A, B, I)                 \
    ASSERT_EQ((A)[I], (B)[I])                   \
    << "inequality between\n"                   \
    <<"  " << #A << "\n"                        \
    <<"and\n"                                   \
    << "  " << #B << "\n\n"                     \
    << "which are respectively\n"               \
    <<"  " << OUT_VEC3(A) << "\n"               \
    << "and\n"                                \
    << "  " << OUT_VEC3(B) << "\n"              \
    <<"unequal at index\n  " << #I << "\n"

#define ASS_VEC3_EQ(A, B) do {                  \
        ASS_VEC3EQ_IND((A), (B), 0);             \
        ASS_VEC3EQ_IND((A), (B), 1);             \
        ASS_VEC3EQ_IND((A), (B), 2);             \
    }while(0)

#define ASS_MAT3_EQ(A, B) do {                  \
        ASS_VEC3_EQ((A[0]), (B[0]));            \
        ASS_VEC3_EQ((A[1]), (B[1]));            \
        ASS_VEC3_EQ((A[2]), (B[2]));            \
    }while(0)


TEST(MatrixTest, MultiplySymmetric) {
    ASS_VEC3_EQ(Matrix3(1, 0, 0,
                        0, 1, 0,
                        0, 0, 1) * FA(1, 2, 3),
                FA(1, 2, 3));

    ASS_VEC3_EQ((Matrix3(2, 0, 0,
                         0, 2, 0,
                         0, 0, 2) * FA(1, 2, 3)),
                FA(2, 4, 6));

    ASS_VEC3_EQ((Matrix3(1, 0, 0,
                         0, 0, 0,
                         0, 0, 0) * FA(1, 2, 3)),
                FA(1, 0, 0));

    ASS_VEC3_EQ((Matrix3(0, 0, 0,
                         0, 1, 0,
                         0, 0, 0) * FA(1, 2, 3)),
                FA(0, 2, 0));

    ASS_VEC3_EQ((Matrix3(0, 0, 0,
                         0, 0, 0,
                         0, 0, 1) * FA(1, 2, 3)),
                FA(0, 0, 3));

    ASS_VEC3_EQ((Matrix3(1, 0, 0,
                         0, 1, 0,
                         0, 0, 0) * FA(1, 2, 3)),
                FA(1, 2, 0));

    ASS_VEC3_EQ((Matrix3(1, 0, 0,
                         0, 0, 0,
                         0, 0, 1) * FA(1, 2, 3)),
                FA(1, 0, 3));

    ASS_VEC3_EQ((Matrix3(0, 0, 0,
                         0, 1, 0,
                         0, 0, 1) * FA(1, 2, 3)),
                FA(0, 2, 3));

    ASS_VEC3_EQ((Matrix3(1, 1, 1,
                         1, 1, 1,
                         1, 1, 1) * FA(1, 2, 3)),
                FA(6, 6, 6));

    ASS_VEC3_EQ((Matrix3(2, 2, 2,
                         2, 2, 2,
                         2, 2, 2) * FA(1, 2, 3)),
                FA(12, 12, 12));
}

TEST(MatrixTest, MultiplyAsymmetric) {
    ASS_VEC3_EQ((Matrix3(0, 0, 1,
                         0, 1, 0,
                         1, 0, 0) * FA(1, 2, 3)),
                FA(3, 2, 1));

    ASS_VEC3_EQ((Matrix3(0, 0, 0,
                         0, 1, 0,
                         1, 0, 0) * FA(1, 2, 3)),
                FA(0, 2, 1));

    ASS_VEC3_EQ((Matrix3(0, 0, 1,
                         0, 0, 0,
                         1, 0, 0) * FA(1, 2, 3)),
                FA(3, 0, 1));

    ASS_VEC3_EQ((Matrix3(0, 0, 0,
                         0, 1, 0,
                         1, 0, 0) * FA(1, 2, 3)),
                FA(0, 2, 1));

    ASS_VEC3_EQ((Matrix3(2, 1, 2,
                         2, 1, 2,
                         2, 1, 2) * FA(1, 2, 3)),
                FA(10, 10, 10));

    ASS_VEC3_EQ((Matrix3(2, 2, 2,
                         1, 1, 1,
                         2, 2, 2) * FA(1, 2, 3)),
                FA(12, 6, 12));
}

TEST(MatrixTest, AccessRows) {
    Matrix3 m(1, 2, 3,
              4, 5, 6,
              7, 8, 9);
    ASS_VEC3_EQ(m.row(0), FA(1, 2, 3));
    ASS_VEC3_EQ(m.row(1), FA(4, 5, 6));
    ASS_VEC3_EQ(m.row(2), FA(7, 8, 9));

    ASS_VEC3_EQ(m.trans().col(0), FA(1, 2, 3));
    ASS_VEC3_EQ(m.trans().col(1), FA(4, 5, 6));
    ASS_VEC3_EQ(m.trans().col(2), FA(7, 8, 9));
}

TEST(MatrixTest, AccessCols) {
    Matrix3 m(1, 2, 3,
              4, 5, 6,
              7, 8, 9);

    ASS_VEC3_EQ(m.col(0), FA(1, 4, 7));
    ASS_VEC3_EQ(m.col(1), FA(2, 5, 8));
    ASS_VEC3_EQ(m.col(2), FA(3, 6, 9));

    ASS_VEC3_EQ(m.trans().row(0), FA(1, 4, 7));
    ASS_VEC3_EQ(m.trans().row(1), FA(2, 5, 8));
    ASS_VEC3_EQ(m.trans().row(2), FA(3, 6, 9));
}

TEST(MatrixTest, AccessElt) {
    Matrix3 m(1, 2, 3,
              4, 5, 6,
              7, 8, 9);

    // as is
    ASSERT_EQ(m[0][0], 1); ASSERT_EQ(m(0)[0], 1); ASSERT_EQ(m(0, 0), 1);
    ASSERT_EQ(m[0][1], 2); ASSERT_EQ(m(0)[1], 2); ASSERT_EQ(m(0, 1), 2);
    ASSERT_EQ(m[0][2], 3); ASSERT_EQ(m(0)[2], 3); ASSERT_EQ(m(0, 2), 3);

    ASSERT_EQ(m[1][0], 4); ASSERT_EQ(m(1)[0], 4); ASSERT_EQ(m(1, 0), 4);
    ASSERT_EQ(m[1][1], 5); ASSERT_EQ(m(1)[1], 5); ASSERT_EQ(m(1, 1), 5);
    ASSERT_EQ(m[1][2], 6); ASSERT_EQ(m(1)[2], 6); ASSERT_EQ(m(1, 2), 6);

    ASSERT_EQ(m[2][0], 7); ASSERT_EQ(m(2)[0], 7); ASSERT_EQ(m(2, 0), 7);
    ASSERT_EQ(m[2][1], 8); ASSERT_EQ(m(2)[1], 8); ASSERT_EQ(m(2, 1), 8);
    ASSERT_EQ(m[2][2], 9); ASSERT_EQ(m(2)[2], 9); ASSERT_EQ(m(2, 2), 9);

    // transposed
    auto t = m.trans();
    ASSERT_EQ(t[0][0], 1); ASSERT_EQ(t(0)[0], 1); ASSERT_EQ(t(0, 0), 1);
    ASSERT_EQ(t[1][0], 2); ASSERT_EQ(t(1)[0], 2); ASSERT_EQ(t(1, 0), 2);
    ASSERT_EQ(t[2][0], 3); ASSERT_EQ(t(2)[0], 3); ASSERT_EQ(t(2, 0), 3);

    ASSERT_EQ(t[0][1], 4); ASSERT_EQ(t(0)[1], 4); ASSERT_EQ(t(0, 1), 4);
    ASSERT_EQ(t[1][1], 5); ASSERT_EQ(t(1)[1], 5); ASSERT_EQ(t(1, 1), 5);
    ASSERT_EQ(t[2][1], 6); ASSERT_EQ(t(2)[1], 6); ASSERT_EQ(t(2, 1), 6);

    ASSERT_EQ(t[0][2], 7); ASSERT_EQ(t(0)[2], 7); ASSERT_EQ(t(0, 2), 7);
    ASSERT_EQ(t[1][2], 8); ASSERT_EQ(t(1)[2], 8); ASSERT_EQ(t(1, 2), 8);
    ASSERT_EQ(t[2][2], 9); ASSERT_EQ(t(2)[2], 9); ASSERT_EQ(t(2, 2), 9);
}

TEST(MatrixTest, MatMulSymmetric) {
    Matrix3 m(1, 2, 3,
              4, 5, 6,
              7, 8, 9);

    ASS_MAT3_EQ((Matrix3(1, 0, 0,
                         0, 1, 0,
                         0, 0, 1) * m),
                (m));

    ASS_MAT3_EQ((Matrix3(2, 0, 0,
                         0, 2, 0,
                         0, 0, 2) * m),
                (Matrix3(2, 4, 6,
                         8, 10, 12,
                         14, 16, 18)));

    ASS_MAT3_EQ((Matrix3(0, 0, 0,
                         0, 0, 0,
                         0, 0, 0) * m),
                (Matrix3(0, 0, 0,
                         0, 0, 0,
                         0, 0, 0)));

}

TEST(MatrixTest, MatMulExtract) {
    Matrix3 m(1, 2, 3,
              4, 5, 6,
              7, 8, 9);

    ASS_MAT3_EQ((Matrix3(1, 0, 0,
                         0, 0, 0,
                         0, 0, 0) * m),
                (Matrix3(1, 2, 3,
                         0, 0, 0,
                         0, 0, 0)));

    ASS_MAT3_EQ((Matrix3(0, 0, 0,
                         0, 1, 0,
                         0, 0, 0) * m),
                (Matrix3(0, 0, 0,
                         4, 5, 6,
                         0, 0, 0)));

    ASS_MAT3_EQ((Matrix3(0, 0, 0,
                         0, 0, 0,
                         0, 0, 1) * m),
                (Matrix3(0, 0, 0,
                         0, 0, 0,
                         7, 8, 9)));

    ASS_MAT3_EQ((m * Matrix3(1, 0, 0,
                             0, 0, 0,
                             0, 0, 0)),
                (Matrix3(1, 0, 0,
                         4, 0, 0,
                         7, 0, 0)));

    ASS_MAT3_EQ((m * Matrix3(0, 0, 0,
                             0, 1, 0,
                             0, 0, 0)),
                (Matrix3(0, 2, 0,
                         0, 5, 0,
                         0, 8, 0)));

    ASS_MAT3_EQ((m * Matrix3(0, 0, 0,
                             0, 0, 0,
                             0, 0, 1)),
                (Matrix3(0, 0, 3,
                         0, 0, 6,
                         0, 0, 9)));
}

TEST(MatrixTest, MatMulASymmetric) {
    Matrix3 m(1, 2, 3,
              4, 5, 6,
              7, 8, 9);
    // had to check fucken
    // https://www.emathhelp.net/calculators/linear-algebra/matrix-multiplication-calculator/?a=%5B%5B1%2C0%2C0%5D%2C%5B1%2C0%2C0%5D%2C%5B1%2C0%2C0%5D%5D&b=%5B%5B1%2C2%2C3%5D%2C%5B4%2C5%2C6%5D%2C%5B7%2C8%2C9%5D%5D

    ASS_MAT3_EQ((Matrix3(1, 0, 0,
                         1, 0, 0,
                         1, 0, 0) * m),
                (Matrix3(1, 2, 3,
                         1, 2, 3,
                         1, 2, 3)));

    ASS_MAT3_EQ((Matrix3(0, 1, 0,
                         0, 1, 0,
                         0, 1, 0) * m),
                (Matrix3(4, 5, 6,
                         4, 5, 6,
                         4, 5, 6)));

    ASS_MAT3_EQ((Matrix3(0, 0, 1,
                         0, 0, 1,
                         0, 0, 1) * m),
                (Matrix3(7, 8, 9,
                         7, 8, 9,
                         7, 8, 9)));

    ASS_MAT3_EQ((m * Matrix3(1, 1, 1,
                             0, 0, 0,
                             0, 0, 0)),
                (Matrix3(1, 1, 1,
                         4, 4, 4,
                         7, 7, 7)));

    ASS_MAT3_EQ((m * Matrix3(0, 0, 0,
                             1, 1, 1,
                             0, 0, 0)),
                (Matrix3(2, 2, 2,
                         5, 5, 5,
                         8, 8, 8)));

    ASS_MAT3_EQ((m * Matrix3(0, 0, 0,
                             0, 0, 0,
                             1, 1, 1)),
                (Matrix3(3, 3, 3,
                         6, 6, 6,
                         9, 9, 9)));
}
// vorrei nuovamente ringraziare
// https://www.emathhelp.net/calculators/linear-algebra/matrix-multiplication-calculator/?a=%5B%5B1%2C2%2C3%5D%2C%5B4%2C5%2C6%5D%2C%5B7%2C8%2C9%5D%5D&b=%5B%5B1%2C1%2C1%5D%2C%5B0%2C0%2C0%5D%2C%5B0%2C0%2C0%5D%5D
// visto che mi sono scordato come moltiplicare due matrici

TEST(MatrixTest, CopyConstructor) {
    Matrix3 m = Matrix3(1, 2, 3,
                        4, 5, 6,
                        7, 8, 9);
    Matrix3 m2 = m;
    ASS_MAT3_EQ(m, m2);
    ASS_MAT3_EQ(m2, m);
}


TEST(MatrixTest, ElementAssignment) {
    Matrix3 m = Matrix3(1, 2, 3,
                        4, 5, 6,
                        7, 8, 9);
    m[0][0] = 5;
    ASS_MAT3_EQ(m, Matrix3(5, 2, 3,
                           4, 5, 6,
                           7, 8, 9));
}

TEST(MatrixTest, WholeAssignment) {
    Matrix3 m = Matrix3(1, 2, 3,
                        4, 5, 6,
                        7, 8, 9);
    Matrix3 m2 = Matrix3(0, 0, 0,
                         0, 0, 0,
                         0, 0, 0);
    m2 = m;
    ASS_MAT3_EQ(m, m2);
    ASS_MAT3_EQ(m2, m);

}
