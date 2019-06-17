/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( graph.h        )                                 |
|  Program Desc  : ( Include file for graphics use.               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Trevor van Bremen    Date Written  : 29/06/90      |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified by :                    |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
=====================================================================*/

#ifndef	GRAPH
#define	GRAPH

#define	GR_TYPE_SBAR	0x0000		/* Stacked bar graph		*/
#define	GR_TYPE_BAR	0x0001		/* Standard bar graph		*/
#define	GR_TYPE_DBAR	0x0002		/* Standard double bar graph	*/
#define	GR_TYPE_1BAR	0x0001		/* Standard bar graph		*/
#define	GR_TYPE_2BAR	0x0002		/* Standard double bar graph	*/
#define	GR_TYPE_3BAR	0x0003		/* Standard triple bar graph	*/
#define	GR_TYPE_4BAR	0x0004		/* Standard quadruple bar graph	*/
#define	GR_TYPE_5BAR	0x0005		/* Standard pentuple bar graph	*/
#define	GR_TYPE_SROW	0x0010		/* Stacked col graph		*/
#define	GR_TYPE_ROW	0x0011		/* Standard col graph		*/
#define	GR_TYPE_DROW	0x0012		/* Standard double col graph	*/
#define	GR_TYPE_1ROW	0x0011		/* Standard col graph		*/
#define	GR_TYPE_2ROW	0x0012		/* Standard double col graph	*/
#define	GR_TYPE_3ROW	0x0013		/* Standard triple col graph	*/
#define	GR_TYPE_4ROW	0x0014		/* Standard quadruple col graph	*/
#define	GR_TYPE_5ROW	0x0015		/* Standard pentuple col graph	*/
#define	GR_TYPE_LINE	0x0020		/* Standard line graph		*/
#define	GR_TYPE_PIE	0x0030		/* Standard pie chart		*/
	/* NB: Currently only types 0x0001 to 0x0005 are implemented	*/

struct	GR_WINDOW
{
	int	x_posn;			/* Offset from left of screen	*/
	int	y_posn;			/* Offset from top of screen	*/
	int	x_size;			/* Width of graph (Characters)	*/
	int	y_size;			/* Height of graph (Lines)	*/
};

struct	GR_NAMES
{
	char	*pr_head;		/* Heading line for Printing	*/
	char	*heading;		/* Title of graph		*/
	char	**legends;		/* Array of legends		*/
	char	*gpx_ch_indx;		/* Indexed map to gpx chrs used	*/
};

struct	GR_LIMITS
{
	float	min_y;			/* Minimum graphable y value	*/
	float	max_y;			/* Maximum graphable y value	*/
};
/* NB:	The GR_LIMITS is passed as a structure pointer.
	If the structure pointer is NULL then the values will be
	automatically determined from within the graph function		*/

#define	EGR_NOT_IMP	-1		/* Not currently implemented	*/
#define	EGR_WIDTH	-2		/* Insufficient width to graph	*/
#define	EGR_HEIGHT	-3		/* Insufficient height to graph	*/

#endif	/* GRAPH	*/
