#ifndef YZ_EVENTS_H
#define YZ_EVENTS_H
/**
 * yz_event.h
 *
 * yzis events
 *
 */


#include "yzis.h" /* NULL */


/** list of all events */
enum yz_events {
	YZ_EV_SETLINE,
	YZ_EV_SETCURSOR,
	YZ_EV_SETSTATUS
};

/** Here are some struct used for event args handling
  * we can use long name as we shouldn't need to use those anyway
  */
class YZLine;


/**
  * Be careful, when the core uses yz_event_setline, it is of its responsability
  * to handle the cursor position
  * for a whole screen refresh
  * 	yz_event_setline for line 1
  * 	....
  * 	yz_event_setline for line n
  *
  * 	yz_event_setcursor(wherever)
  *
  * for a modified line
  * 	yz_event_setline for line y
  * 	yz_event_setcursor(wherever)
  *
  * There will be some special events for that, but not yet
  */
struct yz_event_setline {
	int	y;
	YZLine 	*line;
};

/**
  * x and y are relative to the view
  */
struct yz_event_setcursor {
	int x,y;
};

struct yz_event_setstatus {
	char *text;
};




/**
  * The yz_event struct, mainly an union of all event args
  *
  * exemple of use : 
  * (e is of type yz_event)
  * switch (e.id) {
	 case YZ_EV_SETLINE:
		here we use e.u.setline.line_nb;
		here we use e.u.setline.line;
		break;

	case YZ_EV_SETCURSOR:
   }

  */
 struct yz_event_t {
	enum yz_events		id;
	struct yz_event_t	*next;
	union {
		struct yz_event_setline		setline;
		struct yz_event_setcursor	setcursor;
		struct yz_event_setstatus	setstatus;
	} u;
};

typedef struct yz_event_t yz_event;


yz_event mk_event_setstatus(char *text);
yz_event mk_event_setcursor(int x, int y);


/** Event pool
  */

class EventPool {
public:
	EventPool(void) { nb=0;}

	yz_event *getone(void) { pool[nb].next=NULL; nb++; return &pool[nb-1];}


protected:
	/** ok, it's really basic now :) */
	yz_event	pool[4000];
	int		nb;
};

#endif /*  YZ_EVENTS_H */

