
/*
  dim_curve 'lookup table' to compensate for the nonlinearity of human vision.
  Used in the getRGB function on saturation and brightness to make 'dimming' look more natural.
  Exponential function used to create values below :
  x from 0 - 255 : y = round(pow( 2.0, x+64/40.0) - 1)
*/

const byte dim_curve[] = {
  0,   1,   1,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
  3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
  4,   4,   4,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   6,   6,   6,
  6,   6,   6,   6,   6,   7,   7,   7,   7,   7,   7,   7,   8,   8,   8,   8,
  8,   8,   9,   9,   9,   9,   9,   9,   10,  10,  10,  10,  10,  11,  11,  11,
  11,  11,  12,  12,  12,  12,  12,  13,  13,  13,  13,  14,  14,  14,  14,  15,
  15,  15,  16,  16,  16,  16,  17,  17,  17,  18,  18,  18,  19,  19,  19,  20,
  20,  20,  21,  21,  22,  22,  22,  23,  23,  24,  24,  25,  25,  25,  26,  26,
  27,  27,  28,  28,  29,  29,  30,  30,  31,  32,  32,  33,  33,  34,  35,  35,
  36,  36,  37,  38,  38,  39,  40,  40,  41,  42,  43,  43,  44,  45,  46,  47,
  48,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,
  63,  64,  65,  66,  68,  69,  70,  71,  73,  74,  75,  76,  78,  79,  81,  82,
  83,  85,  86,  88,  90,  91,  93,  94,  96,  98,  99,  101, 103, 105, 107, 109,
  110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
  146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
  193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,
};


#define LE_BRIGHTER  0xF700FF
#define LE_DIMMER  0xF7807F
#define LE_OFF  0xF740BF
#define LE_ON  0xF7C03F
#define MR0C0  0xF700FF
#define MR0C1  0xF7807F
#define MR0C2  0xF740BF
#define MR0C3  0xF7C03F

#define LE_R1  0xF720DF
#define LE_G1  0xF7A05F
#define LE_B1  0xF7609F
#define LE_W  0xF7E01F
#define MR1C0  0xF720DF
#define MR1C1  0xF7A05F
#define MR1C2  0xF7609F
#define MR1C3  0xF7E01F

#define LE_R2  0xF710EF
#define LE_G2  0xF7906F
#define LE_B2  0xF750AF
#define LE_FLASH 0xF7D02F 
#define MR2C0  0xF710EF
#define MR2C1  0xF7906F
#define MR2C2  0xF750AF
#define MR2C3  0xF7D02F 

#define LE_Y1  0xF730CF
#define LE_C1  0xF7B04F
#define LE_V1  0xF7708F
#define LE_STROBE 0xF7F00F
#define MR3C0 0xF730CF
#define MR3C1  0xF7B04F
#define MR3C2  0xF7708F
#define MR3C3  0xF7F00F

#define LE_Y2  0xF708F7
#define LE_C2  0xF78877
#define LE_V2  0xF748B7
#define LE_FADE  0xF7C837
#define MR4C0  0xF708F7
#define MR4C1  0xF78877
#define MR4C2  0xF748B7
#define MR4C3  0xF7C837


#define LE_Y3  0xF728D7
#define LE_C3  0xF7A857
#define LE_V3  0xF76897
#define LE_SMOOTH  0xF7E817
#define MR5C0  0xF728D7
#define MR5C1  0xF7A857
#define MR5C2  0xF76897
#define MR5C3  0xF7E817

/*
//PWM Full bright
LE_R1 100%
LE_G1 100%
LE_B1 100%

LE_R2 R 100% G 20% B 0% -> R 90% G 18%-20% B0 //Bright 50% -> R 50% G 18%-20% B0 // Bright 10% -> R 10% G 10% B 0%
LE_Y1 R 100% G 50% B 0% 
LE_Y2 R 100% G 72-80% B 0%
LE_Y3 R 100% G 100% B 0%
LE_G2 R 0% G 100% B 50%
LE_C1 R 0% G 100% B 72%-80%
LE_C2 R 0% G 80% B 100%
LE_C3 R 0% G 50% B 100%

LE_B2 R 20% G 0% B 100%
LE_V1 R 50% G 0% B 100% 
LE_V2 R 72%-80% G 0% B 100%
LE_V3 R 100% G0 B 100%
*/
