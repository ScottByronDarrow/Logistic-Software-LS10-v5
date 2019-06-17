/*
 *	Prototypes for every single function in the system
 *
 *******************************************************************************
 *	$Log: ProtosBASE.h,v $
 *	Revision 5.2  2001/08/06 22:49:50  scott
 *	RELEASE 5.0
 *	
 *	Revision 5.1  2001/07/25 01:01:56  scott
 *	Updated for 10.5
 *	
 *	Revision 5.0  2001/06/19 06:51:19  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.5  2001/05/14 05:36:44  scott
 *	New Function XML_Error to log XML errors.
 *	
 *	Revision 4.4  2001/03/28 07:11:34  scott
 *	Updated to add function define for UpdateWOStatus.c
 *	
 *	Revision 4.3  2001/03/27 06:15:08  scott
 *	Updated to change arguments passed to DbBalWin to remove need for comm
 *	
 *	Revision 4.2  2001/03/26 10:19:21  scott
 *	Added new function DbBalWin.h
 *	
 *	Revision 4.1  2001/03/22 01:21:32  scott
 *	Updated to add define for sleep functions
 *	
 *	Revision 4.0  2001/03/09 00:59:21  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.5  2001/02/14 09:57:02  scott
 *	Updated to use const
 *	
 *	Revision 3.4  2001/02/14 09:29:14  scott
 *	Updated for changed to OpenSpecial
 *	
 *	Revision 3.3  2001/02/14 09:14:06  scott
 *	Updated to add new defined for OpenSpecial
 *	
 *	Revision 3.2  2001/01/29 04:06:04  scott
 *	Updated to clean/lineup, call me fussy
 *	
 *	Revision 3.1  2001/01/29 03:44:06  scott
 *	Updated to add YearEnd Function
 *	
 *	Revision 3.0  2000/10/12 13:28:51  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.2  2000/10/10 06:08:31  scott
 *	Updated to fix problem prototypes
 *	
 *	Revision 2.1  2000/10/04 06:41:22  scott
 *	Updated to change file_err to use const char to avoid warnings.
 *	
 *	Revision 2.0  2000/07/15 07:15:35  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.17  2000/02/07 04:53:57  scott
 *	Added sort_reopen (added by Trevor.)
 *	
 *	Revision 1.16  2000/01/10 21:43:20  cam
 *	Modified for new sort API sort_reopen ().
 *	
 *	Revision 1.15  1999/11/26 01:01:19  jonc
 *	AgePeriod() extended to include true_age parameter.
 *	
 *	Revision 1.14  1999/11/25 03:47:02  jonc
 *	Updated CalcDueDate to use std date routines, tightened prototypes.
 *	
 *	Revision 1.13  1999/11/19 03:57:02  scott
 *	Updated to remove old date functions
 *	
 *	Revision 1.12  1999/11/19 03:35:35  scott
 *	Updated to remove old date routines.
 *	
 *	Revision 1.11  1999/11/15 23:58:36  scott
 *	Updated for license routines.
 *	
 *	Revision 1.10  1999/11/15 23:21:16  scott
 *	Updated for Licence
 *	
 *	Revision 1.9  1999/11/15 06:47:05  scott
 *	Updated for compile problems on AIX
 *	
 *	Revision 1.8  1999/10/20 07:20:58  nz
 *	Updated for date routines
 *	
 *	Revision 1.7  1999/10/11 22:34:05  cam
 *	Fixed prototype for set_wdays ()
 *	
 *	Revision 1.6  1999/10/08 03:37:32  jonc
 *	Removed prototypes for INFORMIX functions. Now defined in isam.h
 *	
 *	Revision 1.5  1999/10/04 03:43:55  jonc
 *	Updated read_comm to reflect reality
 *	
 *	Revision 1.4  1999/09/30 04:32:52  jonc
 *	Tightened the prototype callist for sys_err.
 *	
 */
/* CustService.c */
extern void LogCustService (long int hhcoHash, long int hhsoHash, long int hhcuHash, char *customerOrderRef, char *consignmentNo, char *carrierCode, char *deliveryZone, int serviceType);
extern void UpdateSosf (long int hhcuHash, long int hhsoHash, long int hhcoHash);

