/* lookup table to convert nice values to priority offsets */
/* nice is between -20 and 19 inclusively */
/* taskniceptr = &tasknicetab[SCHEDNICEHALF]; */
/* prio += taskniceptr[nice]; */
long tasknicetab[SCHEDNICERANGE]
= {
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -64, -60, -57, -54, -51,
    -47, -44, -41, -38, -35, -31, -28, -25,
    -22, -19, -15, -12, -9, -6, -3, 0,
    3, 6, 9, 13, 16, 19, 23, 26,
    29, 33, 36, 39, 43, 46, 49, 53,
    56, 59, 63, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};
