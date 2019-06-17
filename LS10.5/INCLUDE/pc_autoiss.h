/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_autoiss.h,v 5.0 2001/06/19 06:51:47 cha Exp $
|  Program Name  : (pc_autoiss.h) 
|  Program Desc  : (Automatic Production Control Issue raw-materials)
|---------------------------------------------------------------------|
|  Date Written  : 29th March 2001 | Author       : Scott B Darrow    |
|---------------------------------------------------------------------|
| $Log: pc_autoiss.h,v $
| Revision 5.0  2001/06/19 06:51:47  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.3  2001/04/30 04:43:27  scott
| Updated for write off account addition
|
| Revision 4.2  2001/03/29 07:30:19  scott
| Updated to remove unused extern
|
| Revision 4.1  2001/03/29 06:11:53  scott
| New include for new program pc_autoiss.h
| Automatic Stock Issues to Production from XML files.
|
=====================================================================*/
#define		ERR_WOI_BAD_FLAG	1
#define		ERR_WOI_NO_ORDER	2
#define		ERR_WOI_NO_LINE		3
#define		ERR_WOI_NO_MKU		4
#define		ERR_WOI_NO_SKU		5
#define		ERR_WOI_NO_CCMR		6
#define		ERR_WOI_NO_INCC		7
#define		ERR_WOI_NO_INEI		8
#define		ERR_WOI_LOTALLOC	9
#define		ERR_WOI_SOH			10
#define		ERR_WOI_WOFFCODE	11