/* DateToString.c */
extern const char *MonthName 		(int);
extern const char *ShortMonthName 	(int);
extern const char *DateToString 	(Date);
extern const char *DateToDDMMYY 	(Date);
extern char *DateToFmtString 		(Date, const char *, char *);

/* Dsp_heading.c */
extern int Dsp_heading 				(void);

/* FinancialDates.c */
extern Date FinYearStart 			(Date, int);
extern Date FinYearEnd 				(Date date, int);
extern void GetFinYear 				(Date date, int, Date *, Date *);
extern void DateToFinDMY 			(Date, int, int *, int *, int *);
extern Date FinDMYToDate 			(int, int, int, int);
extern Date YearEnd 				(void);

/* ML.c */
extern char	*ML 					(char *);
extern void ML_Open 				(void);
extern char *ML_Find 				(char *);
extern int 	lang_select 			(void);
extern char *ML_Clip 				(char *);

/* StringToDate.c */
extern Date StringToDate 			(const char *);

/* add_batch.c */
extern char *add_batch 				(void);
extern void open_batch_db 			(void);
extern void close_batch_db 			(int);
extern int empty 					(char *);

/* adj_money.c */
extern double adj_money 			(double);
extern double adj_val 				(double);

/* age_bals.c */
extern int age_bals 				(int, long, long);

/* age_per.c */
extern int AgePeriod 				(char *, Date, Date, Date, int, int);
extern Date CalcDueDate 			(const char *, Date);

/* alt_hash.c */
extern long int alt_hash 			(long, long);

/* arralloc.c */
extern void ArrAlloc 				(DArray *, void *, size_t, size_t);
extern void ArrDelete 				(DArray *);
extern int ArrChkLimit 				(DArray *, void *, int);

/* blank_at.c */
extern void blank_at 				(int, int, int);

/* cal_select.c */
#include	<cal_select.h>
extern struct CAL_STORE *set_calendar (struct CAL_STORE *, long, int, int);
extern long int cal_select 			(int, int, int, int, int, struct CAL_STORE *, struct CAL_INFO *, int);
extern int draw_calendar 			(int, int, int, int, int, struct CAL_STORE *, struct CAL_INFO *, int);

/* centre_at.c */
extern void centre_at 				(int, int, char *, ...);

/* check_fiscal.c */
extern void check_fiscal 			(void);

/* check_inuv.c */
extern int ValidItemUom 			(long, long);
extern void AddINUV 				(long, long);

/* check_login.c */
extern int check_login 				(unsigned int, int);
extern int check_logout 			(unsigned int);

/* InumGen.c */
extern	long	GenerateUOM 		(const char *, const char *, float, long);

/* check_page.c */
extern int check_page 				(void);

/* check_short.c */
extern int check_short 				(char *);
extern int check_class 				(char *);

/* chq_date.c */
extern int chq_date 				(long, long);
extern void _cl_space 				(void);

/* clip.c */
extern char *clip 					(char *);
extern char *lclip 					(char *);
extern char *ltrim 					(char *, char *);

/* comma_fmt.c */
extern char *comma_fmt 				(double, char *);

/* dbgroup.c */
#include	<dbgroup.h>
extern int OpenStatementGroups 		(long);
extern int CloseStatementGroups 	(void);
extern int LoadStatementGroups 		(DGroupSet *, long, long, int, FILE *);
extern void FreeStatementGroups 	(DGroupSet *);
extern struct tag_DGroup *GetFirstGroup (DGroupSet *);
extern struct tag_DGroup *GetNextGroup (DGroup *);
extern struct tag_DGroupItem *GetFirstGroupItem (DGroup *);
extern struct tag_DGroupItem *GetNextGroupItem (DGroupItem *);
extern int GetGroupCount 			(DGroup *);
extern double GetGroupValue 		(DGroup *);
extern long int GetLastTransactionDate (DGroup *);

/* dbltow.c */
extern char *dbltow 				(double, char *, char *);
extern int atod 					(char *, char *);

