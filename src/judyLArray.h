#ifndef JUDYLARRAY_CPP_H
#define JUDYLARRAY_CPP_H

/****************************************************************************//**
* \file judyLArray.h C++ wrapper for judyL array implementation
*
*    A judyL array maps a set of ints to corresponding memory cells.
*    Each cell must be set to a non-zero value by the caller.
*
*    Author: Mark Pictor. Public domain.
*
********************************************************************************/

#include "judy.h"
#include "assert.h"
#include <string.h>

template< typename JudyKey, typename JudyValue >
struct judylKVpair {
    JudyKey key;
    JudyValue value;
};

/** The following template parameters must be the same size as a void*
 *  \param JudyKey the type of the key, i.e. uint64_t, pointer-to-object, etc
 *  \param JudyValue the type of the value
 */
template< typename JudyKey, typename JudyValue >
class judyLArray {
    protected:
        Judy * _judyarray;
        unsigned int _maxKeyLen, _depth;
        JudyValue * _lastSlot;
        unsigned char * _buff; //TODO change type for judyL?
        bool _success;
    public:
        typedef judylKVpair< JudyKey, JudyValue > pair;
        judyLArray(): _maxKeyLen( 2 ^ ( sizeof( JudyKey ) ) ), _depth( sizeof( JudyKey ) ), _success( true ) {
            _judyarray = judy_open( _maxKeyLen, _depth );
            _buff = new unsigned char[_maxKeyLen];
            assert( sizeof( JudyValue ) == sizeof( this ) && "JudyValue *must* be the same size as a pointer!" );
        }

        explicit judyLArray( const judyLArray< JudyKey, JudyValue > & other ): _maxKeyLen( other._maxKeyLen ),
            _depth( other._depth ), _success( other._success ) {
            _judyarray = judy_clone( other._judyarray );
            _buff = new unsigned char[_maxKeyLen];
            strncpy( _buff, other._buff, _maxKeyLen );
            _buff[ _maxKeyLen ] = '\0'; //ensure that _buff is null-terminated, since strncpy won't necessarily do so
            find( _buff ); //set _lastSlot
        }

        ~judyLArray() {
            judy_close( _judyarray );
            delete[] _buff;
        }

        JudyValue getLastValue() {
            return &_lastSlot;
        }

        void setLastValue( JudyValue value ) {
            &_lastSlot = value;
        }

        bool success() {
            return _success;
        }
        //TODO
        // allocate data memory within judy array for external use.
        // void *judy_data (Judy *judy, unsigned int amt);

        //can this overwrite?
        void insert( JudyKey key, JudyValue value ) {
            assert( value != 0 );
            _lastSlot = ( JudyValue * ) judy_cell( _judyarray, ( const unsigned char * ) &key, 0 );
            if( _lastSlot ) {
                *_lastSlot = value;
                _success = true;
            } else {
                _success = false;
            }
        }

        /// retrieve the cell pointer greater than or equal to given key
        /// NOTE what about an atOrBefore function?
        const pair atOrAfter( JudyKey key ) {
            _lastSlot = ( JudyValue * ) judy_strt( _judyarray, ( const unsigned char * ) &key, 0 );
            return mostRecentPair();
        }

        /// retrieve the cell pointer, or return NULL for a given key.
        JudyValue find( JudyKey key ) {
            _lastSlot = ( JudyValue * ) judy_slot( _judyarray, ( const unsigned char * ) &key, 0 );
            if( _lastSlot ) {
                _success = true;
                return *_lastSlot;
            } else {
                _success = false;
                return 0;
            }
        }

        /// retrieve the key-value pair for the most recent judy query.
        inline const pair mostRecentPair() {
            pair kv;
            judy_key( _judyarray, _buff, 0 );
            if( _lastSlot ) {
                kv.value = *_lastSlot;
                _success = true;
            } else {
                kv.value = ( JudyValue ) 0;
                _success = false;
            }
            kv.key = ( JudyKey ) * _buff;
            return kv;
        }

        /// retrieve the last key-value pair in the array
        const pair & end() {
            _lastSlot = ( JudyValue * ) judy_end( _judyarray );
            return mostRecentPair();
        }

        /// retrieve the key-value pair for the next string in the array.
        const pair & next() {
            _lastSlot = ( JudyValue * ) judy_nxt( _judyarray );
            return mostRecentPair();
        }

        /// retrieve the key-value pair for the prev string in the array.
        const pair & previous() {
            _lastSlot = ( JudyValue * ) judy_prv( _judyarray );
            return mostRecentPair();
        }

        /** delete a key-value pair. If the array is not empty,
         * getLastValue() will return the entry before the one that was deleted
         * \sa isEmpty()
         */
        bool removeEntry( JudyKey * key ) {
            if( judy_slot( _judyarray, key, sizeof( key ) ) ) {
                _lastSlot = ( JudyValue * ) judy_del( _judyarray );
                return true;
            } else {
                return false;
            }
        }

        ///return true if the array is empty
        bool isEmpty() {
            return ( _judyarray ? false : true );
        }
};
#endif //JUDYLARRAY_CPP_H
