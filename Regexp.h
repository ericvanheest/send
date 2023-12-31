#ifndef __REGEXP_H__
#define __REGEXP_H__

class regexp;

class Regexp
{
public:
	enum { NSUBEXP = 10 };

	Regexp();
	Regexp( LPCTSTR exp, BOOL iCase = 0 );
	Regexp( const Regexp &r );
	~Regexp();
	const Regexp & operator=( const Regexp & r );

	bool Match( const TCHAR * s );
	int SubStrings() const;
	
    const std::wstring operator[]( unsigned int i ) const;
	int SubStart( unsigned int i ) const;
	int SubLength( unsigned int i ) const;

    std::wstring GetReplaceString( LPCTSTR source ) const;

    std::wstring GetErrorString() const;
	bool CompiledOK() const;

#if defined( _RE_DEBUG )
	void Dump();
#endif
private:
	const TCHAR * string;	/* used to return substring offsets only */
    mutable std::wstring m_szError;
	regexp * rc;

	void ClearErrorString() const;
	int safeIndex( unsigned int i ) const;

};

#endif

