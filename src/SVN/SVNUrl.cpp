// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2004 - Tim Kemp and Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "StdAfx.h"
#include "SVNUrl.h"

#include "SVN.h"
#include "Utils.h"
#include "UnicodeUtils.h"



// SVNUrl

SVNUrl::SVNUrl()
{
}

SVNUrl::SVNUrl(const CString& svn_url) :
	CString(Unescape(svn_url))
{
}

SVNUrl::SVNUrl(const CString& path, const CString& revision) :
	CString(Unescape(path + _T("?") + revision))
{
}

SVNUrl::SVNUrl(const CString& path, LONG revision) :
	CString(Unescape(path + _T("?") + GetTextFromRev(revision)))
{
}

SVNUrl::SVNUrl(const SVNUrl& other) :
	CString(other)
{
}

SVNUrl& SVNUrl::operator=(const CString& svn_url)
{
	CString::operator=(Unescape(svn_url));
	return *this;
}

SVNUrl& SVNUrl::operator=(const SVNUrl& svn_url)
{
	CString::operator=(svn_url);
	return *this;
}



// SVNUrl public interface

void SVNUrl::SetPath(const CString& path)
{
	CString new_path = Unescape(path);

	int rev_pos = ReverseFind(_T('?'));
	if (rev_pos >= 0)
		*this = new_path + Mid(rev_pos);
	else
		*this = new_path;
}

CString SVNUrl::GetPath(bool escaped) const
{
	CString path = *this;

	int rev_pos = path.ReverseFind(_T('?'));
	if (rev_pos >= 0)
		path = path.Left(rev_pos);

	if (escaped)
		return Escape(path);
	else
		return path;
}

void SVNUrl::SetRevision(LONG revision)
{
	CString svn_url;

	if (revision == SVN::REV_HEAD)
	{
		svn_url = GetPath();
	}
	else
	{
		svn_url = GetPath() + GetTextFromRev(revision);
	}

	*this = svn_url;
}

LONG SVNUrl::GetRevision() const
{
	int rev_pos = ReverseFind(_T('?'));

	if (rev_pos < 0)
	{
		return SVN::REV_HEAD;
	}
	else
	{
		CString revision = Mid(rev_pos + 1);
		if (revision.CompareNoCase(_T("HEAD")) == 0)
		{
			return SVN::REV_HEAD;
		}
		else if (revision.CompareNoCase(_T("BASE")) == 0)
		{
			return SVN::REV_BASE;
		}
		else if (revision.CompareNoCase(_T("WC")) == 0)
		{
			return SVN::REV_WC;
		}
		return _tcstol(revision, 0, 10);
	}
}

CString SVNUrl::GetRevisionText() const
{
	int rev_pos = ReverseFind(_T('?'));

	if (rev_pos < 0)
		return _T("HEAD");
	else
		return Mid(rev_pos + 1).MakeUpper();
}

bool SVNUrl::IsRoot() const
{
	return GetParentPath().IsEmpty();
}

CString SVNUrl::GetRootPath() const
{
	SVNUrl url = *this;

	while (!url.IsRoot())
		url = url.GetParentPath();

	return url;
}

CString SVNUrl::GetParentPath() const
{
	CString path = GetPath();

	int rev_pos = path.ReverseFind('/');

	if (rev_pos <= 0)
	{
		path = _T("");
	}
	else
	{
		path = path.Left(rev_pos);
		path.TrimRight('/');
		if (path.ReverseFind('/') < 0)
			path = _T("");
	}

	return path;
}

CString SVNUrl::GetName() const
{
	CString path = GetPath();

	int rev_pos = path.ReverseFind('/');

	if (rev_pos > 0)
	{
		path = path.Left(rev_pos - 1);
		if (path.ReverseFind('/') < 0)
			path = GetPath();
	}

	return path;
}



// SVNUrl static helpers

CString SVNUrl::Escape(const CString& url)
{
	return CUtils::PathEscape(url);
}

CString SVNUrl::Unescape(const CString& url)
{
	CString new_url = url;
	new_url.Replace('\\', '/');
	new_url.TrimRight('/');

	CStringA temp = CUnicodeUtils::GetUTF8(new_url);
	CUtils::Unescape(temp.GetBuffer());
	temp.ReleaseBuffer();
	return CUnicodeUtils::GetUnicode(temp);
}

CString SVNUrl::GetTextFromRev(LONG revision)
{
	CString rev_text;

	if (revision == SVN::REV_HEAD)
	{
		rev_text = _T("HEAD");
	}
	else if (revision == SVN::REV_BASE)
	{
		rev_text = _T("BASE");
	}
	else if (revision == SVN::REV_WC)
	{
		rev_text = _T("WC");
	}
	else
	{
		rev_text.Format(_T("%u"), revision);
	}

	return rev_text;
}

 

#ifdef _DEBUG

