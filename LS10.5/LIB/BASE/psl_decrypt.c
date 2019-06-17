/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( psl_decrypt.c  )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                                                                     |
|  $Log: psl_decrypt.c,v $
|  Revision 5.5  2002/07/30 02:48:22  robert
|  Updated to fixed password key loopholes
|
|  Revision 5.4  2002/07/26 05:53:16  robert
|  Fixed number overflow on license encryption (generate longer serial key)
|
|  Revision 5.3  2002/04/24 10:02:47  cha
|  S/C 937 Defined _ldlong for Oracle use only.
|                                                            |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

char	DES_init_perm[]=
{
	 8, 64,
	58, 50, 42, 34, 26, 18, 10,  2,
	60, 52, 44, 36, 28, 20, 12,  4,
	62, 54, 46, 38, 30, 22, 14,  6,
	64, 56, 48, 40, 32, 24, 16,  8,
	57, 49, 41, 33, 25, 17,  9,  1,
	59, 51, 43, 35, 27, 19, 11,  3,
	61, 53, 45, 37, 29, 21, 13,  5,
	63, 55, 47, 39, 31, 23, 15,  7
};

char	DES_last_perm[]=
{
	 8, 64,
	40,  8, 48, 16, 56, 24, 64, 32,
	39,  7, 47, 15, 55, 23, 63, 31,
	38,  6, 46, 14, 54, 22, 62, 30,
	37,  5, 45, 13, 53, 21, 61, 29,
	36,  4, 44, 12, 52, 20, 60, 28,
	35,  3, 43, 11, 51, 19, 59, 27,
	34,  2, 42, 10, 50, 18, 58, 26,
	33,  1, 41,  9, 49, 17, 57, 25
};

char	DES_e_perm[]=
{
	 4, 48,
	32,  1,  2,  3,  4,  5,
	 4,  5,  6,  7,  8,  9,
	 8,  9, 10, 11, 12, 13,
	12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21,
	20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29,
	28, 29, 30, 31, 32,  1
};

char	DES_p_perm[]=
{
	 4, 32,
	16,  7, 20, 21,
	29, 12, 28, 17,
	 1, 15, 23, 26,
	 5, 18, 31, 10,
	 2,  8, 24, 14,
	32, 27,  3,  9,
	19, 13, 30,  6,
	22, 11,  4, 25
};

char	DES_k1_perm[]=
{
	 8, 56,
	57, 49, 41, 33, 25, 17,  9,
	 1, 58, 50, 42, 34, 26, 18,
	10,  2, 59, 51, 43, 35, 27,
	19, 11,  3, 60, 52, 44, 36,
	63, 55, 47, 39, 31, 23, 15,
	 7, 62, 54, 46, 38, 30, 22,
	14,  6, 61, 53, 45, 37, 29,
	21, 13,  5, 28, 20, 12,  4
};

char	DES_k2_perm[]=
{
	 7, 48,
	14, 17, 11, 24,  1,  5,
	 3, 28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8,
	16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55,
	30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53,
	46, 42, 50, 36, 29, 32
};

char	DES_s1_tabl[]=
{
	14,  0,  4, 15, 13,  7,  1,  4,  2, 14, 15,  2, 11, 13,  8,  1,
	 3, 10, 10,  6,  6, 12, 12, 11,  5,  9,  9,  5,  0,  3,  7,  8,
	 4, 15,  1, 12, 14,  8,  8,  2, 13,  4,  6,  9,  2,  1, 11,  7,
	15,  5, 12, 11,  9,  3,  7, 14,  3, 10, 10,  0,  5,  6,  0, 13
};

char	DES_s2_tabl[]=
{
	15,  3,  1, 13,  8,  4, 14,  7,  6, 15, 11,  2,  3,  8,  4, 14,
	 9, 12,  7,  0,  2,  1, 13, 10, 12,  6,  0,  9,  5, 11, 10,  5,
	 0, 13, 14,  8,  7, 10, 11,  1, 10,  3,  4, 15, 13,  4,  1,  2,
	 5, 11,  8,  6, 12,  7,  6, 12,  9,  0,  3,  5,  2, 14, 15,  9
};