/* debug.c */
extern void SetLog 					(const char *);
extern int WriteLog 				(char *, ...);

/* dflt_env.c */
extern char *dflt_env 				(char *, char *);

/* dso_vars.c */

/* dsp_utils.c */
extern void Dsp_nd_prn_open 		(int, int, int, char *, char *, char *, char *, char *, char *, char *);
extern void Dsp_nc_prn_open 		(int, int, int, char *, char *, char *, char *, char *, char *, char *);
extern void Dsp_prn_open 			(int, int, int, char *, char *, char *, char *, char *, char *, char *);
extern void Dsp_nc_open 			(int, int, int);
extern void Dsp_open 				(int, int, int);
extern void _Dsp_open 				(int, int, int, char *, char *, char *, char *, char *, char *, char *, int, int, int);
extern void Dsp_close 				(void);
extern int Dsp_saverec 				(char *);
extern int Dsp_save_fn 				(char *, char *);
extern int _null_fn 				(char *);
extern int Dsp_srch 				(void);
extern int Dsp_srch_fn 				(int (*_run_fn) (char *));
extern int Dsp_srch_fn2 			(int (*_run_fn) (char *), int * key_list);
extern int Dsp_srch_grph 			(int *, int *);
extern int _Dsp_srch_fn 			(int, int (*_run_fn)(char *), int *, int *);

extern int _line_down (int line_no, int page_size);
extern int _line_up (int line_no, int page_size);
extern void _show_display (int line_no);
extern void _show_line (int line_no, int rv);
extern int _Load_display (int page_no);
extern int _Load_d_line (int line_no);
extern void Clear_display (void);
extern int _Print_display (void);
extern void _Parse_str (char *out_str, char *in_str, char *_extra, int page_width);
extern struct _dsp_type *_dsp_alloc (void);
extern int Dsp_print (void);
extern int cnt_carets (char *_string);

/* error_utils.c */
extern void sys_err (const char *text, int value, const char *prg_name);

/* expand.c */
extern char *expand (char *ex_to, char *ex_from);

/* fast.c */
extern int get_fast (char *fast_key);
extern int chk_fast (char *user, char *fast_key);
extern int open_fast (char *user_name, char *mode);
extern int add_fast (int fd, char *fast_code, char *menu_file, int fast_opts);
extern void close_fast (int fd);
extern int _chk_security (char *_secure, char *_security);

/* file_err.c */
extern void file_err (int, const char *, const char *);

/* NB: TvB I have intentionally moved file_trunc.h to AFTER tabdisp.c */
/* as it makes use of the TAB_PTR typedef */

/* fisc_year.c */
extern int fisc_year (long int c_date);

/* foreground.c */
extern int foreground (void);

/* form_accbit.c */
extern long int form_accbit (char *acc_str, int width);

/* gen_ser_no.c */
extern int GenNextSerNo (char *firstSerNo, int firstTime, char *newSerNum);

/* genlook.c */
extern void _LOOK_start (char *);
extern void _LOOK_index (char *);
extern void _LOOK_heading (char *);
extern void _LOOK_fields (char *);
extern void _LOOK_view (struct dbview *);
extern void _LOOK_size (int);
extern void _LOOK_object (char *);
extern void _LOOK_pre (int (*) (char *));
extern void _LOOK_function (int (*) (char *, char *, int *));
extern void _LOOK_row (int);
extern void _LOOK_col (int);
extern void _LOOK_depth (int);
extern void _LOOK_keep (int);
extern void _LOOK_scan (int);
extern void _LOOK_hots (KEY_PTR);
extern void _LOOK_up (void);
extern void _LOOK_clean (void);

/* get_cdate.c */
extern void get_cdate (Date *c_date, short int *fdmy);

/* get_env.c */
extern char *chk_env (char *vble_name);
extern char *get_env (char *vble_name);

/* get_eoy.c */
extern long int get_eoy (long int cur_date, int fiscal);

/* get_fdmy.c */
extern void get_fdmy (short int *fdmy, long int c_date);

/* get_lpno.c */
extern int valid_lp (int chk_lpno);
extern int get_lpno (int dflt_lpno);

