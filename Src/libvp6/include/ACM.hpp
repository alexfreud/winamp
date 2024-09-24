#ifndef ACM_HPP
#define ACM_HPP

#include <windows.h>
#include <vfw.h>
#include <string>
#include <iosfwd>

namespace ACM
{
    const std::string versionAsStr(DWORD version);
    const std::string supportAsStr(DWORD support);

    std::ostream& operator<<(std::ostream&, const ACMDRIVERDETAILS&);

    std::ostream& operator<<(std::ostream&, const WAVEFORMATEX&);

    const std::string mmresultAsStr(MMRESULT);

    std::ostream& operator<<(
        std::ostream&, 
        const ACMFORMATTAGDETAILS&);

    std::ostream& operator<<(
        std::ostream&,
        const ACMFORMATDETAILS&);   
        
             
}

#endif
