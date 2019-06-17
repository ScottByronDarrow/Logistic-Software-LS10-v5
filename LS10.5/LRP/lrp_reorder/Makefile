#
#	Standard application Makefile template
#
############################################
#
#		$Id: Makefile,v 5.1 2001/08/06 23:27:42 scott Exp $
#		$Log: Makefile,v $
#		Revision 5.1  2001/08/06 23:27:42  scott
#		RELEASE 5.0
#		
#		Revision 5.0  2001/06/19 08:07:37  robert
#		LS10-5.0 New Release as of 19 JUNE 2001
#		
#		Revision 4.0  2001/03/09 02:28:45  scott
#		LS10-4.0 New Release as at 10th March 2001
#		
#		Revision 3.0  2000/10/10 12:15:37  gerry
#		Revision No. 3 Start
#		<after Rel-10102000>
#		
#		Revision 2.0  2000/07/15 08:58:45  gerry
#		Forced Revision No. Start 2.0 Rel-15072000
#		
#		Revision 1.3  2000/06/05 05:09:07  scott
#		S/C LSANZ-16401
#		Updated to include processing of works orders. Currently purchase orders and
#		transfers were processed but not works orders. Program processes work orders
#		when source is "BP", "BM", "MC" and "MP" items, otherwise processing is a normal
#		NOTE : The schema sch.pcwo required an additional index.
#		
#
#

APP		=	lrp_reorder
SRCS	=	_reorder.c
ALTS	=	lrp_creorder lrp_breorder

include ${BASE_PATH}/${VERSION}/CONFIG/Make.CBase
