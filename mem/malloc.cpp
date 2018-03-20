#include <mem/malloc.hpp>
#include <mem/vmmap.hpp>

#include <debug.hpp>

namespace masys {
namespace mem {

void * SubpageAllocator::alloc( u32 size )
{
    if ( size == 0 )
        return nullptr;

    // Size is a multiple of 8
    size += 7;
    size -= size % 8;

    Chunk *chunk = freelist,
          *prev = nullptr;
    while ( chunk && chunk->size() < size ) {
        prev = chunk;
        chunk = chunk->next();
    }

    if ( ! chunk ) {
        u32 newpages = ( size + 16 + PAGE_SIZE - 1 ) / PAGE_SIZE;

        u32 newmem = pal->alloc( newpages );
        if ( ! newmem )
            return nullptr;

        chunk = reinterpret_cast< Chunk * >( newmem );
        DBGEXPR(( &chunk->pre_footer ));
        chunk->free( true );
        chunk->size( newpages * PAGE_SIZE - 16 );
        chunk->next() = nullptr;
        chunk->prev() = prev;
        chunk->following()->header.szfl = 0;
        if ( prev )
            prev->next() = chunk;
        dbg::sout() << "- malloc: new chunk of size " << chunk->size() << " at "
                    << dbg::hex() << chunk << '\n';
    }


    Chunk *newfree;
    if ( chunk->size() > size + 8 ) {
        newfree = chunk + ( size / 8 ) + 1;
        newfree->size( chunk->size() - size - 8 );
        newfree->free( true );
        newfree->next() = chunk->next();
        newfree->prev() = chunk;
        chunk->next() = newfree;
        chunk->size( size );
        dbg::sout() << "- malloc: residual chunk of size " << newfree->size()
                    << " at " << dbg::hex() << newfree << '\n';
    } else {
        newfree = chunk->next();
    }

    if ( chunk->prev() )
        chunk->prev()->next() = chunk->next();
    else
        freelist = chunk->next();
    if ( chunk->next() )
        chunk->next()->prev() = chunk->prev();

    chunk->free( false );
    return chunk->data();
}

void * SubpageAllocator::realloc( void *ptr, u32 size )
{
    if ( ptr == nullptr )
        return alloc( size );

    auto chunk = reinterpret_cast< Chunk * >( ptr ) - 1;
    if ( size <= chunk->size() )
        return ptr;

    if ( chunk->following()->free() &&
         size < chunk->following()->size() + chunk->size() + 8 )
    {
        Chunk *foll = chunk->following();
        u32 residual_size = foll->size() + chunk->size() + 8 - size;
        if ( residual_size > 8 ) { // shrink and move
            foll += ( foll->size() - residual_size ) / 8;
            foll->size( residual_size );
            foll->free( true );
            if ( foll->prev() )
                foll->prev()->next() = foll;
            else
                freelist = foll;
            if ( foll->next() )
                foll->next()->prev() = foll;
        } else { // obliterate
            if ( foll->prev() )
                foll->prev()->next() = foll->next();
            else
                freelist = foll->next();
            if ( foll->next() )
                foll->next()->prev() = foll->prev();
        }
        return ptr;
    }

    auto newmem = reinterpret_cast< char * >( alloc( size ) );
    if ( ! newmem )
        return nullptr;
    for ( unsigned i = 0; i < chunk->size(); ++i ) {
        newmem[ i ] = chunk->data()[ i ];
    }
    free( ptr );
    return newmem;
}

void SubpageAllocator::free( void *ptr )
{
    if ( ptr == nullptr )
        return;

    auto chunk = reinterpret_cast< Chunk * >( ptr ) - 1;
    chunk->free( true );

    Chunk *prec = chunk->preceding(), // Only valid if chunk->pre_footer.free
          *foll = chunk->following();

    if ( ! chunk->pre_footer.free() && ! foll->free() ) {
        freelist->prev() = chunk;
        chunk->next() = freelist;
        chunk->prev() = nullptr;
        freelist = chunk;
    } else if ( chunk->pre_footer.free() && ! foll->free() ) {
        prec->size( prec->size() + chunk->size() + 8 );
    } else if ( ! chunk->pre_footer.free() && foll->free() ) {
        chunk->next() = foll->next();
        chunk->prev() = foll->prev();
        if ( chunk->prev() )
            chunk->prev()->next() = chunk;
        else
            freelist = chunk;
        if ( chunk->next() )
            chunk->next()->prev() = chunk;
        chunk->size( chunk->size() + foll->size() + 8 );
    } else /* both free */ {
        prec->size( prec->size() + chunk->size() + 8 );
        chunk = prec;
        if ( foll->prev() )
            foll->prev()->next() = foll->next();
        else
            freelist = foll->next();
        chunk->size( chunk->size() + foll->size() + 8 );
    }
}

void SubpageAllocator::dump_freelist()
{
    dbg::sout() << "malloc's freelist:\n";
    auto chunk = freelist;
    if ( chunk->prev() )
        dbg::sout() << " +-- (broken prev, is " << chunk->prev() << ")\n";
    while ( chunk ) {
        dbg::sout() << " | chunk at " << chunk
                    << ", size: " << dbg::dec() << chunk->size() << '\n';
        if ( chunk->next() && chunk->next()->prev() != chunk )
            dbg::sout() << " +-- (broken prev, is " << chunk->next()->prev() << ")\n";
        chunk = chunk->next();
    }
    dbg::sout() << " `---\n";
}

} /* mem */
} /* masys */
