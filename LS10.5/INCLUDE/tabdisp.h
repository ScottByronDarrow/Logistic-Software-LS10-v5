#ifndef	ITABDISP

#define	TAB_NAME	work_tab->f_name
#define	TAB_BUF		work_tab->line_tab
#define	TAB_PAGE	work_tab->page_off
#define	TAB_ID		work_tab->f_id
#define	TAB_PSIZE	work_tab->page_size
#define	TAB_LPAGENO	work_tab->last_page
#define	TAB_PGENO	work_tab->page
#define	TAB_LNENO	work_tab->cur_line
#define	TAB_LINES	work_tab->lines
#define	TAB_ACTIVE	work_tab->active
#define	TAB_ROW		work_tab->row
#define	TAB_COL		work_tab->col
#define	TAB_SRCH	work_tab->srch
#define	TAB_SWIDTH	work_tab->s_width
#define	TAB_FWIDTH	work_tab->f_width
#define	TAB_DEPTH	work_tab->depth
#define	TAB_MEMORY	work_tab->mem_flag
#define	TAB_KEYS	work_tab->disp_keys
#define	HDR_LINE	work_tab->hdr_buf

#define	TAB_DETAIL(x)	TAB_BUF + (x * (TAB_FWIDTH + 2))
#define	DET_LINE	TAB_DETAIL (TAB_LNENO)
#define	SCR_ROW		(TAB_ROW + TAB_LNENO + 3)
#define	SCR_WIDTH	TAB_SWIDTH, TAB_SWIDTH
#define	FILE_WIDTH	TAB_FWIDTH, TAB_FWIDTH

#define	MAX_LINES	(TAB_LINES - 1)

/*
.structure
	Structure	:	TAB_STRUCT

	Description	:	Structure used to hold table information.

	Elements	:	f_name    - Pointer to string holding name.
				hdr_buf   - Pointer to table header.
				line_tab  - Pointer to table screeen buffer.
				page_off  - Pointer to table of page offsets.
				f_id      - Pointer to FILE descriptor.
				page_size - Number of lines in current page.
				last_page - Page number of last page displayed.
				page      - Page number of current page.
				row       - Start row of table.
				col       - Start column of table.
				srch      - True iff search table.
				s_width   - Table width on screen.
				f_width   - Table width on disc.
				depth     - Table depth.
				cur_line  - Current line in page.
				lines     - Number of lines in entire table.
				active    - if TRUE table is being scanned.
				mem_flag  - If TRUE table is memory resident.
				disp_keys - Hot-keys.

.end
*/
typedef	struct {
	char	*f_name;		/* file name			*/
	char	*line_tab;		/* 				*/
	char	*hdr_buf;		/* 				*/
	long	*page_off;		/* 				*/
	FILE	*f_id;			/* 				*/
	int	page_size;		/* 				*/
	int	last_page;		/* 				*/
	int	page;			/* 				*/
	int	row;			/* 				*/
	int	col;			/* 				*/
	int	srch;			/* 				*/
	int	s_width;		/* 				*/
	int	f_width;		/* 				*/
	int	depth;			/* 				*/
	int	cur_line;		/* 				*/
	int	lines;			/* 				*/
	int	active;			/* 				*/
	int	mem_flag;		/* 				*/
	KEY_PTR disp_keys;		/* 				*/
} *TAB_PTR, TAB_STRUCT;

#define	ITABDISP
#endif
