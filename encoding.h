#pragma once
#ifndef MARTY_ENCODING_API_INCLUDED
#define MARTY_ENCODING_API_INCLUDED

// Страж включения, нужен, в том числе, чтобы детектить подключенный API в других хидерах

#include <cstdint>

#if defined(_WIN32) || defined(WIN32)

    #include <winsock2.h>
    #include <windows.h>

#else

    #ifndef UINT
        #define UINT  std::uint32_t
    #endif

#endif

#include <cstddef>
#include <map>
#include <string>

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
namespace encoding {



//----------------------------------------------------------------------------
// https://learn.microsoft.com/en-us/windows/console/console-functions

//----------------------------------------------------------------------------
UINT getConsoleInputCodePage();
UINT getConsoleOutputCodePage();
UINT getSystemCodePage();

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
#if defined(_MSC_VER)
   #pragma warning(push)
   #pragma warning(disable:4820) // - warning C4820: 'encoding::WindowsEncodingInfo': '4' bytes padding added after data member 'encoding::WindowsEncodingInfo::codePage'
#endif

//----------------------------------------------------------------------------
struct WindowsEncodingInfo
{
    UINT           codePage;
    const char*    codePageIdName;
    const char*    codePageInfo;
};

//----------------------------------------------------------------------------
#if defined(_MSC_VER)
   #pragma warning(pop)
#endif

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
class EncodingsApi;

EncodingsApi* getEncodingsApi();

class EncodingsApi
{

public:

    // EncodingsApi::codepage_type
    using codepage_type = UINT;

    static const UINT cpid_UTF16   = 1200; // LE
    static const UINT cpid_UTF16BE = 1201;
    static const UINT cpid_UTF8    = 65001;

    friend EncodingsApi* getEncodingsApi();

    UINT getCodePageByName( const std::string name );
    std::string getCodePageName( UINT id );
    std::string getCodePageInfo( UINT id );

    std::wstring decode ( const char   * data, std::size_t size, UINT cp );
    std::string  encode ( const wchar_t* data, std::size_t size, UINT cp );
    std::string  detect ( const char   * data, std::size_t size, std::size_t &bomSize, std::string httpHint=std::string(), std::string metaHint=std::string() );
    std::string  convert( const char   * data, std::size_t size, UINT cpSrc, UINT cpDst );

    std::wstring decode( const std::string &str, UINT cp )                 { return decode( str.data(), str.size(), cp); }
    std::string  encode( const std::wstring &str, UINT cp )                { return encode( str.data(), str.size(), cp); }
    std::string  detect( const std::string &str, std::size_t &bomSize, std::string httpHint=std::string(), std::string metaHint=std::string() )
    {
        return detect( str.data(), str.size(), bomSize, httpHint, metaHint );
    }

    std::string  detect( const std::string &str, std::size_t *pBomSize=0, std::string httpHint=std::string(), std::string metaHint=std::string() )
    {
        std::size_t bomSize;
        auto res = detect( str.data(), str.size(), bomSize, httpHint, metaHint );
        if (pBomSize)
           *pBomSize = bomSize;
        return res;
    }

    std::string  convert( const std::string &str, UINT cpSrc, UINT cpDst ) { return convert( str.data(), str.size(), cpSrc, cpDst ); }

    bool isEqualEncodingNames( const std::string &enc1, const std::string &enc2 );

    bool getEncodingInfo( std::size_t encNumber, std::string &name, std::string &description);

    //! return code page ID or 0, if no BOM found or BOM is invalid. Can return cpid_UTF16, cpid_UTF16BE or cpid_UTF8
    UINT checkTheBom( const char* data, std::size_t size, std::size_t *pBomLen = 0 );
    UINT checkTheBom( const std::string &text, std::size_t *pBomLen = 0 )
    {
        return checkTheBom(text.data(), text.size(), pBomLen);
    }

    // returns BOM removed from text argument
    std::string stripTheBom(std::string &text)
    {
        std::size_t bomSize = 0;
        UINT cpId = checkTheBom(text, &bomSize);
        if (!cpId)
            std::string();
        if (bomSize)
        {
            std::string bom = std::string(text, 0, bomSize);
            text.erase(0,bomSize);
            return bom;
        }

        return std::string();
    }

private:

    UINT tryGetCodePageByName( const std::string name );

    static std::map< std::string, UINT>  nameToId;
    static std::map< UINT, std::string>  idToName;
    static std::map< UINT, std::string>  idToInfo;

