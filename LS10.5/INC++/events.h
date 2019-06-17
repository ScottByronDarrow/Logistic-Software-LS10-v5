#ifndef	_events_h
#define	_events_h
/*	$Id: events.h,v 5.0 2002/05/08 01:50:44 scott Exp $
 *
 *	List of known events
 *
 *	$Log: events.h,v $
 *	Revision 5.0  2002/05/08 01:50:44  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:11  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:06:43  scott
 *	Updated to clean code while working in format-p
 *	
 *	Revision 3.0  2000/10/12 13:39:02  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 1.1.1.1  1999/07/14 23:58:50  jonc
 *	Initial C++ sources (adopted minimally from V10)
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:27  jonc
 *	Version 10 start
 *
 *	Revision 2.7  1996/11/07 02:40:16  jonc
 *	Added EvtStart
 *
 *	Revision 2.6  1996/06/30 21:15:35  jonc
 *	Removed unused Event.
 *
 *	Revision 2.5  1996/05/29 23:02:38  jonc
 *	Added EVT_Message
 *
 *	Revision 2.4  1996/05/20 00:44:01  jonc
 *	Added InsertSet & DeleteSet
 *
 *	Revision 2.3  1996/05/13 23:10:09  jonc
 *	Added EVT_Debug, EVT_SetFocus
 *
 *	Revision 2.2  1996/03/26 02:40:19  jonc
 *	Support added for clearing out Events to deleted objects on the EventQueue
 *
 *	Revision 2.1  1996/03/21 23:49:33  jonc
 *	Renamed EVT_[Next|Prev]Set -> EVT_[Next|Prev]Page
 *	Semantics for EVT_[Next|Prev]Set changed
 *
 *	Revision 2.0  1996/02/13 03:45:04  jonc
 *	Updated to 2.0
 *
 *	Revision 1.1  1996/02/13 03:44:29  jonc
 *	Initial cut
 *
 *	Revision 1.1  1996/01/07 21:08:48  jonc
 *	Initial revision
 *
 */
enum EventType
{
	/*
	 *	Application Level Events
	 */
	EVT_Bad,				// internal error
	EVT_Debug,				// debug on/off request (internal use only?)
	EVT_Dead,				// event to non-existent object

	EVT_Message,			// simple message event

	EVT_Start,				// first message to be sent
	EVT_Quit,				// request to quit
	EVT_Abort,				// Clear
	EVT_Accept,				// Commit
	EVT_Redraw,				// clear screen and redraw
	EVT_Help,				// invoke context sensitive help
	EVT_Refresh,			// refresh screen (render displays)

	EVT_WideScreen,			// request to wide mode
	EVT_NormalScreen,		// request to normal mode

	/*
	 *	Window Level Events
	 */
	EVT_Next,				// next item in sequence
	EVT_Prev,				// previous item in sequence
	EVT_NextSet,			// prev set of fields (only for multi-field page)
	EVT_PrevSet,			// next set of fields (only for multi-field page)
	EVT_NextPage,			// prev page of fields (only for multi-field page)
	EVT_PrevPage,			// next page of fields (only for multi-field page)

	EVT_InsertSet,			// insert new set (only for multi-field page)
	EVT_DeleteSet,			// delete current set (only for multi-field page)

	/*
	 *	Field Level Events
	 */
	EVT_Char,				// common character received
	EVT_ListVal,			// list possible values

	EVT_Display,			// display

	EVT_SetFocus,			// request to change focus (internal use only?)
	EVT_GetFocus,			// just got input focus
	EVT_LoseFocus,			// lost input focus

	EVT_Last				// for internal use only - *DON'T* *USE*
};

#endif	//_events_h
