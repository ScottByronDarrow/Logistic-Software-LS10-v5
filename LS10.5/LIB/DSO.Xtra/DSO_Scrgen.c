// SEVERELY stripped down pslscr.h
#define	LIN	0
#define	TRUE	1
#define	FALSE	0

#ifndef	MAXSCNS
#define	MAXSCNS	5
#endif

#ifndef	MAXLINES
#define	MAXLINES	50
#endif

#ifndef	MAXWIDTH
#define	MAXWIDTH	200
#endif

struct	tab_struc
{
	char	*_desc;
	int	_width;
	int	_scn;
	int	_row;
	int	(* _actn)(void);
	int	_win;
};

struct	tab_struc tab_data [MAXSCNS];
int	delta_edit;
int	in_sub_edit = FALSE;
char	temp_mask [MAXWIDTH],
	prv_ntry [MAXWIDTH],
	restart_msg [132];
char	*lstore [MAXLINES * MAXSCNS];
char	*arg [20];
int	lcount [MAXSCNS + 1],
        tab_index [MAXWIDTH],
        tab_row = 6,
        tab_col = 0,
        input_row = 3,
        input_col = 30,
        dec_pt = 0,
        row = 0,
        col = 0,
        max_prompt = 0,
        nbr_fields = 0,
        scn_start = 0,
        prog_status,
        edit_status = 0,
        cur_screen = 0,
        cur_field = 0,
        scn_page = 0,
        line_cnt = 0,
        sr_lcnt = 0,
        scn_type = LIN,
        tabscn = 0,
        msg_line = 23,
        comment_line = 23,
        error_line = 23,
        entry_exit = 0,
        edit_exit = 0,
        end_input = 0,
        skip_entry = 0,
        skip_tab = 1,
        new_rec = 0,
        length,
        eoi_ok = 1,
        init_ok = 1,
        sr_err = 0,
        start_lin = 0,
	testchar = 0,
	restart_ck = 0,
	pslw_asl = FALSE,
	up_ok = 1,
	SYS_LANG = 0,
	Dsp_prn_ok = FALSE;