/* get_secure.c */
extern int chk_secure (char *co_no, char *br_no);
extern void add_secure (int fd, char *user_name, char *co_no, char *br_no);
extern int open_secure (void);
extern void close_secure (int fd);

/* get_ybeg.c */
extern long int get_ybeg (long int c_date);

/* get_yend.c */
extern long int get_yend (long int c_date, int nxt_flg);

/* getnum.c */
extern double getnum (int x, int y, int btype, char *mask);
extern int getint (int x, int y, char *mask);
extern long int getlong (int x, int y, char *mask);
extern float getfloat (int x, int y, char *mask);
extern double getdouble (int x, int y, char *mask);
extern double getmoney (int x, int y, char *mask);
extern void getalpha (int x, int y, char *mask, char *buf);
extern void getalpha2 (int x, int y, char *mask, char *buf);
extern void get_date (int x, int y, char *mask, char *buf);

/* glcons.c
extern void cons_start (char *co_no, char *co_name, int budg_no, int lp_no);
extern void cons_post (char *acc_no, long int c_date, double amount);
extern int cons_end (char *jtype, int pid);
*/

/* glob_vars.c */

/* graph.c */
extern int GR_graph (struct GR_WINDOW *window, int gr_type, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits);

/* hot_keys.c */
extern void disp_hotkeys (int key_row, int key_col, int scr_width, KEY_PTR key_tab);
extern int run_hotkeys (KEY_PTR key_tab, int (*pre_func) (int, KEY_PTR), int (*post_func) (int, KEY_PTR));
extern int set_keys (KEY_PTR key_tab, char *tag, int stat);
extern void set_help (int key, char *msg);
extern void disp_help (int width);
extern int null_func (int up_c, KEY_PTR key_tab);

/* input_utils.c */
extern char *mid (char *s, int n, int l);
extern char *string (int nbr, char *chr);

/* ip_comms.c */
extern int IP_CREATE (int uniqid);
extern int IP_OPEN (int uniqid);
extern void IP_READ (int np_fn);
extern void IP_WRITE (int np_fn);
extern void IP_CLOSE (int np_fn);
extern void IP_UNLINK (int uniqid);

/* ip_print.c */
extern void ip_open (char *co_no, char *br_no, int lpno);
extern void _ip_open (char *co_no, char *br_no, int lpno, char *EnvName, char *ProgName);
extern void ip_close (void);
extern void ip_print (long int ip_hash);
extern char *sv_check (char *_env, char *_prg, char *_co_no, char *_br_no);

/* lc_check.c */
extern int lc_check (struct DES_REC *des_rec, struct LIC_REC *lic_rec);

/* lc_i_no.c */
extern long int lc_i_no (void);

/* lc_io.c */
extern void lc_read (struct LIC_REC *license);
extern void lc_write (struct LIC_REC *license);

/* line_at.c */
extern void line_at (int row, int col, int len);

/* menu_utils.c */
extern void new_menu (char *mname);
extern void sub_menu (char *mname);
extern int _add_menu (char *mname, char is_sub);
extern void init_mtab (void);

/* minimenu.c */
extern void mmenu_print (char *m_hdr, MENUPTR m_ptr, int curr_opt);
extern void mmenu_scan (MENUPTR m_ptr);
extern int mmenu_select (MENUPTR m_ptr);

/* mod_env.c */
extern char *mod_env (char *e_var, char *e_val);

/* no_option.c */
extern void no_option (char *env_desc);

/* number.c */
#include	<number.h>
extern void NumAsc (number *n, char *c);
extern void NumShort (number *n, short int v);
extern void NumInt (number *n, int v);
extern void NumLong (number *n, long int v);
extern void NumFlt (number *n, float v);
extern void NumDbl (number *n, double v);
extern char *NumToAsc (number *n, char *c);
extern short int NumToShort (number *n);
extern int NumToInt (number *n);
extern long int NumToLong (number *n);
extern float NumToFlt (number *n);
extern double NumToDbl (number *n);
extern void NumAdd (number *n1, number *n2, number *result);
extern void NumSub (number *n1, number *n2, number *result);
extern void NumMul (number *n1, number *n2, number *result);
extern void NumDiv (number *n1, number *n2, number *result);
extern int NumCmp (number *n1, number *n2);
extern void NumCpy (number *n1, number *n2);

