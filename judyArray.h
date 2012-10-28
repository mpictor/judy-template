#ifndef JUDYARRAY_CPP_H
#define JUDYARRAY_CPP_H

/****************************************************************************//**
* \file judyArray.h C++ wrapper for judy array implementation
*
*    A judy array maps a set of strings to corresponding memory cells.
*    Each cell must be set to a non-zero value by the caller.
*
*    (C)opyright 2012, Mark Pictor. License: 3-clause BSD
*
********************************************************************************/

#include "judy64k.h"
#include "assert.h"
#include <string.h>

struct judyKVpair {
    unsigned char * key;
    unsigned long long value;
};

class judyArray {
protected:
    Judy * _judyarray;
    unsigned int _maxKeyLen;
    JudySlot * _lastSlot;
    unsigned char *_buff;
public:
    judyArray( unsigned int maxKeyLen ): _maxKeyLen( maxKeyLen ) {
        _judyarray = judy_open( _maxKeyLen ); //TODO if max key len is 8, treat as integer???
        _buff = new char[_maxKeyLen];
        assert( JUDY_cache_line >= 8 );
    }

    ~judyArray() {
        judy_close( _judyarray );
        delete[] _buff;
    }

    unsigned long long getLastValue() {
        return &_lastSlot;
    }

    void setLastValue( unsigned long long value ) {
        &_lastSlot = value;
    }

    explicit judyArray( const judyArray & other ): _maxKeyLen( other._maxKeyLen ) {
        _judyarray = judy_clone( other._judyarray );
        _buff = new char[_maxKeyLen];
        strncpy( _buff, other._buff, _maxKeyLen );
        _buff[ _maxKeyLen ] = '\0'; //ensure that _buff is null-terminated, since strncpy won't necessarily do so
        find( _buff ); //set _lastSlot
    }

// allocate data memory within judy array for external use.
// void *judy_data (Judy *judy, unsigned int amt);

    //can this overwrite?
    insert( unsigned char * key, unsigned long long value, unsigned int keyLen = 0 ) {
        assert( value != 0 );
        assert( keyLen <= _maxKeyLen );
        if( keyLen == 0 ) {
            keyLen = _maxKeyLen;
        }
        _lastSlot = judy_cell( _judyarray, key, keyLen );
        &_lastSlot = value;
    }

    /// retrieve the cell pointer greater than or equal to given key
    /// NOTE what about an atOrBefore function?
    const judyKVpair & atOrAfter( unsigned char * key, unsigned int keyLen = 0 ) {
        assert( keyLen <= _maxKeyLen );
        if( keyLen == 0 ) {
            keyLen = _maxKeyLen;
        }
        _lastSlot = judy_strt( _judyarray, key, keyLen );
        return mostRecentPair();
    }

    /// retrieve the cell pointer, or return NULL for a given key.
    unsigned long long find( unsigned char * key, unsigned int keyLen = 0 ) {
        assert( keyLen <= _maxKeyLen );
        if( keyLen == 0 ) {
            keyLen = _maxKeyLen;
        }
        _lastSlot = judy_slot( _judyarray, key, keyLen );
    }

    /// retrieve the key-value pair for the most recent judy query.
    inline const judyKVpair & mostRecentPair() {
        judyKVpair kv;
        judy_key( _judyarray, _buff, _maxKeyLen );
        kv.value = &_lastSlot;
        kv.key = _buff;
        return & kv;
    }

    /// retrieve the last key-value pair in the array
    const judyKVpair & end() {
        _lastSlot = judy_end( _judyarray );
        return mostRecentPair();
    }

    /// retrieve the key-value pair for the next string in the array.
    const judyKVpair & next() {
        _lastSlot = judy_nxt( _judyarray );
        return mostRecentPair();
    }

    /// retrieve the key-value pair for the prev string in the array.
    const judyKVpair & previous() {
        _lastSlot = judy_prv( _judyarray );
        return mostRecentPair();
    }

    /// delete the key and cell for the current stack entry.
    void removeEntry() {
        judy_del ( _judyarray );
    }
};
#endif //JUDYARRAY_CPP_H
