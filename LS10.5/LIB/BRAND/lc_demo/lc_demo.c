/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: lc_demo.c,v 5.2 2002/07/30 07:28:49 scott Exp $
|  Program Name  : (lc_create.c)
|  Program Desc  : (Logistic License Creation Program)
|----------------------------------------------------------------------
| $Log: lc_demo.c,v $
| Revision 5.2  2002/07/30 07:28:49  scott
| .
|
| Revision 5.1  2002/07/29 00:41:34  scott
| Updated lc_demo with new code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lc_demo.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LIB/BRAND/lc_demo/lc_demo.c,v 5.2 2002/07/30 07:28:49 scott Exp $";

#include <pslscr.h>
#include <_stlong.h>

extern	int	errno;

struct	lic_log
{
	long	brand_key;
	char	brand_password [23];
	char	brand_user [15];	
	long	brand_date;
	char	brand_time [6];
	char	brand_client [41];
	int		brand_logins;
	long	brand_expire;
	int		brand_terms;
} LogRec;

	char	*cus_space = "                                        ";

	struct
	{
		char	*pass_code;
		char	*pass_desc;
	} p_exp [] =
	{
		{ "A", "Alpha" },
		{ "B", "Bravo" },
		{ "C", "Charlie" },
		{ "D", "Delta" },
		{ "E", "Echo" },
		{ "F", "Foxtrot" },
		{ "G", "Golf" },
		{ "H", "Hotel" },
		{ "I", "India" },
		{ "J", "Juliet" },
		{ "K", "Kilo" },
		{ "L", "Lima" },
		{ "M", "Mike" },
		{ "N", "November" },
		{ "O", "Oscar" },
		{ "P", "Papa" },
		{ "Q", "Quebec" },
		{ "R", "Romeo" },
		{ "S", "Sierra" },
		{ "T", "Tango" },
		{ "U", "Uniform" },
		{ "V", "Victor" },
		{ "W", "Whiskey" },
		{ "X", "X-ray" },
		{ "Y", "Yankee" },
		{ "Z", "Zulu" },
		{ "a", "alpha" },
		{ "b", "bravo" },
		{ "c", "charlie" },
		{ "d", "delta" },
		{ "e", "echo" },
		{ "f", "foxtrot" },
		{ "g", "golf" },
		{ "h", "hotel" },
		{ "i", "india" },
		{ "j", "juliet" },
		{ "k", "kilo" },
		{ "l", "lima" },
		{ "m", "mike" },
		{ "n", "november" },
		{ "o", "oscar" },
		{ "p", "papa" },
		{ "q", "quebec" },
		{ "r", "romeo" },
		{ "s", "sierra" },
		{ "t", "tango" },
		{ "u", "uniform" },
		{ "v", "victor" },
		{ "w", "whiskey" },
		{ "x", "x-ray" },
		{ "y", "yankee" },
		{ "z", "zulu" },
		{ "0", "Zero" },
		{ "1", "One" },
		{ "2", "Two" },
		{ "3", "Three" },
		{ "4", "Four" },
		{ "5", "Five" },
		{ "6", "Six" },
		{ "7", "Seven" },
		{ "8", "Eight" },
		{ "9", "Nine" },
		{ "/", "Forward slash" },
		{ ".", "Full stop." },
		{ "","" },
	};

int		heading 		(int);
void	Update 			(void);
void	ProcessPassword (void);
void	LogisticEncrypt (struct	DES_REC	*);
int		OpenLog 		(void);
int		GetCode 		(int);
void	CloseLog 		(int);
/*----------------------------
| Local & Screen Structure   |
----------------------------*/
struct
{
	char	dummy [11];
	char	code [11];
	char	client [41];
} local_rec;

struct	DES_REC	des_rec;

