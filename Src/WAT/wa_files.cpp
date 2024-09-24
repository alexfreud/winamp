#include "WAT.h"

bool wa::files::file_exists( const char *p_filename )
{
	if ( p_filename == NULL )
		return false;


	struct stat l_buffer;
	return ( stat( p_filename, &l_buffer ) == 0 );
}

bool wa::files::file_exists( const std::string &p_filename )
{
	return wa::files::file_exists( p_filename.c_str() );
}

bool wa::files::file_exists( const wchar_t *p_filename )
{
	return wa::files::file_exists( wa::strings::convert::to_string( p_filename ) );
}


int wa::files::file_size( const char *p_filename )
{
    int l_file_size = -1;

    struct stat l_file_info{};


    if ( !stat( p_filename, &l_file_info ) )
        l_file_size = l_file_info.st_size;


    return l_file_size;
}

int wa::files::file_size( const wchar_t *p_filename )
{
    std::string l_filename = wa::strings::convert::to_string( p_filename );

    return file_size( l_filename.c_str() );
}


bool wa::files::folder_exists( const char *p_folder )
{
    struct stat info;

    if ( stat( p_folder, &info) != 0 )
        return false;
    else if ( info.st_mode & S_IFDIR )
        return true;
    else
        return false;
}


bool wa::files::getFilenamesFromFolder( std::vector<std::string> &p_result, const std::string &p_folder_path, const std::string &p_reg_ex, const size_t p_limit )
{
	_finddata_t l_file_info;
	std::string l_file_pattern = p_folder_path + "\\" + p_reg_ex;

    intptr_t l_handle = _findfirst( l_file_pattern.c_str(), &l_file_info );
    //If folder_path exsist, using l_file_pattern will find at least two files "." and "..", 
    //of which "." means current dir and ".." means parent dir
    if ( l_handle != -1 )
    {
        //iteratively check each file or sub_directory in current folder
        do
        {
            std::string l_file_name = l_file_info.name; //from char array to string
            //check whtether it is a sub direcotry or a file
            if ( l_file_info.attrib & _A_SUBDIR )
            {
                if ( l_file_name != "." && l_file_name != ".." )
                    wa::files::getFilenamesFromFolder( p_result, p_folder_path + "\\" + l_file_name, p_reg_ex );
            }
            else
                p_result.push_back( p_folder_path + "\\" + l_file_name );
                
        } while ( _findnext( l_handle, &l_file_info ) == 0 && p_result.size() < p_limit - 1 );


        _findclose( l_handle );

        return true;
    }

    //
    _findclose( l_handle );

	return false;
}