/* open_env.c */
extern int open_env (void);
extern void close_env (int fd);

/* out_cost.c */
extern double out_cost (double cost, float outer_size);

/* pDate.c */
extern void DateToDMY (Date date, int *d, int *m, int *y);
extern Date DMYToDate (int d, int m, int y);
extern int DaysInMonth (Date date);
extern int DaysInMonthYear (int month, int year);
extern int DaysInYear (Date date);
extern enum _DayOfWeek DayOfWeek (Date date);
extern int IsWeekDay (Date date);
extern int IsLeapYear (Date date);
extern Date AddMonths (Date date, int months);
extern Date AddYears (Date date, int years);
extern Date MonthStart (Date date);
extern Date MonthEnd (Date date);
extern int FullYear (void);

/* pad_num.c */
extern char *zero_pad (char *pad_str, int pad_len);
extern char *pad_num (char *istring);
extern char *pad_batch (char *bstring);

/* per_val.c */
extern int val_period (char *per_num, long int gl_date);

/* pin_bcopy.c */
extern void pin_bcopy (char *to, char *from, unsigned int siz);

/* pin_bfill.c */
extern void pin_bfill (char *to, char f_char, unsigned int siz);

/* pin_time.c */
extern void set_wdays (int dom, int dow);
extern int month_wdays (int dom, int dow);
extern int is_weekday (int dom);

/* TodaysDate.c */
extern long int TodaysDate (void);

/* pr_format3.c */
extern FILE *pr_open (char *filename);
extern int pr_format (FILE *fin, FILE *fout, char *label, int fld_no, ...);

/* print_at.c */
extern void print_at (int row, int col, char *mask, ...);

/* print_err.c */
extern int print_err (char *mask, ...);

/* PrintReport.c */
extern void PrintReport (char *, char *, int);

/* prmptmsg.c */
extern int prmptmsg (char *text, char *allowed, int x, int y);

/* proc_sobg.c */
extern void add_hash (char *_co_no, char *_br_no, char *_type, int _lpno, long int _hhbr_hash, long int _hhcc_hash, long int _pid, double _value);
extern void add_hash_RC (char *_co_no, char *_br_no, long int _hhbr_hash, long int _hhcc_hash, long int _pid, int _last_line);
extern void recalc_sobg (void);

/* psl_clock.c */
extern void cl_sigset (int x);
extern void print_clock (int x, int y);
extern void print_aclock (int x, int y);
extern void _print_clock (int x, int y, int am_pm);
extern void init_clock (void);
extern void proc_clock (int x, int y, int h, int m, int s, int am_pm);

/* psl_decrypt.c */
extern void DES_make_keys (char *src);
extern void DES_make_perm (char *dst, char *src, char *perm_tabl);
extern void DES_shft (char *src, int cnt);
extern void DES_decode (char *data, char *key);
extern int DES_ascii_bin (char *data, char *dst);
extern int DES_look_tabl (char cc);
extern int DES_decrypt (char *src, char *dst, char *salt);
extern void psl_decrypt (struct DES_REC *des_rec);

/* psl_print.c */
extern void psl_print (void);

/* psl_round.c */
extern double psl_round (double x, int n);
extern double round (double d);
extern double fourdec (double d);
extern double threedec (double d);
extern double twodec (double d);
extern double onedec (double d);
extern double no_dec (double d);
extern double tw_no_dec (double d);
extern double n_dec (double d, int n);

/* put_env.c */
extern void put_env (char *vble_name, char *vble_value, char *vble_desc);

/* rdchk.c */
extern int rdchk (int fd);

/* read_comm.c */
extern void read_comm (struct dbview *comm_view, int comm_no, void * obj);

/* rnd_mltpl.c */
extern float rnd_mltpl (char *rnd_type, float ord_mltpl, float ord_qty);

/* rstrdate.c */
extern void rstrdate (char *str, long int *l_date, int type);

