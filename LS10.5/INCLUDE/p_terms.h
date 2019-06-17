#ifndef	_p_terms_h
#define	_p_terms_h
/*
 *
 *******************************************************************************
 *	$Log: p_terms.h,v $
 *	Revision 5.0  2001/06/19 06:51:47  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:28  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:28:58  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:43  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.6  1999/12/06 01:32:31  nz
 *	Fixed typo in #ifndef
 *	
 *	Revision 1.5  1999/12/06 00:57:04  jonc
 *	Added entry for GMS's 27A.
 *	
 */
	struct
	{

		char	*_pcode;
		char	*_pterm;

	} p_terms[] = {
		{"0  ", "0  - CASH ON DELIVERY.                  "},
		{"1  ", "1  - NETT 1 DAYS.                       "},
		{"2  ", "2  - NETT 2 DAYS.                       "},
		{"3  ", "3  - NETT 3 DAYS.                       "},
		{"4  ", "4  - NETT 4 DAYS.                       "},
		{"5  ", "5  - NETT 5 DAYS.                       "},
		{"6  ", "6  - NETT 6 DAYS.                       "},
		{"7  ", "7  - NETT 7 DAYS.                       "},
		{"8  ", "8  - NETT 8 DAYS.                       "},
		{"9  ", "9  - NETT 9 DAYS.                       "},
		{"10 ", "10 - NETT 10 DAYS.                      "},
		{"11 ", "11 - NETT 11 DAYS.                      "},
		{"12 ", "12 - NETT 12 DAYS.                      "},
		{"13 ", "13 - NETT 13 DAYS.                      "},
		{"14 ", "14 - NETT 14 DAYS.                      "},
		{"15 ", "15 - NETT 15 DAYS.                      "},
		{"16 ", "16 - NETT 16 DAYS.                      "},
		{"17 ", "17 - NETT 17 DAYS.                      "},
		{"18 ", "18 - NETT 18 DAYS.                      "},
		{"19 ", "19 - NETT 19 DAYS.                      "},
		{"20 ", "20 - NETT 20 DAYS.                      "},
		{"21 ", "21 - NETT 21 DAYS.                      "},
		{"22 ", "22 - NETT 22 DAYS.                      "},
		{"23 ", "23 - NETT 23 DAYS.                      "},
		{"24 ", "24 - NETT 24 DAYS.                      "},
		{"25 ", "25 - NETT 25 DAYS.                      "},
		{"26 ", "26 - NETT 26 DAYS.                      "},
		{"27 ", "27 - NETT 27 DAYS.                      "},
		{"28 ", "28 - NETT 28 DAYS.                      "},
		{"29 ", "29 - NETT 29 DAYS.                      "},
		{"30 ", "30 - NETT 30 DAYS.                      "},
		{"31 ", "31 - NETT 31 DAYS.                      "},
		{"32 ", "32 - NETT 32 DAYS.                      "},
		{"33 ", "33 - NETT 33 DAYS.                      "},
		{"34 ", "34 - NETT 34 DAYS.                      "},
		{"35 ", "35 - NETT 35 DAYS.                      "},
		{"36 ", "36 - NETT 36 DAYS.                      "},
		{"37 ", "37 - NETT 37 DAYS.                      "},
		{"38 ", "38 - NETT 38 DAYS.                      "},
		{"39 ", "39 - NETT 39 DAYS.                      "},
		{"40 ", "40 - NETT 40 DAYS.                      "},
		{"41 ", "41 - NETT 41 DAYS.                      "},
		{"42 ", "42 - NETT 42 DAYS.                      "},
		{"43 ", "43 - NETT 43 DAYS.                      "},
		{"44 ", "44 - NETT 44 DAYS.                      "},
		{"45 ", "45 - NETT 45 DAYS.                      "},
		{"46 ", "46 - NETT 46 DAYS.                      "},
		{"47 ", "47 - NETT 47 DAYS.                      "},
		{"48 ", "48 - NETT 48 DAYS.                      "},
		{"49 ", "49 - NETT 49 DAYS.                      "},
		{"50 ", "50 - NETT 50 DAYS.                      "},
		{"60 ", "60 - NETT 60 DAYS.                      "},
		{"90 ", "90 - NETT 90 DAYS.                      "},
		{"120", "120 - NETT 120 DAYS.                    "},
		{"150", "150 - NETT 150 DAYS.                    "},
		{"180", "180 - NETT 180 DAYS.                    "},
		{"20A", "20A-20TH OF 1ST MONTH AFTER DATE OF INV."},
		{"25A", "25A-20TH OF 1ST MONTH AFTER DATE OF INV."},
		{"27A", "27A-20TH OF 1ST MONTH AFTER DATE OF INV."},
		{"20B", "20B-20TH OF 2ND MONTH AFTER DATE OF INV."},
		{"25B", "25B-25TH OF 2ND MONTH AFTER DATE OF INV."},
		{"20C", "20C-20TH OF 3RD MONTH AFTER DATE OF INV."},
		{"25C", "25C-25TH OF 3RD MONTH AFTER DATE OF INV."},
		{"20D", "20D-20TH OF 4TH MONTH AFTER DATE OF INV."},
		{"25D", "25D-25TH OF 4TH MONTH AFTER DATE OF INV."},
		{"20E", "20E-20TH OF 5TH MONTH AFTER DATE OF INV."},
		{"25E", "25E-25TH OF 5TH MONTH AFTER DATE OF INV."},
		{"20F", "20F-20TH OF 6TH MONTH AFTER DATE OF INV."},
		{"25F", "25F-25TH OF 6TH MONTH AFTER DATE OF INV."},
		{"20G", "20G-20TH OF 7TH MONTH AFTER DATE OF INV."},
		{"25G", "25G-25TH OF 7TH MONTH AFTER DATE OF INV."},
		{"30A", "30A-LAST DAY OF 1ST MONTH AFTER INVOICE."},
		{"30B", "30B-LAST DAY OF 2ND MONTH AFTER INVOICE."},
		{"30C", "30C-LAST DAY OF 3RD MONTH AFTER INVOICE."},
		{"30D", "30D-LAST DAY OF 4TH MONTH AFTER INVOICE."},
		{"30E", "30E-LAST DAY OF 5TH MONTH AFTER INVOICE."},
		{"30F", "30F-LAST DAY OF 6TH MONTH AFTER INVOICE."},
		{"30G", "30G-LAST DAY OF 7TH MONTH AFTER INVOICE."},
		{"",""},
		{"",""},
	};

#endif	/* _p_terms_h */
