#include "kyzisview.h"
#include "kyzis_factory.h"
#include <qlayout.h>
#include <qevent.h>
#include <qapplication.h>
#include <kdebug.h>

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name )
	: KTextEditor::View (doc, parent, name),
		YZView(doc, 1) {
	last_event_done=0;
	editor = new KYZisEdit (this,"editor");
	status = new KStatusBar (this, "status");
	status->insertItem("Yzis Ready",0);
	
	QVBoxLayout *l = new QVBoxLayout(this);
	l->addWidget(editor);
	l->addWidget(status);

	register_manager(this);

	buffer = doc;
	editor->show();
	status->show();
}

KYZisView::~KYZisView () {
	delete editor;
	delete status;
}

void KYZisView::postEvent(yz_event ev) {
	kdDebug() << "postEvent " << events_nb_last <<endl;
	QCustomEvent *myev = new QCustomEvent (QEvent::User+events_nb_last);//so that we know what kind of event it is, may be useless ...
	QApplication::postEvent( this, myev ); //this gives Qt the priority before processing our own events
}

//receives previously generated events from Qt event loop. hopefully it will do
//what I want :)
void KYZisView::customEvent (QCustomEvent *) {
	yz_event *event/*=fetch_event()*/;
	while (last_event_done < events_nb_last) {
		event = fetch_event(last_event_done++);
		kdDebug() << "** Processing Event " << last_event_done << " Id : " << event->id << endl;
//	while ( ( event= fetch_event() ) != NULL) {
		if (! event ) {
			kdDebug() << "OUPS, no event to fetch !" << endl;
			return;
		}
		switch ( event->id ) {
			case YZ_EV_SETLINE:
				kdDebug() << "event SETLINE" << endl;
//				editor->setText( *(event->u.setline.line) );
				break;
			case YZ_EV_SETCURSOR:
				kdDebug() << "event SETCURSOR" << endl;
				break;
			case YZ_EV_SETSTATUS:
				kdDebug() << "event SETSTATUS" << event->u.setstatus.text <<  endl;
				status->changeItem( event->u.setstatus.text,0 );
				break;
		}
 }
}

#include "kyzisview.moc"
