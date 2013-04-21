#ifndef JUDYL2ARRAY_H
#define JUDYL2ARRAY_H

/****************************************************************************//**
* \file judyL2Array.h C++ wrapper for judyL2 array implementation
*
* A judyL2 array maps JudyKey's to multiple JudyValue's, similar to
* std::multimap. Internally, this is a judyL array of std::vector< JudyValue >.
*
*    Author: Mark Pictor. Public domain.
*
********************************************************************************/

#include "judy.h"
#include "assert.h"
#include <vector>
#include <stdint.h>

template< typename JudyKey, typename vec >
struct judyl2KVpair {
    JudyKey key;
    vec value;
};

/** A judyL2 array maps JudyKey's to multiple JudyValue's, similar to std::multimap.
 * Internally, this is a judyL array of std::vector< JudyValue >.
 * The first template parameter must be the same size as a void*
 *  \param JudyKey the type of the key, i.e. uint64_t, etc
 *  \param JudyValue the type of the value, i.e. int, pointer-to-object, etc. With judyL2Array, the size of this value can vary.
 */
template< typename JudyKey, typename JudyValue >
class judyL2Array {
    public:
        typedef std::vector< JudyValue > vector;
        typedef const vector cvector;
        typedef judyl2KVpair< JudyKey, vector * > pair;
        typedef judyl2KVpair< JudyKey, cvector * > cpair;
    protected:
        Judy * _judyarray;
        unsigned int _maxLevels, _depth;
        vector ** _lastSlot;
        vector temp;
        JudyKey _buff[1];
        bool _success, _altMemStrategy, _valuesAreDeletablePointers;
        /// used when pointer may point to a vector or to a few values
        union taggedPtr {
            uintptr_t asInt;
            vector *  asPtr;
            vector ** asPtrPtr;
        };
        /// count values starting with * u.asPtr
        /// side effect: clears LSB of u; u points beyond last used cell when done
        unsigned int countStuffedValues( taggedPtr & tp ) {
            assert( tp.asInt & 0x1 );
            tp.asInt ^= 0x1;
            unsigned int count = 0;
            for( ; count < maxStuffedValues(); count++, tp.asPtr++ ) {
                if( ! (* tp.asPtrPtr ) ) {
                    break;
                }
            }
            return count;
        }
        void copyToVector() {
            taggedPtr tp;
            tp.asPtr = * _lastSlot;
            assert( tp.asInt & 0x1 );
            tp.asInt ^= 0x1;
            temp.clear();
            for( unsigned int i = 0; i < maxStuffedValues(); i++ ) {
                temp.push_back( ( JudyValue ) (* tp.asPtrPtr ) );
                tp.asPtr++;
            }
        }
        void migrateToVector() {
            taggedPtr tp;
            tp.asPtr = * _lastSlot;
            assert( tp.asInt & 0x1 );
            tp.asInt ^= 0x1;
            copyToVector();
            * _lastSlot = tp.asPtr;
            new( * _lastSlot ) vector; //create vector in place
            ( * _lastSlot )->swap( temp );
        }
    public:
        /** \param altMemStrategy If true, the array will consume more memory when first populated, but less when
         * large numbers of values have been added and keys (or many values) have been removed. When false, placement new
         * and judy_data() are used to place some data(*) within the judy array. The downside is that this memory cannot be
         * freed until the judy array is deleted.
         *
         * (*) The data that is placed within the array is either the vector object, or however many JudyValue's will fit
         * within sizeof(vector) bytes. Uses tagged pointers, so this will not work on plat's with unaligned malloc.
         *
         * \param valuesAreDeletablePointers If true, operator delete will be called for each value when it is removed
         * from the array (i.e. with clear() or when the destructor is called).
        */
        judyL2Array( bool altMemStrategy = false, bool valuesAreDeletablePointers = false ): _maxLevels( sizeof( JudyKey ) ),
                _depth( 1 ), _lastSlot( 0 ), _success( true ), _altMemStrategy( altMemStrategy ),
                _valuesAreDeletablePointers( valuesAreDeletablePointers ) {
            assert( sizeof( JudyKey ) == JUDY_key_size && "JudyKey *must* be the same size as a pointer!" );
            _judyarray = judy_open( _maxLevels, _depth );
            _buff[0] = 0;
        }

        explicit judyL2Array( const judyL2Array< JudyKey, JudyValue > & other ): _maxLevels( other._maxLevels ),
                _depth( other._depth ), _success( other._success ), _altMemStrategy( other._altMemStrategy ),
                _valuesAreDeletablePointers( other._valuesAreDeletablePointers ) {
            _judyarray = judy_clone( other._judyarray );
            _buff[0] = other._buff[0];
            find( _buff ); //set _lastSlot
        }

        /// calls clear, so should be safe to call at any point
        ~judyL2Array() {
            clear();
            judy_close( _judyarray );
        }

        inline unsigned int maxStuffedValues() const {
            if( _altMemStrategy ) {
                return 0;
            }
            return sizeof( vector ) / sizeof( judyvalue );
        }

        /// delete all vectors and empty the array
        void clear() {
            JudyKey key = 0;
            while( 0 != ( _lastSlot = ( vector ** ) judy_strt( _judyarray, ( const unsigned char * ) &key, 0 ) ) ) {
                if( _valuesAreDeletablePointers ) {
                    //TODO loop through vector contents, deleting everything
                }
                if( _altMemStrategy ) {
                    ( * _lastSlot )->~vector();
                } else {
                    delete( * _lastSlot );
                    judy_del( _judyarray );
                }
            }
        }
/* these need work, or need removed
        vector * getLastValue() {
            assert( _lastSlot );
            return &_lastSlot;
        }

        void setLastValue( vector * value ) {
            assert( _lastSlot );
            &_lastSlot = value;
        }
*/
        bool success() {
            return _success;
        }