char	DES_s3_tabl[]=
{
	10, 13,  0,  7,  9,  0, 14,  9,  6,  3,  3,  4, 15,  6,  5, 10,
	 1,  2, 13,  8, 12,  5,  7, 14, 11, 12,  4, 11,  2, 15,  8,  1,
	13,  1,  6, 10,  4, 13,  9,  0,  8,  6, 15,  9,  3,  8,  0,  7,
	11,  4,  1, 15,  2, 14, 12,  3,  5, 11, 10,  5, 14,  2,  7, 12
};

char	DES_s4_tabl[]=
{
	 7, 13, 13,  8, 14, 11,  3,  5,  0,  6,  6, 15,  9,  0, 10,  3,
	 1,  4,  2,  7,  8,  2,  5, 12, 11,  1, 12, 10,  4, 14, 15,  9,
	10,  3,  6, 15,  9,  0,  0,  6, 12, 10, 11,  1,  7, 13, 13,  8,
	15,  9,  1,  4,  3,  5, 14, 11,  5, 12,  2,  7,  8,  2,  4, 14
};

char	DES_s5_tabl[]=
{
	 2, 14, 12, 11,  4,  2,  1, 12,  7,  4, 10,  7, 11, 13,  6,  1,
	 8,  5,  5,  0,  3, 15, 15, 10, 13,  3,  0,  9, 14,  8,  9,  6,
	 4, 11,  2,  8,  1, 12, 11,  7, 10,  1, 13, 14,  7,  2,  8, 13,
	15,  6,  9, 15, 12,  0,  5,  9,  6, 10,  3,  4,  0,  5, 14,  3
};

char	DES_s6_tabl[]=
{
	12, 10,  1, 15, 10,  4, 15,  2,  9,  7,  2, 12,  6,  9,  8,  5,
	 0,  6, 13,  1,  3, 13,  4, 14, 14,  0,  7, 11,  5,  3, 11,  8,
	 9,  4, 14,  3, 15,  2,  5, 12,  2,  9,  8,  5, 12, 15,  3, 10,
	 7, 11,  0, 14,  4,  1, 10,  7,  1,  6, 13,  0, 11,  8,  6, 13
};

char	DES_s7_tabl[]=
{
	 4, 13, 11,  0,  2, 11, 14,  7, 15,  4,  0,  9,  8,  1, 13, 10,
	 3, 14, 12,  3,  9,  5,  7, 12,  5,  2, 10, 15,  6,  8,  1,  6,
	 1,  6,  4, 11, 11, 13, 13,  8, 12,  1,  3,  4,  7, 10, 14,  7,
	10,  9, 15,  5,  6,  0,  8, 15,  0, 14,  5,  2,  9,  3,  2, 12
};

char	DES_s8_tabl[]=
{
	13,  1,  2, 15,  8, 13,  4,  8,  6, 10, 15,  3, 11,  7,  1,  4,
	10, 12,  9,  5,  3,  6, 14, 11,  5,  0,  0, 14, 12,  9,  7,  2,
	 7,  2, 11,  1,  4, 14,  1,  7,  9,  4, 12, 10, 14,  8,  2, 13,
	 0, 15,  6, 12, 10,  9, 13,  0, 15,  3,  3,  5,  5,  6,  8, 11
};

char	DES_cvrt_tabl[]=
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
	'y', 'z', '/', '.'
};

char	keys [16] [6];
#ifdef ORA734
#define ldlong _ldlong
long
 _ldlong(
   char *data )
{
  long longdata = 0;
  int  i,
       sign;
  unsigned char b;

  sign =  (( data[0] & 0x80 ) == 0x80 ) ? 1 : 0;

  for( i = 3; i >= 0; i-- )
  {
    b = data[i];
    if ( sign )
      b = ~b;
    longdata += ( b << (8 * (3-i)) );
  }
  if (sign)
  {
    longdata++;
    longdata = -longdata;
  }
  return longdata;
}
#endif

