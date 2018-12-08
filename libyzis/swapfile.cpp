/* This file is part of the Yzis libraries
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

/* Yzis */
#include "swapfile.h"
#include "debug.h"
#include "yzis.h"
#include "internal_options.h"
#include "buffer.h"
#include "action.h"
#include "view.h"
#include "session.h"
#include "portability.h"

/* Qt */
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTime>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef YZIS_WIN32_MSVC
#include <io.h>
#endif

#define dbg() yzDebug("YSwapFile")
#define err() yzError("YSwapFile")

YSwapFile::YSwapFile(YBuffer *b)
{
    mParent = b;
    mRecovering = false;
    mFilename = QString();
    setFileName(b->fileName());
    mNotResetted = true;
    //init();
}

void YSwapFile::setFileName(const QString &fname)
{
    dbg() << "setFileName( " << fname << ")" << endl;
    unlink();
    mFilename = fname.section('/', 0, -2) + "/." + fname.section('/', -1) + ".ywp";
    dbg() << "Swap filename = " << mFilename << endl;
}

void YSwapFile::flush()
{
    if (mRecovering) {
        return;
    }

    if (mParent->getLocalIntegerOption("updatecount") == 0) {
        return;
    }

    if (mNotResetted) {
        init();
    }

    dbg() << "Flushing swap to " << mFilename << endl;
    QFile f(mFilename);
    bool proceed;
#ifndef YZIS_WIN32
    struct stat buf;
    int i = lstat(mFilename.toLocal8Bit(), &buf);
    proceed = (i != -1 && S_ISREG(buf.st_mode) && !S_ISLNK(buf.st_mode) && buf.st_uid == geteuid());
#else
    proceed = true;
#endif

    if (proceed && f.open(QIODevice::WriteOnly | QIODevice::Append)) { //open at end of file
#ifndef YZIS_WIN32
        chmod(mFilename.toLocal8Bit(), S_IRUSR | S_IWUSR);
#endif
        QTextStream stream(&f);

        if (!mHistory.empty()) {
            for (int ab = 0; ab < mHistory.size(); ++ab) {
                const swapEntry *e = &mHistory.at(ab);
#define DUMP_BOUND(bound) \
    (bound).pos().x() << " " << (bound).pos().y() << " " << ((bound).closed() ? 1 : 0)
                stream << e->type << " " << DUMP_BOUND(e->interval.from()) << " " << DUMP_BOUND(e->interval.to()) << endl;

                for (int i = 0; i < e->data.count(); ++i) {
                    stream << " " << e->data.at(i) << endl;
                }

                stream << endl;
            }
        }

        f.close();
    } else {
        YSession::self()->guiPopupMessage(_("Warning, the swapfile could not be opened maybe due to restrictive permissions."));
        mNotResetted = true; //don't try again ...
    }

    mHistory.clear(); //clear previous history
}

void YSwapFile::addToSwap(YBufferOperation::OperationType type, const YRawData &data, const YInterval &interval)
{
    if (mRecovering) {
        return;
    }

    if (mParent->getLocalIntegerOption("updatecount") == 0) {
        return;
    }

    swapEntry e;
    e.type = type;
    e.interval = interval;
    e.data = data;
    mHistory.append(e);

    if (((int)mHistory.size()) >= mParent->getLocalIntegerOption("updatecount")) {
        flush();
    }
}

void YSwapFile::unlink()
{
    dbg() << "Unlink swap file " << mFilename << endl;

    if (!mFilename.isNull() && QFile::exists(mFilename)) {
        QFile::remove(mFilename);
    }

    mNotResetted = true;
}

void YSwapFile::init()
{
    dbg() << "init() mFilename=" << mFilename << endl;

    if (QFile::exists(mFilename)) {
        dbg() << "Swap file already EXISTS ! " << endl;
        //that should really not happen ...
        mNotResetted = true; //don't try to access that file later ...
        return;
    }

    QFile f(mFilename);

    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
#ifdef YZIS_WIN32_MSVC
        _chmod(mFilename.toLocal8Bit(), S_IRUSR | S_IWUSR);
#else
        chmod(mFilename.toLocal8Bit(), S_IRUSR | S_IWUSR);
#endif
        QTextStream stream(&f);
        stream << "WARNING : do not edit, this file is a temporary file created by Yzis and used to recover files in case of crashes" << endl
               << endl;
        stream << "Generated by Yzis " << VERSION_CHAR << endl;
        stream << "Edited file: " << mParent->fileName() << endl;
        stream << "Creation date: " << QDateTime::currentDateTime().toString() << endl;
        // XXX stream << "Process ID: " << QString::number( getpid() ) << endl;
        stream << endl
               << endl
               << endl;
        f.close();
    } else {
        YSession::self()->guiPopupMessage(_("Warning, the swapfile could not be created maybe due to restrictive permissions."));
        mNotResetted = true;
        return;
    }

    mNotResetted = false;
}

bool YSwapFile::recover()
{
    mRecovering = true;
    QFile f(mFilename);

    if (f.open(QIODevice::ReadOnly)) {
        QTextStream stream(&f);

        while (!stream.atEnd()) {
            QString line = stream.readLine();
            QRegExp rx("([0-9]) ([0-9]*) ([0-9]*) ([01]) ([0-9]*) ([0-9]*) ([01])");
            bool error = !stream.atEnd();

            if (!error && rx.exactMatch(line)) {
                YBufferOperation::OperationType type = (YBufferOperation::OperationType)rx.cap(1).toInt();
                YBound from = YBound(YCursor(rx.cap(2).toInt(), rx.cap(3).toInt()), rx.cap(4) == "0");
                YBound to = YBound(YCursor(rx.cap(5).toInt(), rx.cap(6).toInt()), rx.cap(7) == "0");
                YRawData data;
                line = stream.readLine();

                while (!error && line.length() > 0) {
                    if (stream.atEnd()) {
                        error = true;
                    } else {
                        data << line.mid(1);
                        line = stream.readLine();
                    }
                }

                if (!error) {
                    replay(type, YInterval(from, to), data);
                }
            } else {
                error = true;
            }

            if (error) {
                dbg() << "Error reading swap file" << endl;
            }
        }

        f.close();
    } else {
        YSession::self()->guiPopupMessage(_("The swap file could not be opened, there will be no recovering for this file, you might want to check permissions of files."));
        mRecovering = false;
        return false;
    }

    mRecovering = false;
    return true;
}

void YSwapFile::replay(YBufferOperation::OperationType type, const YInterval &interval, const YRawData &data)
{
    switch (type) {
    case YBufferOperation::OpAddRegion:
        mParent->insertRegion(interval.fromPos(), data);
        break;

    case YBufferOperation::OpDelRegion:
        mParent->deleteRegion(interval);
        break;
    }
}