        /* TODO
         * test for std::vector::shrink_to_fit (C++11), use it once the array is as full as it will be
         * void freeUnused() {...}
         */

        /// insert value into the vector for key.
        bool insert( JudyKey key, JudyValue value ) {
            _lastSlot = ( vector ** ) judy_cell( _judyarray, ( const unsigned char * ) &key, _depth * JUDY_key_size );
            if( _lastSlot ) {
                if( _altMemStrategy ) {
                    if( ! ( * _lastSlot ) ) {
                        * _lastSlot = new vector;
                    }
                    ( * _lastSlot )->push_back( value );
                } else {
                    taggedPtr tp;
                    if( * _lastSlot ) {
                        tp.asPtr = * _lastSlot;
                        if( tp.asInt & 0x1 ) { // LSB set -> storing individual JudyValue's in the space
                            unsigned int count = countStuffedValues( tp ); // NOTE - this function affects the value of 'u'
                            if( count < maxStuffedValues() ) {
                                * tp.asPtrPtr = (vector*) value;
//                                 std::cout << "pp " << u.asPtrPtr << std::endl;
                            } else {
                                migrateToVector();
                                ( * _lastSlot )->push_back( value );
                            }
                        } else { // LSB unset -> JudyValue's are in a vector
                            ( * _lastSlot )->push_back( value );
                        }
                    } else {
                        tp.asPtr = ( vector * ) judy_data( _judyarray, sizeof( vector ) );
                        tp.asInt |= 0x1;
                        * _lastSlot = tp.asPtr;
                        * tp.asPtrPtr = (vector*) value;
                        // judy_data clears the malloc'd memory, so it is not necessary to zero the following byte
                    }
                }
                _success = true;
            } else {
                _success = false;
            }
            return _success;
        }

        /** for a given key, append to or overwrite the vector
         * this never simply re-uses the pointer to the given vector because
         * that would mean that two keys could have the same value (pointer).
         */
        bool insert( JudyKey key, const vector & values, bool overwrite = false ) {
            _lastSlot = ( vector ** ) judy_cell( _judyarray, ( const unsigned char * ) &key, _depth * JUDY_key_size );
            if( _lastSlot ) {
                if( ! ( * _lastSlot ) ) {
                    * _lastSlot = new vector;
                    /* TODO store vectors inside judy with placement new
                     * (see other insert(), above)
                     */
                } else if( overwrite ) {
                    ( * _lastSlot )->clear();
                }
                std::copy( values.begin(), values.end(), std::back_inserter< vector >( ( ** _lastSlot ) ) );
                _success = true;
            } else {
                _success = false;
            }
            return _success;
        }

        /// retrieve the cell pointer greater than or equal to given key
        /// NOTE what about an atOrBefore function?
        const cpair atOrAfter( JudyKey key ) {
            _lastSlot = ( vector ** ) judy_strt( _judyarray, ( const unsigned char * ) &key, _depth * JUDY_key_size );
            return mostRecentPair();
        }

        /// retrieve the cell pointer, or return NULL for a given key.
        cvector * find( JudyKey key ) {
            _lastSlot = ( vector ** ) judy_slot( _judyarray, ( const unsigned char * ) &key, _depth * JUDY_key_size );
            if( ( _lastSlot ) && ( * _lastSlot ) ) {
                _success = true;
                return * _lastSlot;
            } else {
                _success = false;
                return 0;
            }
        }

        /// retrieve the key-value pair for the most recent judy query.
        inline const cpair mostRecentPair() {
            cpair kv;
            judy_key( _judyarray, (unsigned char *) _buff, _depth * JUDY_key_size );
            if( _lastSlot ) {
                kv.value = *_lastSlot;
                _success = true;
            } else {
                kv.value = ( JudyValue ) 0;
                _success = false;
            }
            kv.key = _buff[0];
            return kv;
        }

        /// retrieve the first key-value pair in the array
        const cpair & begin() {
            JudyKey key = 0;
            _lastSlot = ( vector ** ) judy_strt( _judyarray, ( const unsigned char * ) &key, 0 );
            return mostRecentPair();
        }

        /// retrieve the last key-value pair in the array
        const cpair & end() {
            _lastSlot = ( vector ** ) judy_end( _judyarray );
            return mostRecentPair();
        }

        /// retrieve the key-value pair for the next string in the array.
        const cpair & next() {
            _lastSlot = ( vector ** ) judy_nxt( _judyarray );
            return mostRecentPair();
        }

        /// retrieve the key-value pair for the prev string in the array.
        const cpair & previous() {
            _lastSlot = ( vector ** ) judy_prv( _judyarray );
            return mostRecentPair();
        }

        /** delete a key-value pair. If the array is not empty,
         * getLastValue() will return the entry before the one that was deleted
         * \sa isEmpty()
         */
        bool removeEntry( JudyKey key ) {
            if( 0 != ( _lastSlot = ( vector ** ) judy_slot( _judyarray, ( const unsigned char * ) &key, _depth * JUDY_key_size ) ) ) {
                // _lastSlot->~vector(); //for use with placement new
                delete _lastSlot;
                _lastSlot = ( vector ** ) judy_del( _judyarray );
                return true;
            } else {
                return false;
            }
        }

        /// true if the array is empty
        bool isEmpty() {
            JudyKey key = 0;
            return ( ( judy_strt( _judyarray, ( const unsigned char * ) &key, _depth * JUDY_key_size ) ) ? false : true );
        }
};
#endif //JUDYL2ARRAY_H