/* search_utils.c */
extern int work_open (void);
extern int _work_open (int CodeLength, int DispLength, int DescLength);
extern void work_close (void);
extern int save_rec (char *field1, char *field2);
extern int disp_srch (void);
extern int _disp_srch (void);
extern void _draw_page (int disp);
extern int _load_page (int max_lines, int lines);
extern int _load_line (int Offset, int line_no);

/* secs.c */
extern void txt_init (void);
extern int txt_open (int y, int x, int dy, int dx, int max_y, char *hdg);
extern int txt_display (int window, int line);
extern int txt_close (int window, int clr_flg);
extern int txt_pval (int window, char *text, int line);
extern char *txt_gval (int window, int line);
extern int txt_edit (int window);
extern int txt_scn_edit (struct var *tmp_vars, int tmp_scn_start);
extern int _txt_edit (int window, struct var *tmp_vars, int tmp_scn_start);

/* ser_msg.c */
extern void ser_msg (int error_no, struct LIC_REC *lic_rec, int show);

/* set_file.c */
extern void set_file (char *_filename);

/* shift.c */
extern char *upshift (char *string);
extern char *downshift (char *string);

/* sleeper.c */
extern void time_out (void);
extern void signal_on (void);
extern void set_timer (void);

/* sort_utils.c */
extern FILE *sort_open (char *filename);
extern FILE *sort_reopen (char *filename);
extern int sort_rewind (FILE *stream);
extern void sort_save (FILE *stream, char *data);
extern FILE *dsort_sort (FILE *stream, char *filename);
extern FILE *sort_sort (FILE *stream, char *filename);
extern char *sort_read (FILE *stream);
extern int sort_delete (FILE *stream, char *filename);

/* spec_valid.c */
extern int spec_valid (int field);

/* stk_vars.c */

/* str_token.c */
extern char *str_token (char *, char *);

/* strsave.c */
extern char *p_strsave (char *);

/* sys_exec.c */
extern int sys_exec (char *);
extern int SystemExec (char *, int);

/* sys_log.c */
extern void sys_log (enum SysLogFile log_type, char *fmt, ...);

/* tab_other.c */
extern void tab_other (int x);

/* tabdisp.c */
extern void tab_open (char *f_name, KEY_PTR xtra_keys, int row, int col, int depth, int mem_flag);
extern void tab_srch (char *f_name, int srch);
extern void tab_keyset (char *f_name, KEY_PTR xtra_keys);
extern void tab_close (char *f_name, int init_flag);
extern int tab_update (char *f_name, char *mask, ...);
extern int tab_add (char *f_name, char *mask, ...);
extern void tab_overide (char *nm);
extern char *tab_name (char *nm);
extern int tab_get (char *f_name, char *l_buf, int get_flag, int line_no);
extern int tab_tline (char *f_name);
extern int tab_sline (char *f_name);
extern void redraw_form (char *f_name);
extern void redraw_heading (char *f_name, int r_flag);
extern void redraw_page (char *f_name, int r_flag);
extern void redraw_line (char *f_name, int r_flag);
extern void redraw_keys (char *f_name);
extern void redraw_table (char *f_name);
extern int tab_display (char *f_name, int disp_flag);
extern void tab_clear (char *f_name);
extern void blank_table (char *f_name);
extern void load_page (char *f_name, int r_flag);
extern TAB_PTR find_tabname (char *f_name);
extern TAB_PTR add_tabname (char *f_name);
extern int tab_scan (char *f_name);
extern int clear_file (TAB_PTR work_tab, long int size);

/* file_trunc.c */
extern int file_trunc (TAB_PTR work_tab, long int size);

/* tag_other.c */
extern void tag_other (void);
extern void ptab_scan (void);

/* tag_utils.c */
extern int tag_toggle (char *t_name);
extern int tag_all (char *t_name);
extern int tag_set (char *t_name);
extern int tag_unset (char *t_name);
extern int tagged (char *s);

/* tc_cbox.c */
extern void cl_box (int x, int y, int h, int v);

/* tc_ebox.c */
extern void erase_box (int x, int y, int h, int v);