void
DES_make_keys(char *src)
{
	char	cd[7];
	int	lcnt;

	DES_make_perm (cd, src, DES_k1_perm);

	for (lcnt = 0; lcnt < 16; lcnt++)
	{
		if (lcnt == 0 || lcnt == 1 || lcnt == 8 || lcnt == 15)
			DES_shft (cd, 1);
		else
			DES_shft (cd, 2);

		DES_make_perm ((char *)keys [lcnt], cd, DES_k2_perm);
	}
}

void
DES_make_perm(char *dst, char *src, char *perm_tabl)
{
	char    bit_cnt, chr_posn, mask;

	for (mask = 0; mask < (perm_tabl[1] / 8); mask++)
		*(dst + mask) = 0;
	for (bit_cnt = 0; bit_cnt < perm_tabl[1]; bit_cnt++)
	{
		chr_posn = ((perm_tabl[bit_cnt + 2] - 1) / 8);
		mask = 0x80 >> ((perm_tabl[bit_cnt + 2] - 1) % 8);
		mask = *(src + chr_posn) & mask;
		if (mask)
			*(dst + (bit_cnt / 8)) += (0x80 >> (bit_cnt % 8));
	}
}

void
DES_shft(char *src, int cnt)
{
	int     loop, tmp1, tmp2;

	tmp2 = 0;
	for (loop = 6; loop >= 0; loop--)
	{
		if (cnt == 2)
		{
			tmp1 = (*(src + loop) >> 6) & 0x03;
			*(src + loop) = ((*(src + loop) << 2) & 0xfc) + tmp2;
			tmp2 = tmp1;
		}
		else
		{
			tmp1 = (*(src + loop) >> 7) & 0x01;
			*(src + loop) = ((*(src + loop) << 1) & 0xfe) + tmp2;
			tmp2 = tmp1;
		}
	}
	if (cnt == 2)
	{
		*(src + 6) += (*(src + 3) >> 4) & 0x03;
		*(src + 3) &= 0xcf;
		tmp2 = (tmp2 << 4) & 0x30;
		*(src + 3) += tmp2;
	}
	else
	{
		*(src + 6) += (*(src + 3) >> 4) & 0x01;
		*(src + 3) &= 0xef;
		tmp2 = (tmp2 << 4) & 0x10;
		*(src + 3) += tmp2;
	}
}

void
DES_decode (char *data, char *key)
{
	char    cc, tmp_e[6], tmp[4], tmp_p[4];
	int     cnt;

	DES_make_perm (tmp_e, data, DES_e_perm);
	for (cnt = 0; cnt < 6; cnt++)
		tmp_e[cnt] ^= *(key + cnt);
	tmp_p[0]  = DES_s1_tabl[tmp_e[0] >> 2 & 0x3f] << 4;
	tmp_p[0] += DES_s2_tabl[(tmp_e[0] << 4 & 0x30) + (tmp_e[1] >> 4 & 0x0f)];
	tmp_p[1]  = DES_s3_tabl[(tmp_e[1] << 2 & 0x3c) + (tmp_e[2] >> 6 & 0x03)] << 4;
	tmp_p[1] += DES_s4_tabl[tmp_e[2] & 0x3f];
	tmp_p[2]  = DES_s5_tabl[tmp_e[3] >> 2 & 0x3f] << 4;
	tmp_p[2] += DES_s6_tabl[(tmp_e[3] << 4 & 0x30) + (tmp_e[4] >> 4 & 0x0f)];
	tmp_p[3]  = DES_s7_tabl[(tmp_e[4] << 2 & 0x3c) + (tmp_e[5] >> 6 & 0x03)] << 4;
	tmp_p[3] += DES_s8_tabl[tmp_e[5] & 0x3f];
	DES_make_perm (tmp, tmp_p, DES_p_perm);
	for (cnt = 4; cnt < 8; cnt++)
		*(data + cnt) ^= tmp[cnt - 4];
	for (cnt = 0; cnt < 4; cnt++)
	{
		cc = *(data + cnt);
		*(data + cnt) = *(data + cnt + 4);
		*(data + cnt + 4) = cc;
	}
}