// This test can easily be integrated in a CPPUNIT framework

#ifndef CPPUNIT_ASSERT
#define CPPUNIT_ASSERT(x) ASSERT(x)
#endif

static class CSVNUrlTest {
public:
	CSVNUrlTest()
	{
		// Constructor tests
		{
			CPPUNIT_ASSERT( SVNUrl().IsEmpty() );
			CPPUNIT_ASSERT( SVNUrl() == _T(""));
			CPPUNIT_ASSERT( SVNUrl(_T("")).IsEmpty() );
			CPPUNIT_ASSERT( SVNUrl(_T("")) == _T(""));
			CPPUNIT_ASSERT( SVNUrl(_T("x")) == _T("x"));
			CPPUNIT_ASSERT( SVNUrl(_T("/x")) == _T("/x"));
			CPPUNIT_ASSERT( SVNUrl(_T("/x/")) == _T("/x"));
			CPPUNIT_ASSERT( SVNUrl(_T("\\x")) == _T("/x"));
			CPPUNIT_ASSERT( SVNUrl(_T("\\x\\")) == _T("/x"));
//			CPPUNIT_ASSERT( SVNUrl(_T("M%FCller")) == _T("M�ller"));
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b/c")) == _T("http://a/b/c"));
			CPPUNIT_ASSERT( SVNUrl(_T("http:\\\\a\\b\\c")) == _T("http://a/b/c"));
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:/a/b/c")) == _T("file:///x:/a/b/c"));
			CPPUNIT_ASSERT( SVNUrl(_T("file:\\\\\\x:\\a\\b\\c")) == _T("file:///x:/a/b/c"));
		}

		// GetPath/SetPath tests
		{
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b")).GetPath() == _T("http://a/b") );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b/")).GetPath() == _T("http://a/b") );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b?22")).GetPath() == _T("http://a/b") );
//			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b/?22")).GetPath() == _T("http://a/b") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a/b")).GetPath() == _T("file:///a/b") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a/b/")).GetPath() == _T("file:///a/b") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a/b?22")).GetPath() == _T("file:///a/b") );
//			CPPUNIT_ASSERT( SVNUrl(_T("file:///a/b/?22")).GetPath() == _T("file:///a/b") );
		}

		// GetRevision/SetRevision tests
		{
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b")).GetRevision() == SVN::REV_HEAD );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b/")).GetRevision() == SVN::REV_HEAD );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b?42")).GetRevision() == 42 );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b/?42")).GetRevision() == 42 );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b?HEAD")).GetRevision() == SVN::REV_HEAD );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b?BASE")).GetRevision() == SVN::REV_BASE );

			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b")).GetRevisionText() == _T("HEAD") );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b?42")).GetRevisionText() == _T("42") );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b?HEAD")).GetRevisionText() == _T("HEAD") );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b?bAsE")).GetRevisionText() == _T("BASE") );
		}

		// URL root tests
		{
			CPPUNIT_ASSERT( SVNUrl(_T("http://a")).IsRoot() == true );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a")).GetRootPath() == _T("http://a") );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/")).IsRoot() == true );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/")).GetRootPath() == _T("http://a") );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b")).IsRoot() == false );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a/b")).GetRootPath() == _T("http://a") );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a.b.com")).IsRoot() == true );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a.b.com")).GetRootPath() == _T("http://a.b.com") );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a.b.com/d/e")).IsRoot() == false );
			CPPUNIT_ASSERT( SVNUrl(_T("http://a.b.com/d/e")).GetRootPath() == _T("http://a.b.com") );

			CPPUNIT_ASSERT( SVNUrl(_T("file:///a")).IsRoot() == true );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a")).GetRootPath() == _T("file:///a") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a/")).IsRoot() == true );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a/")).GetRootPath() == _T("file:///a") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a/b")).IsRoot() == false );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a/b")).GetRootPath() == _T("file:///a") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a.b.com")).IsRoot() == true );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a.b.com")).GetRootPath() == _T("file:///a.b.com") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a.b.com/d/e")).IsRoot() == false );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///a.b.com/d/e")).GetRootPath() == _T("file:///a.b.com") );

			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:")).IsRoot() == true );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:")).GetRootPath() == _T("file:///x:") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:/")).IsRoot() == true );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:/")).GetRootPath() == _T("file:///x:") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:/a")).IsRoot() == false );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:/a")).GetRootPath() == _T("file:///x:") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:/a")).GetPath() == _T("file:///x:/a") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:/a/")).IsRoot() == false );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:/a/")).GetRootPath() == _T("file:///x:") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:/a/")).GetPath() == _T("file:///x:/a") );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:/a/b")).IsRoot() == false );
			CPPUNIT_ASSERT( SVNUrl(_T("file:///x:/a/b")).GetRootPath() == _T("file:///x:") );
		}
	}
} SVNUrlTest;

#endif
