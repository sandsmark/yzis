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

#ifndef YZ_SWAPFILE
#define YZ_SWAPFILE

#include "undo.h"

class YBuffer;

/**
 * Creates a swapfile on a buffer
 */
class YSwapFile
{
public:
    /**
     * Default constructor
     */
    YSwapFile(YBuffer *b);

    /**
     * Add an inputs event to history
     */
    void addToSwap(YBufferOperation::OperationType type, const YRawData &data, const YInterval &interval);

    /**
     * Clear the history
     */
    void reset()
    {
        mHistory.clear();
    }

    /**
     * Writes the swap to the file
     */
    void flush();

    /**
     * Changes the swap filename
     */
    void setFileName(const QString &filename);
    inline const QString &filename() const
    {
        return mFilename;
    }

    /**
     * Deletes the swapfile
     */
    void unlink();

    /**
     * Initialise a swap file
     */
    void init();

    /**
     * Recover a buffer from a swap file
     */
    bool recover();

protected:
    /**
     * Replay one event on the buffer during a recover
     */
    void replay(YBufferOperation::OperationType type, const YInterval &interval, const YRawData &data);

private:
    struct swapEntry
    {
        YBufferOperation::OperationType type;
        YInterval interval;
        YRawData data;
    } sE;

    QList<swapEntry> mHistory;
    YBuffer *mParent;
    QString mFilename;
    bool mRecovering;
    bool mNotResetted;
};

#endif