int
DES_ascii_bin (char *data, char *dst)
{
	char	cc;

	if (strlen (data) != 11)
		return (-1);
	cc = DES_look_tabl (*(data + 0));
	*(dst + 0)  = cc << 2 & 0xfc;
	cc = DES_look_tabl (*(data + 1));
	*(dst + 0) += cc >> 4 & 0x03;
	*(dst + 1)  = cc << 4 & 0xf0;
	cc = DES_look_tabl (*(data + 2));
	*(dst + 1) += cc >> 2 & 0x0f;
	*(dst + 2)  = cc << 6 & 0xc0;
	cc = DES_look_tabl (*(data + 3));
	*(dst + 2) += cc;
	cc = DES_look_tabl (*(data + 4));
	*(dst + 3)  = cc << 2 & 0xfc;
	cc = DES_look_tabl (*(data + 5));
	*(dst + 3) += cc >> 4 & 0x03;
	*(dst + 4)  = cc << 4 & 0xf0;
	cc = DES_look_tabl (*(data + 6));
	*(dst + 4) += cc >> 2 & 0x0f;
	*(dst + 5)  = cc << 6 & 0xc0;
	cc = DES_look_tabl (*(data + 7));
	*(dst + 5) += cc;
	cc = DES_look_tabl (*(data + 8));
	*(dst + 6)  = cc << 2 & 0xfc;
	cc = DES_look_tabl (*(data + 9));
	*(dst + 6) += cc >> 4 & 0x03;
	*(dst + 7)  = cc << 4 & 0xf0;
	cc = DES_look_tabl (*(data + 10));
	*(dst + 7) += cc >> 2 & 0x0f;
	return (EXIT_SUCCESS);
}

int
DES_look_tabl (char cc)
{
	int	count;

	for (count = 0; count < 64; count++)
	{
		if (DES_cvrt_tabl[count] == cc)
			return (count);
	}
	return (-1);
}

int
DES_decrypt(char *src, char *dst, char *salt)
{
	char    tmp1[8], tmp2[8];
	int     count;

	if (DES_ascii_bin (dst, tmp1) == 0)
	{
		DES_make_keys (salt);
		DES_make_perm (tmp2, tmp1, DES_init_perm);
		for (count = 15; count >= 0; count--)
			DES_decode (tmp2, (char *) keys [count]);
		DES_make_perm (src, tmp2, DES_last_perm);
		return (EXIT_SUCCESS);
	}
	return (-1);
}

void
psl_decrypt (struct DES_REC *des_rec)
{
	char	lcGenNo [12];
	char	lcPassword [12];	
	char	out_buf[8];
	long	tmp_rslt;
/*
	sprintf (lcGenNo, "%010ld", lc_i_no ());
	DES_decrypt (out_buf, des_rec->passwd, lcGenNo);
	tmp_rslt = ldlong ((unsigned char *)&out_buf[0]);

	des_rec->max_usr = (tmp_rslt >> 24) & 0xff; 
	des_rec->user_key = tmp_rslt & 0xffffff;
	tmp_rslt = ldlong ((unsigned char *)&out_buf[4]);

	des_rec->max_trm = (tmp_rslt >> 16) & 0xffff;
	des_rec->expiry = tmp_rslt & 0xffff;
*/
	sprintf (lcGenNo, "%010ld", lc_i_no ());

	strncpy (lcPassword, des_rec->passwd, 11);
	lcPassword [11] = 0;
	
	DES_decrypt (out_buf, lcPassword, lcGenNo);
	des_rec->user_key = ldlong ((unsigned char *)&out_buf[0]);
	des_rec->expiry = ldlong ((unsigned char *)&out_buf[4]);

	strcpy (lcPassword, &des_rec->passwd[11]);
	DES_decrypt (out_buf, lcPassword, lcGenNo);

	tmp_rslt = ldlong ((unsigned char *)&out_buf[0]);
	
	des_rec->max_usr = (tmp_rslt >> 16) & 0xffff;
	des_rec->max_trm = tmp_rslt & 0xffff;
	
	// user_key on two encryption password should be equal
	des_rec->user_key &= ldlong ((unsigned char *)&out_buf[4]);
}