static	struct	var	vars [] =
{
	{1, LIN, "client_key",	 4, 20, LONGTYPE,
		"NNNNNNNNN", "          ",
		"0", " ", "Client Key     : ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&des_rec.user_key},
	{1, LIN, "uname",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "           ",
		" ", " ", "User Name      : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.client},
	{1, LIN, "paswd",	 6, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAA", "           ",
		" ", " ", "Password       : ", " ",
		 NA, NO,  JUSTLEFT, "", "", des_rec.passwd},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	SETUP_SCR (vars);

	sprintf (LogRec.brand_user, "%-14.14s", getenv ("LOGNAME"));

	init_scr 	();
	clear 		();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	/*----------------------
	| Reset Control flags  |
	----------------------*/
	entry_exit = FALSE;
	edit_exit  = FALSE;
	prog_exit  = FALSE;
	restart    = FALSE;
	init_ok    = TRUE;
	search_ok  = TRUE;

	/*--------------------- 
	| Entry Screen input  |
	---------------------*/
	heading (1);
	scn_display (1);
	entry (1);
	des_rec.max_trm	=	999;
	des_rec.max_usr	=	4;
	des_rec.expiry	=	TodaysDate () + 120;
	ProcessPassword ();
	scn_display (1);
	crsr_off ();
	Update ();
	print_at (23, 29, " Please press any key ");
	getchar ();

	rset_tty ();
	return (EXIT_SUCCESS);
}

int
heading (
	int	scn)
{
	if (!restart)
	{
		clear ();

		centre_at (0, 80, "%R Logistic Software License Creation ");

		line_at (1, 0, 80);
		box (0, 3, 80, 3);
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

int
spec_valid (
	int	field)
{
	if (LCHECK ("client_key"))
	{
		if (des_rec.user_key > 0)
			return (EXIT_SUCCESS);

		sprintf (err_str, "Client key cannot be zero %c%s. ",
					toupper (LogRec.brand_user [0]),
					LogRec.brand_user + 1);

		errmess (err_str);
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (LCHECK ("uname"))
	{
		int		nameLength	=	(int) strlen (clip (local_rec.client));
		if (!strcmp (local_rec.client, cus_space) || !nameLength)
		{
			sprintf (err_str, "Client name must be input %c%s. ",
						toupper (LogRec.brand_user [0]),
						LogRec.brand_user + 1);

			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
ProcessPassword (void)
{
	LogisticEncrypt (&des_rec);
}

char	DES_init_perm []=
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

char	DES_last_perm []=
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

char	DES_e_perm []=
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

char	DES_p_perm []=
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

char	DES_k1_perm []=
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

char	DES_k2_perm []=
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

char	DES_s1_tabl []=
{
	14,  0,  4, 15, 13,  7,  1,  4,  2, 14, 15,  2, 11, 13,  8,  1,
	 3, 10, 10,  6,  6, 12, 12, 11,  5,  9,  9,  5,  0,  3,  7,  8,
	 4, 15,  1, 12, 14,  8,  8,  2, 13,  4,  6,  9,  2,  1, 11,  7,
	15,  5, 12, 11,  9,  3,  7, 14,  3, 10, 10,  0,  5,  6,  0, 13
};

char	DES_s2_tabl []=
{
	15,  3,  1, 13,  8,  4, 14,  7,  6, 15, 11,  2,  3,  8,  4, 14,
	 9, 12,  7,  0,  2,  1, 13, 10, 12,  6,  0,  9,  5, 11, 10,  5,
	 0, 13, 14,  8,  7, 10, 11,  1, 10,  3,  4, 15, 13,  4,  1,  2,
	 5, 11,  8,  6, 12,  7,  6, 12,  9,  0,  3,  5,  2, 14, 15,  9
};

char	DES_s3_tabl []=
{
	10, 13,  0,  7,  9,  0, 14,  9,  6,  3,  3,  4, 15,  6,  5, 10,
	 1,  2, 13,  8, 12,  5,  7, 14, 11, 12,  4, 11,  2, 15,  8,  1,
	13,  1,  6, 10,  4, 13,  9,  0,  8,  6, 15,  9,  3,  8,  0,  7,
	11,  4,  1, 15,  2, 14, 12,  3,  5, 11, 10,  5, 14,  2,  7, 12
};

char	DES_s4_tabl []=
{
	 7, 13, 13,  8, 14, 11,  3,  5,  0,  6,  6, 15,  9,  0, 10,  3,
	 1,  4,  2,  7,  8,  2,  5, 12, 11,  1, 12, 10,  4, 14, 15,  9,
	10,  3,  6, 15,  9,  0,  0,  6, 12, 10, 11,  1,  7, 13, 13,  8,
	15,  9,  1,  4,  3,  5, 14, 11,  5, 12,  2,  7,  8,  2,  4, 14
};

char	DES_s5_tabl []=
{
	 2, 14, 12, 11,  4,  2,  1, 12,  7,  4, 10,  7, 11, 13,  6,  1,
	 8,  5,  5,  0,  3, 15, 15, 10, 13,  3,  0,  9, 14,  8,  9,  6,
	 4, 11,  2,  8,  1, 12, 11,  7, 10,  1, 13, 14,  7,  2,  8, 13,
	15,  6,  9, 15, 12,  0,  5,  9,  6, 10,  3,  4,  0,  5, 14,  3
};

char	DES_s6_tabl []=
{
	12, 10,  1, 15, 10,  4, 15,  2,  9,  7,  2, 12,  6,  9,  8,  5,
	 0,  6, 13,  1,  3, 13,  4, 14, 14,  0,  7, 11,  5,  3, 11,  8,
	 9,  4, 14,  3, 15,  2,  5, 12,  2,  9,  8,  5, 12, 15,  3, 10,
	 7, 11,  0, 14,  4,  1, 10,  7,  1,  6, 13,  0, 11,  8,  6, 13
};

char	DES_s7_tabl []=
{
	 4, 13, 11,  0,  2, 11, 14,  7, 15,  4,  0,  9,  8,  1, 13, 10,
	 3, 14, 12,  3,  9,  5,  7, 12,  5,  2, 10, 15,  6,  8,  1,  6,
	 1,  6,  4, 11, 11, 13, 13,  8, 12,  1,  3,  4,  7, 10, 14,  7,
	10,  9, 15,  5,  6,  0,  8, 15,  0, 14,  5,  2,  9,  3,  2, 12
};

char	DES_s8_tabl []=
{
	13,  1,  2, 15,  8, 13,  4,  8,  6, 10, 15,  3, 11,  7,  1,  4,
	10, 12,  9,  5,  3,  6, 14, 11,  5,  0,  0, 14, 12,  9,  7,  2,
	 7,  2, 11,  1,  4, 14,  1,  7,  9,  4, 12, 10, 14,  8,  2, 13,
	 0, 15,  6, 12, 10,  9, 13,  0, 15,  3,  3,  5,  5,  6,  8, 11
};

char	DES_cvrt_tabl []=
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

void
DES_make_keys (char *src)
{
	char	cd [7];
	int	lcnt;

	DES_make_perm (cd, src, DES_k1_perm);

	for (lcnt = 0; lcnt < 16; lcnt++)
	{
		if (lcnt == 0 || lcnt == 1 || lcnt == 8 || lcnt == 15)
			DES_shft (cd, 1);
		else
			DES_shft (cd, 2);

		DES_make_perm (keys [lcnt], cd, DES_k2_perm);
	}
}

void
DES_make_perm (char *dst, char *src, char *perm_tabl)
{
	char    bit_cnt, chr_posn, mask;

	for (mask = 0; mask < (perm_tabl [1] / 8); mask++)
		* (dst + mask) = 0;
	for (bit_cnt = 0; bit_cnt < perm_tabl [1]; bit_cnt++)
	{
		chr_posn = ( (perm_tabl [bit_cnt + 2] - 1) / 8);
		mask = 0x80 >> ( (perm_tabl [bit_cnt + 2] - 1) % 8);
		mask = * (src + chr_posn) & mask;
		if (mask)
			* (dst + (bit_cnt / 8)) += (0x80 >> (bit_cnt % 8));
	}
}

void
DES_shft (char *src, int cnt)
{
	int     loop, tmp1, tmp2;

	tmp2 = 0;
	for (loop = 6; loop >= 0; loop--)
	{
		if (cnt == 2)
		{
			tmp1 = (* (src + loop) >> 6) & 0x03;
			* (src + loop) = ( (* (src + loop) << 2) & 0xfc) + tmp2;
			tmp2 = tmp1;
		}
		else
		{
			tmp1 = (* (src + loop) >> 7) & 0x01;
			* (src + loop) = ( (* (src + loop) << 1) & 0xfe) + tmp2;
			tmp2 = tmp1;
		}
	}
	if (cnt == 2)
	{
		* (src + 6) += (* (src + 3) >> 4) & 0x03;
		* (src + 3) &= 0xcf;
		tmp2 = (tmp2 << 4) & 0x30;
		* (src + 3) += tmp2;
	}
	else
	{
		* (src + 6) += (* (src + 3) >> 4) & 0x01;
		* (src + 3) &= 0xef;
		tmp2 = (tmp2 << 4) & 0x10;
		* (src + 3) += tmp2;
	}
}

void
DES_encode (
	char	*data, 
	char	*key)
{
	char    cc, tmp_e [6], tmp [4], tmp_p [4];
	int     cnt;

	for (cnt = 0; cnt < 4; cnt++)
	{
		cc = * (data + cnt);
		* (data + cnt) = * (data + cnt + 4);
		* (data + cnt + 4) = cc;
	}
	DES_make_perm (tmp_e, data, DES_e_perm);
	for (cnt = 0; cnt < 6; cnt++)
		tmp_e [cnt] ^= * (key + cnt);
	tmp_p [0]  = DES_s1_tabl [tmp_e [0] >> 2 & 0x3f] << 4;
	tmp_p [0] += DES_s2_tabl [ (tmp_e [0] << 4 & 0x30) + (tmp_e [1] >> 4 & 0x0f)];
	tmp_p [1]  = DES_s3_tabl [ (tmp_e [1] << 2 & 0x3c) + (tmp_e [2] >> 6 & 0x03)] << 4;
	tmp_p [1] += DES_s4_tabl [tmp_e [2] & 0x3f];
	tmp_p [2]  = DES_s5_tabl [tmp_e [3] >> 2 & 0x3f] << 4;
	tmp_p [2] += DES_s6_tabl [ (tmp_e [3] << 4 & 0x30) + (tmp_e [4] >> 4 & 0x0f)];
	tmp_p [3]  = DES_s7_tabl [ (tmp_e [4] << 2 & 0x3c) + (tmp_e [5] >> 6 & 0x03)] << 4;
	tmp_p [3] += DES_s8_tabl [tmp_e [5] & 0x3f];
	DES_make_perm (tmp, tmp_p, DES_p_perm);
	for (cnt = 4; cnt < 8; cnt++)
		* (data + cnt) ^= tmp [cnt - 4];
}

void
DES_bin_ascii (
	char	*dst, 
	char	*data)
{
	* (dst + 0) = DES_cvrt_tabl [*data >> 2 & 0x3f];
	* (dst + 1) = DES_cvrt_tabl [ ( (*data & 0x03) << 4) + (* (data + 1) >> 4 & 0x0f)];
	* (dst + 2) = DES_cvrt_tabl [ ( (* (data + 1) & 0x0f) << 2) + (* (data + 2) >> 6 & 0x03)];
	* (dst + 3) = DES_cvrt_tabl [* (data + 2) & 0x3f];
	* (dst + 4) = DES_cvrt_tabl [* (data + 3) >> 2 & 0x3f];
	* (dst + 5) = DES_cvrt_tabl [ ( (* (data + 3) & 0x03) << 4) + (* (data + 4) >> 4 & 0x0f)];
	* (dst + 6) = DES_cvrt_tabl [ ( (* (data + 4) & 0x0f) << 2) + (* (data + 5) >> 6 & 0x03)];
	* (dst + 7) = DES_cvrt_tabl [* (data + 5) & 0x3f];
	* (dst + 8) = DES_cvrt_tabl [* (data + 6) >> 2 & 0x3f];
	* (dst + 9) = DES_cvrt_tabl [ ( (* (data + 6) & 0x03) << 4) + (* (data + 7) >> 4 & 0x0f)];
	* (dst + 10) = DES_cvrt_tabl [ (* (data + 7) & 0x0f) << 2];
	* (dst + 11) = 0;
}

void
DES_encrypt (
	char	*dst, 
	char	*src, 
	char	*salt)
{
	char    tmp1 [8], tmp2 [8];
	int     count;

	DES_make_keys (salt);
	DES_make_perm (tmp1, src, DES_init_perm);
	for (count = 0; count < 16; count++)
		DES_encode (tmp1, keys [count]);
	DES_make_perm (tmp2, tmp1, DES_last_perm);
	DES_bin_ascii (dst, tmp2);
}

void
LogisticEncrypt (
	struct	DES_REC	*des_rec)
{
	char	tmp_key [11];
	char	inp_buf [8];
	long	tmp_rslt;

	sprintf (tmp_key, "%010ld", des_rec->user_key);
	stlong (des_rec->user_key, &inp_buf [0]);
	stlong (des_rec->expiry, &inp_buf [4]);
	DES_encrypt (des_rec->passwd, inp_buf, tmp_key);


	tmp_rslt  = des_rec->max_usr << 16;
	tmp_rslt  += des_rec->max_trm & 0xffff;
	stlong (tmp_rslt, &inp_buf [0]);
	tmp_rslt = des_rec->user_key;
	stlong (tmp_rslt, &inp_buf [4]);
	DES_encrypt (&des_rec->passwd[11], inp_buf, tmp_key);
}

/*=============
| Update log. |
=============*/
void
Update (void)
{
	int	fd = OpenLog ();

	LogRec.brand_key = des_rec.user_key;
	strcpy (LogRec.brand_password, des_rec.passwd);
	LogRec.brand_date = TodaysDate ();
	strcpy (LogRec.brand_time, TimeHHMM ());
	strcpy (LogRec.brand_client, local_rec.client);
	LogRec.brand_logins = des_rec.max_usr;
	LogRec.brand_expire = des_rec.expiry;
	LogRec.brand_terms = des_rec.max_trm;

	_cc = RF_ADD (fd, (char *)&LogRec);
	if (_cc)
		file_err (_cc, "BRAND.LOG", "WKADD");

	CloseLog (fd);
}

/*=================
| Open audit log. |
=================*/
int
OpenLog (void)
{
	int		fd;
	char	*basepath = getenv ("BASE_PATH");
	char	filename [100];

	sprintf (filename, "%s/ver.etc/BRAND/BRAND.LOG",
		basepath ? basepath : "/usr");

	/*---------------------------------------
	| If variable file doesn't exist	|
	---------------------------------------*/
	if (access (filename,00) < 0)
	{
		_cc = RF_OPEN (filename,sizeof (struct lic_log),"w",&fd);
		if (_cc)
			file_err (_cc, "BRAND.LOG", "WKOPEN");

		_cc = RF_CLOSE (fd);
		if (_cc)
			file_err (_cc, "BRAND.LOG", "WKCLOSE");
	}

	_cc = RF_OPEN (filename,sizeof (struct lic_log),"a",&fd);
	if (_cc)
		file_err (_cc, "BRAND.LOG", "WKOPEN");

	return (fd);
}

/*==================
| Close Audit log. |
==================*/
void
CloseLog (
	int	fd)
{
	_cc = RF_CLOSE (fd);
	if (_cc)
		file_err (_cc, "BRAND.LOG", "WKCLOSE");
}

int
GetCode (
	int	code)
{
	int	i;

	for (i = 0; strlen (p_exp [i].pass_code) ; i++)
		if (code == p_exp [i].pass_code [0])
			return (i);

	return (-1);
}
