/*
 *  Copyright (c) 2004 C. Boemann <cbo@boemann.dk>
 *            (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QtGlobal>
#include "kis_debug.h"
#include "kis_global.h"

//#define SHARED_TILES_SANITY_CHECK


template<class T>
KisTileHashTableTraits<T>::KisTileHashTableTraits(KisMementoManager *mm)
        : m_lock(QReadWriteLock::NonRecursive)
{
    m_hashTable = new TileTypeSP [TABLE_SIZE];
    Q_CHECK_PTR(m_hashTable);

    m_numTiles = 0;
    m_defaultTileData = 0;
    m_mementoManager = mm;
}

template<class T>
KisTileHashTableTraits<T>::KisTileHashTableTraits(const KisTileHashTableTraits<T> &ht,
        KisMementoManager *mm)
        : m_lock(QReadWriteLock::NonRecursive)
{
    QReadLocker locker(&ht.m_lock);

    m_mementoManager = mm;
    m_defaultTileData = 0;
    setDefaultTileDataImp(ht.m_defaultTileData);

    m_hashTable = new TileTypeSP [TABLE_SIZE];
    Q_CHECK_PTR(m_hashTable);


    TileTypeSP foreignTile;
    TileType* nativeTile;
    TileType* nativeTileHead;
    for (qint32 i = 0; i < TABLE_SIZE; i++) {
        nativeTileHead = 0;

        foreignTile = ht.m_hashTable[i];
        while (foreignTile) {
            nativeTile = new TileType(*foreignTile, m_mementoManager);
            nativeTile->setNext(nativeTileHead);
            nativeTileHead = nativeTile;

            foreignTile = foreignTile->next();
        }

        m_hashTable[i] = nativeTileHead;
    }
    m_numTiles = ht.m_numTiles;
}

template<class T>
KisTileHashTableTraits<T>::~KisTileHashTableTraits()
{
    clear();
    delete[] m_hashTable;
    setDefaultTileDataImp(0);
}

template<class T>
quint32 KisTileHashTableTraits<T>::calculateHash(qint32 col, qint32 row)
{
    return ((row << 5) + (col & 0x1F)) & 0x3FF;
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getTile(qint32 col, qint32 row)
{
    qint32 idx = calculateHash(col, row);
    TileTypeSP tile = m_hashTable[idx];

    for (; tile; tile = tile->next()) {
        if (tile->col() == col &&
                tile->row() == row) {

            return tile;
        }
    }

    return 0;
}

template<class T>
void KisTileHashTableTraits<T>::linkTile(TileTypeSP tile)
{
    qint32 idx = calculateHash(tile->col(), tile->row());
    TileTypeSP firstTile = m_hashTable[idx];

#ifdef SHARED_TILES_SANITY_CHECK
    Q_ASSERT_X(!tile->next(), "KisTileHashTableTraits<T>::linkTile",
               "A tile can't be shared by several hash tables, sorry.");
#endif

    tile->setNext(firstTile);
    m_hashTable[idx] = tile;
    m_numTiles++;
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::unlinkTile(qint32 col, qint32 row)
{
    qint32 idx = calculateHash(col, row);
    TileTypeSP tile = m_hashTable[idx];
    TileTypeSP prevTile = 0;

    for (; tile; tile = tile->next()) {
        if (tile->col() == col &&
                tile->row() == row) {

            if (prevTile)
                prevTile->setNext(tile->next());
            else
                /* optimize here*/
                m_hashTable[idx] = tile->next();

            /**
             * The shared pointer may still be accessed by someone, so
             * we need to disconnects the tile from memento manager
             * explicitly
             */
            tile->setNext(0);
            tile->notifyDead();
            tile = 0;

            m_numTiles--;
            return tile;
        }
        prevTile = tile;
    }

    return 0;
}

template<class T>
inline void KisTileHashTableTraits<T>::setDefaultTileDataImp(KisTileData *defaultTileData)
{
    if (m_defaultTileData) {
        m_defaultTileData->unblockSwapping();
        m_defaultTileData->release();
        m_defaultTileData = 0;
    }

    if (defaultTileData) {
        defaultTileData->acquire();
        defaultTileData->blockSwapping();
        m_defaultTileData = defaultTileData;
    }
}

template<class T>
inline KisTileData* KisTileHashTableTraits<T>::defaultTileDataImp() const
{
    return m_defaultTileData;
}


