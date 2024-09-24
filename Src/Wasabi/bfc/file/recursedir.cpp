#include "precomp_wasabi_bfc.h"
#include "recursedir.h"

RecurseDir::RecurseDir( const wchar_t *_path, const wchar_t *_match ) :
    path( _path ), match( _match )
{
    if ( match.isempty() ) match = Wasabi::Std::matchAllFiles();

    curdir = new ReadDir( path, match );
}

RecurseDir::~RecurseDir()
{
    dirstack.deleteAll();
}

int RecurseDir::next()
{
    for ( ;;)
    {
        if ( curdir == NULL )
        {	// pop one off the stack
            curdir = dirstack.getLast();
            if ( curdir == NULL ) return 0;	// done
            dirstack.removeLastItem();
        }
        int r = curdir->next();
        if ( r <= 0 )
        {
            delete curdir; curdir = NULL;
            continue;	// get another one
        }

        // ok, we have a file to look at
        if ( curdir->isDir() )
        {	// descend into it
            StringW newpath = curdir->getPath();
            newpath.AppendPath( curdir->getFilename() );

            dirstack.addItem( curdir );	// push the old one
            curdir = new ReadDir( newpath, match );	// start new one

            continue;
        }

        return r;
    }
}

const wchar_t *RecurseDir::getPath()
{
    if ( curdir == NULL )
        return NULL;
    return curdir->getPath();
}

const wchar_t *RecurseDir::getFilename()
{
    if ( curdir == NULL )
        return NULL;
    return curdir->getFilename();
}

const wchar_t *RecurseDir::getOriginalPath()
{
    return path;
}
