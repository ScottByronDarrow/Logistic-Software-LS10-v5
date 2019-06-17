#
#  Make Script for libbase
#
################################################################################
#	$Id: Makefile,v 5.10 2002/10/22 06:19:53 robert Exp $
#	
#	$Log: Makefile,v $
#	Revision 5.10  2002/10/22 06:19:53  robert
#	Initial check-in. XML functions
#	
#	Revision 5.9  2002/04/29 05:38:31  scott
#	Added Archive.c
#	
#	Revision 5.8  2001/10/26 00:53:31  scott
#	Update make file to remove unused file.
#	
#	Revision 5.7  2001/09/27 02:33:41  cha
#	New program for auditing.
#	
#	Revision 5.6  2001/09/11 23:28:11  scott
#	Updated from Scott machine - 12th Sep 2001
#	
#	

SHELL	= /bin/sh
LIBNAME	=	LogisticSoftware

CDEF	=	-D${MACH_TYPE} 
CINC	=	-I../../INCLUDE -I${INFORMIXDIR}/include

CFLAGS	= -s -O3 ${CDEF} ${CINC} \
		-DDEBUG \
		-DVERSION="\"${VERSION}\"" \
		-DOS_VER="\"`uname -r`\"" \
		-DLIBNAME="\"$(LIBNAME)\""

LIB	=	../libbase$(VERSION).a

SRCS	=	CheckIndent.c		Costing.c			CustService.c \
			DateToString.c		DbBalWin.c			DeleteControl.c \
			Dsp_heading.c		FinancialDates.c	FindCumr.c \
			FindInmr.c			FindSumr.c			FinishProgram.c \
			FormatP.c			GlAudit.c			GlFormat.c \
			GlUtils.c			InumGen.c			LocCode.c \
			ML.c				OpenSpecial.c		PrintReport.c \
			RF_ADD.c			RF_CLOSE.c			RF_DELETE.c \
			RF_DISPLAY.c		RF_OPEN.c			RF_READ.c \
			RF_REWIND.c			SleepDelay.c		StringToDate.c \
			SystemTime.c		TimeStrings.c		TodaysDate.c \
			UpdateWOStatus.c	XML_Error.c			account.c \
			adj_money.c			age_bals.c			age_per.c \
			alt_hash.c			arralloc.c			blank_at.c \
			cal_select.c		centre_at.c			check_fiscal.c \
			check_inuv.c		check_login.c		check_page.c \
			check_short.c		chq_date.c			clip.c \
			comma_fmt.c			configfile.c		DBAudit.c \
			dbgroup.c			 dbltow.c			debug.c	\
			dflt_env.c 			dso_vars.c			dsp_utils.c \
			error_utils.c 		expand.c			fast.c	\
			file_err.c 			file_trunc.c		fisc_year.c	\
			foreground.c 		gen_ser_no.c		get_cdate.c	\
			get_env.c 			get_eoy.c			get_fdmy.c	\
			get_lpno.c 			get_secure.c		get_ybeg.c \
			get_yend.c 			getnum.c			glob_vars.c	\
			graph.c 			hot_keys.c			input_utils.c \
			ip_comms.c 			ip_print.c			lc_check.c	\
			lc_i_no.c 			lc_io.c				line_at.c	\
			menu_utils.c 		minimenu.c			mod_env.c \
			no_option.c 		number.c			open_env.c \
			out_cost.c 			pDate.c				pad_num.c	\
			pause_key.c 		per_val.c			pin_bcopy.c \
			pin_bfill.c 		pin_time.c			pr_format3.c \
			print_at.c 			print_err.c			Archive.c \
			prmptmsg.c 			proc_sobg.c			psl_decrypt.c \
			psl_print.c 		psl_round.c			ptab_scan.c	\
			put_env.c 			rdchk.c				read_comm.c \
			ring_menu.c			rnd_mltpl.c			search_utils.c \
			secs.c 				ser_msg.c			ser_value.c	\
			set_file.c			shift.c				sleeper.c	\
			sort_utils.c 		spec_valid.c		stk_vars.c	\
			str_token.c 		strip_form.c		strsave.c	\
			sys_exec.c 			sys_log.c			tab_other.c	\
			tabdisp.c			tag_other.c			tag_utils.c	\
			tc_cbox.c 			tc_ebox.c			tcap.c	\
			time_day.c			toggle_crsr.c		tty_slot.c \
			ttyctl.c			wild_card.c			wild_search.c \
			win_select.c 		LSXMLInterface.c	XMLCommon.c \
			XMLReader.c			XMLWriter.c

OBJS	=	${SRCS:.c=.o}

all:	depend ${LIB}

${LIB}:	${OBJS}
	${AR} srv $@ $?

depend:	${SRCS}
	w4 ${CINC} ${CDEF} ${SRCS} > $@

clean:
	cp /dev/null depend
	-touch 01010000 depend
	rm -f *.o

include depend
