/**
 * $Id$
 */
#ifndef NYZ_VIEW_H
#define NYZ_VIEW_H
/**
 * ncurses-based GUI for yzis
 */

#include "yz_view.h"
#include "gui.h"
#include <curses.h>
#include <qstringlist.h>
#include <qmap.h>

class NYZSession;

class NYZView : public YZView, public Gui  {
public:
	/**
	  * constructor. Each view is binded to a buffer, @param lines is the initial number of lines that
	  * this view can display
	  */
	NYZView(NYZSession *_session, WINDOW *_window, YZBuffer *b);
	virtual ~NYZView();

	void event_loop();
	void flush_events();

	/* Gui */
	virtual void postEvent(yz_event);
	virtual YZSession *getCurrentSession(void);

	virtual void scrollDown(int lines=1);
	virtual void scrollUp(int lines=1);

	virtual QString getCommandLineText(void) const;
	virtual void setCommandLineText( const QString& );
	virtual void setFocusCommandLine() {}
	virtual void setFocusMainWindow() {}



protected:
	void handle_event(yz_event e);

	WINDOW		*window;	/* ncurses window to write to */
	NYZSession	*session;

	unsigned int	h, w;		/** height and width of the window */

private:
	void update_info(void) { getmaxyx(window, h, w); lines_vis = h; }
	/**
	  * Display a line
	  * @arg line is the line number, taken from the beginning of the file ( and not
	  * the beginning of the displayed part )
	  */
	void printLine( int line );
	/**
	  * print a void line ( that is "~    " )
	  * @arg line is the line # relative to the displayed screen, not the buffer ( as this
	  * doesn't belong to the buffer anyway..)
	  */
	void printVoid( int line );

	QString commandline;

};

#endif // NYZ_VIEW_H