/* tcap.c */
extern int init_scr (void);
extern void line (int x);
extern void box (int x, int y, int h, int v);
extern int load_ (char *load_line);
extern int atoo (char *str);
extern void move (int x, int y);
extern void clear_mess (void);
extern void print_mess (char *message);
extern void errmess (char *errm);
extern void li_pr (char *rv_desc, int rv_x, int rv_y, int rv_flag);
extern void rv_pr (char *rv_desc, int rv_x, int rv_y, int rv_flag);
extern void fl_pr (char *fl_desc, int fl_x, int fl_y, int fl_flag);
extern void so_pr (char *so_desc, int so_x, int so_y, int so_flag);
extern void us_pr (char *us_desc, int us_x, int us_y, int us_flag);
extern void swide (void);
extern void snorm (void);
extern int getkey (void);
extern int run_mail (void);

/* term_slot.c */
extern int get_ttyslot (void);
extern int AddTerminalNo (int TerminalNo, major_t MajorDev, minor_t MinorDev);

/* time_day.c */
extern char *ttod (void);
extern long int atot (char *time_str);
extern char *ttoa (long int tm_val, char *mask);
extern long int rnd_time (long int raw_time, long int mult_of);

/* toggle_crsr.c */
extern int crsr_toggle (int stat);

/* tty_slot.c */
extern int ttyslt (void);

/* ttyctl.c */
extern void set_tty (void);
extern void rset_tty (void);

/* wild_card.c */
extern void wild_break (char *key_val, BIT_PTR bit_tab, int len);
extern int wild_check (char *s_ptr, BIT_PTR bit_ptr);
extern int is_wild (char *s_ptr);
extern char *wild_beg (char *s_ptr, BIT_PTR bit_ptr, int len);
extern char *wild_end (char *s_ptr, BIT_PTR bit_ptr, int len);

/* win_select.c */
#include	<win_select.h>
extern void win_display (char *, int, struct SEL_STR *, int, int, int, int);
extern struct SEL_STR *win_select (char *, int, struct SEL_STR *,int,int,int,int);

/* RF_ADD.c */
extern int RF_ADD (int fileno, char *buff);
extern int RF_UPDATE (int fileno, char *buff);

/* RF_CLOSE.c */
extern int RF_CLOSE (int _fileno);

/* RF_DELETE.c */
extern int RF_DELETE (int fileno);

/* RF_DISPLAY.c */
extern void RF_DISPLAY (int _fileno);

/* RF_OPEN.c */
extern int RF_OPEN (char *name, int size, char *mode, int *fd_ptr);

/* RF_READ.c */
extern int RF_READ (int _fileno, char *buff);

/* RF_REWIND.c */
extern int RF_REWIND (int _fileno);
extern int RF_SEEK (int fileno, int rec_no);

/* The BALANCE of these come from the converted INCLUDE files. */
/* account.c */
#include	<account.h>
extern int UserAccountOpen (char *fname, char *mode);
extern int UserAccountAdd (char *moption);

/* ring_menu.c */
#include	<ring_menu.h>
extern void _draw_menu (menu_type *curr_menu, char *prompt, int row, int old_page, int new_page, int old_opt, int new_opt);
extern int _init_menu (menu_type *curr_menu, char *prompt);
extern int _check_menu (menu_type *curr_menu, int opt, int x);
extern int _no_option (void);

/* ip_comms.c */
extern int IP_CREATE (int uniqid);
extern int IP_OPEN (int uniqid);
extern void IP_READ (int np_fn);
extern void IP_WRITE (int np_fn);
extern void IP_CLOSE (int np_fn);
extern void IP_UNLINK (int uniqid);

/* tcap.c */
extern int _mail_ok;

extern char *OpenSpecial (const char	*, const char *);	/* Passed SubDir/Filename		*/
											/* ("LAYOUT", "so_ctr_pac.fp") 	*/

extern	int		ErrorDelay		(void);	
extern	int		SleepDelay		(void);	
extern	int		MessageDelay	(void);	

extern void DbBalWin		(long, int, long);
extern int	UpdateWOStatus	(char *,char *,char *,char *,char *);
extern	void	XML_Error	(char *,char *,char *,char *,char *);