    EncodingsApi();
};

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct BomStripper
{
    std::string operator()(std::string &text) const
    {
        return getEncodingsApi()->stripTheBom(text);
    }
};

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::string toUtf8(const std::wstring &str)
{
    auto pApi = getEncodingsApi();
    return pApi->encode(str, EncodingsApi::cpid_UTF8);
}

//----------------------------------------------------------------------------
inline
std::string toUtf8(const wchar_t *pStr)
{
    if (!pStr)
        return std::string();

    auto pApi = getEncodingsApi();
    return pApi->encode(std::wstring(pStr), EncodingsApi::cpid_UTF8);
}

//----------------------------------------------------------------------------
inline
std::string toUtf8(wchar_t ch)
{
    auto wstr = std::wstring(1,ch);
    return toUtf8(wstr);
}

//----------------------------------------------------------------------------
//! Детектим кодировку и конвертим в UTF-8
inline
std::string toUtf8(const std::string &text)
{
    EncodingsApi* pApi = getEncodingsApi();

    std::size_t bomSize = 0;
    UINT bomCpId = pApi->checkTheBom(text, &bomSize);
    if (bomCpId==EncodingsApi::cpid_UTF8)
    {
        return std::string(text, bomSize, text.size()-bomSize); // return without BOM
    }

    std::string detectedCpName = pApi->detect( text /* , bomSize */  );
    if (detectedCpName.empty())
        return text;

    auto cpId = pApi->getCodePageByName(detectedCpName);
    if (cpId==0)
        return text;

    if (!bomSize)
    {
        return pApi->convert( text, cpId, EncodingsApi::cpid_UTF8 );
    }
    else
    {
        return pApi->convert( std::string(text, bomSize, text.size()-bomSize), cpId, EncodingsApi::cpid_UTF8 );
    }
}

//----------------------------------------------------------------------------
struct ToUtf8
{
    std::string operator()(const std::wstring &str) const
    {
        return toUtf8(str);
    }
    std::string operator()(const std::string &str) const
    {
        return toUtf8(str);
    }
};

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
std::wstring fromUtf8(const std::string &str)
{
    auto pApi = getEncodingsApi();
    return pApi->decode(str, EncodingsApi::cpid_UTF8);
}

//----------------------------------------------------------------------------
inline
std::wstring fromUtf8(const char *pStr)
{
    if (!pStr)
        return std::wstring();

    auto pApi = getEncodingsApi();
    return pApi->decode(std::string(pStr), EncodingsApi::cpid_UTF8);
}

//----------------------------------------------------------------------------
inline
std::wstring fromUtf8(char ch)
{
    auto str = std::string(1,ch);
    return fromUtf8(str);
}

//----------------------------------------------------------------------------
struct FromUtf8
{
    std::wstring operator()(const std::string &str) const
    {
        return fromUtf8(str);
    }
};


//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::wstring toUnicodeAuto(const std::wstring &str, EncodingsApi::codepage_type *pCodepageId=0)
{
    if (pCodepageId)
       *pCodepageId = 0;
    return str;
}

//----------------------------------------------------------------------------
inline
std::wstring toUnicodeAuto(const std::wstring::value_type *str, EncodingsApi::codepage_type *pCodepageId=0)
{
    if (pCodepageId)
       *pCodepageId = 0;
    return str ? std::wstring(str) : std::wstring();
}

//----------------------------------------------------------------------------
inline
std::string toUnicodeAuto(std::string str, EncodingsApi::codepage_type *pCodepageId=0)
{
    // !!! Надо бы тут разобраться с файлами, которые UTF-16

    // static const UINT cpid_UTF16   = 1200; // LE
    // static const UINT cpid_UTF16BE = 1201;

    EncodingsApi* pEncApi = getEncodingsApi();

    size_t bomSize = 0;
    std::string detectRes = pEncApi->detect( str, bomSize );

    if (bomSize)
    {
        str.erase(0,bomSize);
    }

    auto cpId = pEncApi->getCodePageByName(detectRes);

    if (pCodepageId)
       *pCodepageId = cpId;

    return pEncApi->convert( str, cpId, EncodingsApi::cpid_UTF8 );
}

//----------------------------------------------------------------------------
inline
std::string toUnicodeAuto(const std::string::value_type *str, EncodingsApi::codepage_type *pCodepageId=0)
{
    if (pCodepageId)
       *pCodepageId = 0;

    return str ? toUnicodeAuto(std::string(str), pCodepageId) : std::string();
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::string fromUnicodeToCodepage(const std::string &str, EncodingsApi::codepage_type toCp)
{
    if (toCp==0 || toCp==EncodingsApi::cpid_UTF8)
        return str;

    return getEncodingsApi()->convert( str, EncodingsApi::cpid_UTF8, toCp);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::wstring toUnicodeFromConsole(const std::wstring &str)
{
    return str;
}

//----------------------------------------------------------------------------
inline
std::wstring toUnicodeFromConsole(const std::wstring::value_type *str)
{
    return str ? std::wstring(str) : std::wstring();
}

//----------------------------------------------------------------------------
inline
std::string toUnicodeFromConsole(const std::string &str)
{
    EncodingsApi* pApi = getEncodingsApi();
    //return pApi->decode(str, getConsoleInputCodePage());
    //std::string  convert( const std::string &str, UINT cpSrc, UINT cpDst ) { return convert( str.data(), str.size(), cpSrc, cpDst ); }
    return pApi->convert( str, getConsoleInputCodePage(), encoding::EncodingsApi::cpid_UTF8 );
}

//----------------------------------------------------------------------------
inline
std::string toUnicodeFromConsole(const std::string::value_type *str)
{
    return str ? toUnicodeFromConsole(std::string(str)) : std::string();
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::wstring fromConsoleMultibyte(const std::string &str) // С фига ли мультибайт?
{
    EncodingsApi* pApi = getEncodingsApi();
    return pApi->decode(str, getConsoleInputCodePage());
}

//----------------------------------------------------------------------------
inline
std::wstring fromConsoleMultibyte(const char *pStr) // С фига ли мультибайт?
{
    if (!pStr)
        return std::wstring();

    return fromConsoleMultibyte(std::string(pStr));
}

//----------------------------------------------------------------------------
inline
std::wstring fromConsoleMultibyte(char ch) // С фига ли мультибайт?
{
    return fromConsoleMultibyte(std::string(1, ch));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::string toConsoleMultibyte(const std::wstring &str)
{
    EncodingsApi* pApi = getEncodingsApi();
    return pApi->encode(str, getConsoleOutputCodePage());
}

//----------------------------------------------------------------------------
inline
std::string toConsoleMultibyte(const wchar_t *pStr)
{
    if (!pStr)
        return std::string();

    return toConsoleMultibyte(std::wstring(pStr));
}

//----------------------------------------------------------------------------
inline
std::string toConsoleMultibyte(wchar_t ch)
{
    return toConsoleMultibyte(std::wstring(1, ch));
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
std::wstring fromSystemMultibyte(const std::string &str)
{
    EncodingsApi* pApi = getEncodingsApi();
    return pApi->decode(str, getSystemCodePage());
}

//----------------------------------------------------------------------------
inline
std::wstring fromSystemMultibyte(const char *pStr)
{
    if (!pStr)
        return std::wstring();

    return fromSystemMultibyte(std::string(pStr));
}

//----------------------------------------------------------------------------
inline
std::wstring fromSystemMultibyte(char ch)
{
    return fromSystemMultibyte(std::string(1, ch));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::string toSystemMultibyte(const std::wstring &str)
{
    EncodingsApi* pApi = getEncodingsApi();
    return pApi->encode(str, getSystemCodePage());
}

//----------------------------------------------------------------------------
inline
std::string toSystemMultibyte(const wchar_t *pStr)
{
    if (!pStr)
        return std::string();

    return toSystemMultibyte(std::wstring(pStr));
}

//----------------------------------------------------------------------------
inline
std::string toSystemMultibyte(wchar_t ch)
{
    return toSystemMultibyte(std::wstring(1, ch));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// https://learn.microsoft.com/en-us/windows/console/console-functions

//----------------------------------------------------------------------------
inline
UINT getConsoleInputCodePage()
{
#if defined(_WIN32) || defined(WIN32)
    // https://learn.microsoft.com/en-us/windows/console/getconsolecp
    return GetConsoleCP();
#else
    return EncodingsApi::cpid_UTF8; // В этих ваших линупсах и пр. обычно UTF-8
#endif
}

inline
UINT getConsoleOutputCodePage()
{
#if defined(_WIN32) || defined(WIN32)
    // https://learn.microsoft.com/en-us/windows/console/getconsoleoutputcp
    return GetConsoleOutputCP();
#else
    return EncodingsApi::cpid_UTF8; // В этих ваших линупсах и пр. обычно UTF-8
#endif
}

// Возвращает кодовую страницу неюникодного приложения (Ansi Code Page)
inline
UINT getSystemCodePage()
{
#if defined(_WIN32) || defined(WIN32)
    return GetACP();
#else
    return EncodingsApi::cpid_UTF8; // В этих ваших линупсах и пр. обычно UTF-8
#endif
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

} // namespace encoding



//----------------------------------------------------------------------------

#if defined(GLOBAL_ENCODINGS_API)

    using encoding::EncodingsApi;

#endif

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

#endif // MARTY_ENCODING_API_INCLUDED