template<class T>
bool KisTileHashTableTraits<T>::tileExists(qint32 col, qint32 row)
{
    QReadLocker locker(&m_lock);
    return getTile(col, row);
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getExistedTile(qint32 col, qint32 row)
{
    QReadLocker locker(&m_lock);
    return getTile(col, row);
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getTileLazy(qint32 col, qint32 row,
                                       bool& newTile)
{
    /**
     * FIXME: Read access is better
     */
    QWriteLocker locker(&m_lock);

    newTile = false;
    TileTypeSP tile = getTile(col, row);
    if (!tile) {
        tile = new TileType(col, row, m_defaultTileData, m_mementoManager);
        linkTile(tile);
        newTile = true;
    }

    return tile;
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getReadOnlyTileLazy(qint32 col, qint32 row)
{
    QReadLocker locker(&m_lock);

    TileTypeSP tile = getTile(col, row);
    if (!tile)
        tile = new TileType(col, row, m_defaultTileData, 0);

    return tile;
}

template<class T>
void KisTileHashTableTraits<T>::addTile(TileTypeSP tile)
{
    QWriteLocker locker(&m_lock);
    linkTile(tile);
}

template<class T>
void KisTileHashTableTraits<T>::deleteTile(qint32 col, qint32 row)
{
    QWriteLocker locker(&m_lock);

    TileTypeSP tile = unlinkTile(col, row);

    /* Done by KisSharedPtr */
    //if(tile)
    //    delete tile;

}

template<class T>
void KisTileHashTableTraits<T>::deleteTile(TileTypeSP tile)
{
    deleteTile(tile->col(), tile->row());
}

template<class T>
void KisTileHashTableTraits<T>::clear()
{
    QWriteLocker locker(&m_lock);
    TileTypeSP tile = 0;
    qint32 i;

    for (i = 0; i < TABLE_SIZE; i++) {
        tile = m_hashTable[i];

        while (tile) {
            TileTypeSP tmp = tile;
            tile = tile->next();

            /**
             * About disconnection of tiles see a comment in unlinkTile()
             */

            tmp->setNext(0);
            tmp->notifyDead();
            tmp = 0;

            m_numTiles--;
        }

        m_hashTable[i] = 0;
    }

    Q_ASSERT(!m_numTiles);
}

template<class T>
void KisTileHashTableTraits<T>::setDefaultTileData(KisTileData *defaultTileData)
{
    QWriteLocker locker(&m_lock);
    setDefaultTileDataImp(defaultTileData);
}

template<class T>
KisTileData* KisTileHashTableTraits<T>::defaultTileData() const
{
    QWriteLocker locker(&m_lock);
    return defaultTileDataImp();
}


/*************** Debugging stuff ***************/

template<class T>
void KisTileHashTableTraits<T>::debugPrintInfo()
{
    dbgTiles << "==========================\n"
             << "TileHashTable:"
             << "\n   def. data:\t\t" << m_defaultTileData
             << "\n   numTiles:\t\t" << m_numTiles;
    debugListLengthDistibution();
    dbgTiles << "==========================\n";
}

template<class T>
qint32 KisTileHashTableTraits<T>::debugChainLen(qint32 idx)
{
    qint32 len = 0;
    for (TileTypeSP it = m_hashTable[idx]; it; it = it->next(), len++) ;
    return len;
}

template<class T>
void KisTileHashTableTraits<T>::debugMaxListLength(qint32 &min, qint32 &max)
{
    TileTypeSP tile;
    qint32 maxLen = 0;
    qint32 minLen = m_numTiles;
    qint32 tmp = 0;

    for (qint32 i = 0; i < TABLE_SIZE; i++) {
        tmp = debugChainLen(i);
        if (tmp > maxLen)
            maxLen = tmp;
        if (tmp < minLen)
            minLen = tmp;
    }

    min = minLen;
    max = maxLen;
}

template<class T>
void KisTileHashTableTraits<T>::debugListLengthDistibution()
{
    qint32 min, max;
    qint32 arraySize;
    qint32 tmp;

    debugMaxListLength(min, max);
    arraySize = max - min + 1;

    qint32 *array = new qint32[arraySize];
    memset(array, 0, sizeof(qint32)*arraySize);

    for (qint32 i = 0; i < TABLE_SIZE; i++) {
        tmp = debugChainLen(i);
        array[tmp-min]++;
    }

    dbgTiles << QString("   minChain:\t\t%d\n"
                        "   maxChain:\t\t%d").arg(min, max);

    dbgTiles << "   Chain size distribution:";
    for (qint32 i = 0; i < arraySize; i++)
        dbgTiles << QString("      %1:\t%2\n").arg(i + min, array[i]);

    delete[] array;
}

template<class T>
void KisTileHashTableTraits<T>::sanityChecksumCheck()
{
    /**
     * We assume that the lock should have already been taken
     * by the code that was going to change the table
     */
    Q_ASSERT(!m_lock.tryLockForWrite());

    TileTypeSP tile = 0;
    qint32 exactNumTiles = 0;

    for (qint32 i = 0; i < TABLE_SIZE; i++) {
        tile = m_hashTable[i];
        while (tile) {
            exactNumTiles++;
            tile = tile->next();
        }
    }

    if (exactNumTiles != m_numTiles) {
        qDebug() << "Sanity check failed!";
        qDebug() << ppVar(exactNumTiles);
        qDebug() << ppVar(m_numTiles);
        qDebug() << "Wrong tiles checksum!";
        Q_ASSERT(0); // not qFatal() for a backtrace support
    }
}
