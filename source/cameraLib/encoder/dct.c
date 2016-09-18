#include "dct.h"

#include "arm_math.h"

uint32_t dual_mul_add_products(uint32_t val1, uint32_t val2)
{
  return __SMUAD(val1, val2);
}

uint32_t dual_mul_sub_products(uint32_t val1, uint32_t val2)
{
  return __SMUSD(val1, val2);
}

uint32_t dual_mul_add_products_x(uint32_t val1, uint32_t val2)
{
  return __SMUADX(val1, val2);
}

uint32_t dual_mul_add_products_lx(uint32_t val1, uint32_t val2, uint32_t val3)
{
  return __SMLADX(val1, val2, val3);
}

uint32_t dual_mul_sub_products_x(uint32_t val1, uint32_t val2)
{
  return __SMUSDX(val1, val2);
}

uint32_t dual_mul_sub_products_lx(uint32_t val1, uint32_t val2, uint32_t val3)
{
  return __SMLSDX(val1, val2, val3);
}

/******************************************************************************
**  dct
**  --------------------------------------------------------------------------
**  Fast DCT - Discrete Cosine Transform.
**  This function converts 8x8 pixel block into frequencies.
**  Lowest frequencies are at the upper-left corner.
**  The input and output could point at the same array, in this case the data
**  will be overwritten.
**  
**  ARGUMENTS:
**      pixels  - 8x8 pixel array;
**      data    - 8x8 freq block;
**
**  RETURN: -
******************************************************************************/
void dct(int16_t pixels[8][8], int16_t data[8][8])
{
	int16_t rows[8][8];
	unsigned          i;

	static const int16_t // Ci = cos(i*PI/16)*(1 << 14);
		C1 = 16070,
		C2 = 15137,
		C3 = 13623,
		C4 = 11586,
		C5 = 9103,
		C6 = 6270,
		C7 = 3197;

//	static const int32_t
//		C1H = 1053163520,
//		C2H = 992018432,
//		C3H = 892796928,
//		C4H = 759300096,
//		C5H = 596574208,
//		C6H = 406781952,
//		C7H = 209518592;
	// simple but fast DCT - 22*16 multiplication 28*16 additions and 8*16 shifts.


	/* transform rows */
	for (i = 0; i < 8; i++)
	{
		int16_t s07,s16,s25,s34,s0734,s1625;
		int16_t d07,d16,d25,d34,d0734,d1625;

		s07 = pixels[i][0] + pixels[i][7];
		d07 = pixels[i][0] - pixels[i][7];
		s16 = pixels[i][1] + pixels[i][6];
		d16 = pixels[i][1] - pixels[i][6];
		s25 = pixels[i][2] + pixels[i][5];
		d25 = pixels[i][2] - pixels[i][5];
		s34 = pixels[i][3] + pixels[i][4];
		d34 = pixels[i][3] - pixels[i][4];

//		rows[i][1] = ((dual_mul_add_products_lx(((uint32_t)C1H | (uint32_t)(d16 & 0x0000FFFF)), ((uint32_t)C3H | (uint32_t)(d07 & 0x0000FFFF)), \
//				  dual_mul_add_products_x(((uint32_t)C5H | ((uint32_t)d34 & 0x0000FFFF)), ((uint32_t)C7H | (uint32_t)(d25 & 0x0000FFFF))))) >> 14);
//
//		rows[i][3] = ((dual_mul_sub_products_x(((uint32_t)C7H | (uint32_t)(d07 & 0x0000FFFF)), ((uint32_t)C3H | (uint32_t)(d16 & 0x0000FFFF))) \
//				- dual_mul_add_products_x(((uint32_t)C5H | (uint32_t)(d25 & 0x0000FFFF)), ((uint32_t)C1H | (uint32_t)(d34 & 0x0000FFFF)))) >> 14);
//
//		rows[i][5] = ((dual_mul_sub_products_lx(((uint32_t)C1H | (uint32_t)(d07 & 0x0000FFFF)), ((uint32_t)C5H | (uint32_t)(d16 & 0x0000FFFF)), \
//				 dual_mul_add_products_x(((uint32_t)C7H | (uint32_t)(d34 & 0x0000FFFF)), ((uint32_t)C3H | (uint32_t)(d25 & 0x0000FFFF))))) >> 14);
//
//		rows[i][7] = ((dual_mul_sub_products_lx(((uint32_t)C5H | (uint32_t)(d07 & 0x0000FFFF)), ((uint32_t)C7H | (uint32_t)(d16 & 0x0000FFFF)), \
//				 dual_mul_sub_products_x(((uint32_t)C1H | (uint32_t)(d25 & 0x0000FFFF)), ((uint32_t)C3H | (uint32_t)(d34 & 0x0000FFFF))))) >> 14);

		rows[i][1] = (C1*d07 + C3*d16 + C5*d25 + C7*d34) >> 14;
		rows[i][3] = (C3*d07 - C7*d16 - C1*d25 - C5*d34) >> 14;
		rows[i][5] = (C5*d07 - C1*d16 + C7*d25 + C3*d34) >> 14;
		rows[i][7] = (C7*d07 - C5*d16 + C3*d25 - C1*d34) >> 14;

		s0734 = s07 + s34;
		d0734 = s07 - s34;
		s1625 = s16 + s25;
		d1625 = s16 - s25;

//		rows[i][0] = ((dual_mul_add_products_x(((uint32_t)C4H | (uint32_t)(s1625 & 0x0000FFFF)), \
//				((uint32_t)C4H | (uint32_t)(s0734 & 0x0000FFFF)))) >> 14);
//		rows[i][4] = ((dual_mul_sub_products_x(((uint32_t)C4H | (uint32_t)(s0734 & 0x0000FFFF)), \
//				((uint32_t)C4H | (uint32_t)(s1625 & 0x0000FFFF)))) >> 14);


		rows[i][0] = (C4*(s0734 + s1625)) >> 14;
		rows[i][4] = (C4*(s0734 - s1625)) >> 14;

//		rows[i][2] = ((dual_mul_add_products_x(((uint32_t)C2H | (uint32_t)(d1625 & 0x0000FFFF)), \
//				((uint32_t)C6H | (uint32_t)(d0734 & 0x0000FFFF)))) >> 14);
//		rows[i][6] = ((dual_mul_sub_products_x(((uint32_t)C2H | (uint32_t)(d0734 & 0x0000FFFF)), \
//				((uint32_t)C6H | (uint32_t)(d1625 & 0x0000FFFF)))) >> 14);

		rows[i][2] = (C2*d0734 + C6*d1625) >> 14;
		rows[i][6] = (C6*d0734 - C2*d1625) >> 14;
	}

	/* transform columns */
	for (i = 0; i < 8; i++)
	{
		int16_t s07,s16,s25,s34,s0734,s1625;
		int16_t d07,d16,d25,d34,d0734,d1625;

		s07 = rows[0][i] + rows[7][i];
		d07 = rows[0][i] - rows[7][i];
		s16 = rows[1][i] + rows[6][i];
		d16 = rows[1][i] - rows[6][i];
		s25 = rows[2][i] + rows[5][i];
		d25 = rows[2][i] - rows[5][i];
		s34 = rows[3][i] + rows[4][i];
		d34 = rows[3][i] - rows[4][i];


//		data[1][i] = ((dual_mul_add_products_lx(((uint32_t)C1H | (uint32_t)(d16 & 0x0000FFFF)), ((uint32_t)C3H | (uint32_t)(d07 & 0x0000FFFF)), \
//				 dual_mul_add_products_x(((uint32_t)C5H | ((uint32_t)d34 & 0x0000FFFF)), ((uint32_t)C7H | (uint32_t)(d25 & 0x0000FFFF))))) >> 16);
//
//		data[3][i] = ((dual_mul_sub_products_x(((uint32_t)C7H | (uint32_t)(d07 & 0x0000FFFF)), ((uint32_t)C3H | (uint32_t)(d16 & 0x0000FFFF))) \
//				- dual_mul_add_products_x(((uint32_t)C5H | (uint32_t)(d25 & 0x0000FFFF)), ((uint32_t)C1H | (uint32_t)(d34 & 0x0000FFFF)))) >> 16);
//
//		data[5][i] = ((dual_mul_sub_products_lx(((uint32_t)C1H | (uint32_t)(d07 & 0x0000FFFF)), ((uint32_t)C5H | (uint32_t)(d16 & 0x0000FFFF)), \
//				 dual_mul_add_products_x(((uint32_t)C7H | (uint32_t)(d34 & 0x0000FFFF)), ((uint32_t)C3H | (uint32_t)(d25 & 0x0000FFFF))))) >> 16);
//
//		data[7][i] = ((dual_mul_sub_products_lx(((uint32_t)C5H | (uint32_t)(d07 & 0x0000FFFF)), ((uint32_t)C7H | (uint32_t)(d16 & 0x0000FFFF)), \
//				 dual_mul_sub_products_x(((uint32_t)C1H | (uint32_t)(d25 & 0x0000FFFF)), ((uint32_t)C3H | (uint32_t)(d34 & 0x0000FFFF))))) >> 16);


		data[1][i] = (C1*d07 + C3*d16 + C5*d25 + C7*d34) >> 16;
		data[3][i] = (C3*d07 - C7*d16 - C1*d25 - C5*d34) >> 16;
		data[5][i] = (C5*d07 - C1*d16 + C7*d25 + C3*d34) >> 16;
		data[7][i] = (C7*d07 - C5*d16 + C3*d25 - C1*d34) >> 16;

		s0734 = s07 + s34;
		d0734 = s07 - s34;
		s1625 = s16 + s25;
		d1625 = s16 - s25;


//		data[0][i] = ((dual_mul_add_products_x(((uint32_t)C4H | (uint32_t)(s1625 & 0x0000FFFF)), \
//				((uint32_t)C4H | (uint32_t)(s0734 & 0x0000FFFF)))) >> 16);
//		data[4][i] = ((dual_mul_sub_products_x(((uint32_t)C4H | (uint32_t)(s0734 & 0x0000FFFF)), \
//				((uint32_t)C4H | (uint32_t)(s1625 & 0x0000FFFF)))) >> 16);

		data[0][i] = (C4*(s0734 + s1625)) >> 16;
		data[4][i] = (C4*(s0734 - s1625)) >> 16;

//		data[2][i] = ((dual_mul_add_products_x(((uint32_t)C2H | (uint32_t)(d1625 & 0x0000FFFF)), \
//				((uint32_t)C6H | (uint32_t)(d0734 & 0x0000FFFF)))) >> 16);
//		data[6][i] = ((dual_mul_sub_products_x(((uint32_t)C2H | (uint32_t)(d0734 & 0x0000FFFF)), \
//				((uint32_t)C6H | (uint32_t)(d1625 & 0x0000FFFF)))) >> 16);

		data[2][i] = (C2*d0734 + C6*d1625) >> 16;
		data[6][i] = (C6*d0734 - C2*d1625) >> 16;
	}
}