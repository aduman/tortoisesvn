// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2007-2008 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "StdAfx.h"
#include "RepositoryInfo.h"
#include "CachedLogInfo.h"

#include "svn_client.h"

#include "SVN.h"
#include "TSVNPath.h"
#include "PathUtils.h"
#include "resource.h"
#include "Registry.h"
#include "GoOffline.h"

// begin namespace LogCache

namespace LogCache
{

// construct the dump file name

CString CRepositoryInfo::GetFileName() const
{
    return cacheFolder + _T("Repositories.dat");
}

// read the dump file

void CRepositoryInfo::Load()
{
    modified = false;

    // any cached info at all?

	if (GetFileAttributes (GetFileName()) == INVALID_FILE_ATTRIBUTES)
        return;

	CFile file (GetFileName(), CFile::modeRead | CFile::shareDenyWrite);
    CArchive stream (&file, CArchive::load);

    // format ID

    int version = 0;
    stream >> version;

    // number of entries to read
    // (old file don't have a version info -> "version" is the count)

    int count = 0;
    if (version >= MIN_COMPATIBLE_VERSION)
        stream >> count;
    else
        count = version;

    // actually load the data

    for (int i = 0; i < count; ++i)
    {
        int connectionState = online;

        SPerRepositoryInfo info;
        stream >> info.root 
               >> info.uuid
               >> info.headURL
               >> info.headRevision
               >> info.headLookupTime;

        if (version >= MIN_COMPATIBLE_VERSION)
            stream >> connectionState;

        info.connectionState = static_cast<ConnectionState>(connectionState);

        data[info.root] = info;
    }
}

// find cache entry (or data::end())

CRepositoryInfo::TData::iterator 
CRepositoryInfo::Lookup (const CString& url)
{
    // the repository root URL should be a prefix of url

    TData::iterator iter = data.lower_bound (url);

    // no suitable prefix?

    if (data.empty())
        return iter;

    // does prefix match?

    if ((iter != data.end()) && (iter->first == url))
        return iter;

    if (iter != data.begin())
    {
        --iter;
        if (iter->first == url.Left (iter->first.GetLength()))
		{
			// make sure we have the right entry
			if ((iter->first == url)||(url.GetAt(iter->first.GetLength())=='/'))
				return iter;
		}
    }

    // not found

    return data.end();
}

CRepositoryInfo::TData::iterator 
CRepositoryInfo::Lookup (const CTSVNPath& url)
{
    return Lookup (url.GetSVNPathString());
}

// does the user want to be this repository off-line?

bool CRepositoryInfo::IsOffline (SPerRepositoryInfo& info)
{
    // default connectivity setting

    CRegStdWORD defaultConnectionState (_T("Software\\TortoiseSVN\\DefaultConnectionState"), 0);

    // is this repository already off-line?

    if (info.connectionState != online)
        return true;

    // something went wrong. 

    if (defaultConnectionState == online)
    {
        // Default behavior is "Ask the user what to do"

        CGoOffline dialog;
        dialog.DoModal();
        if (dialog.asDefault)
            defaultConnectionState = dialog.selection;

        info.connectionState = dialog.selection;
        return info.connectionState != online;
    }
    else
    {
        // set default

        info.connectionState = static_cast<ConnectionState>
                                (static_cast<int>(defaultConnectionState));
        return true;
    }
}

// try to get the HEAD revision from the log cache

void CRepositoryInfo::SetHeadFromCache (SPerRepositoryInfo& info)
{
	SVN svn;
    CCachedLogInfo* cache = svn.GetLogCachePool()->GetCache (info.uuid);
    info.headRevision = cache != NULL
        ? cache->GetRevisions().GetLastCachedRevision()-1
        : NO_REVISION;

    // HEAD info is outdated

    info.headLookupTime = 0;
}

// construction / destruction: auto-load and save

CRepositoryInfo::CRepositoryInfo (SVN& svn, const CString& cacheFolderPath)
    : cacheFolder (cacheFolderPath)
    , modified (false)
    , svn (svn)
{
    // load the list only if the URL->UUID,head etc. mapping cache shall be used

    if (IsPermanent())
        Load();
}

CRepositoryInfo::~CRepositoryInfo(void)
{
}

// look-up and ask SVN if the info is not in cache. 
// cache the result.

CString CRepositoryInfo::GetRepositoryRoot (const CTSVNPath& url)
{
    CString uuid;
    return GetRepositoryRootAndUUID (url, uuid);
}

CString CRepositoryInfo::GetRepositoryUUID (const CTSVNPath& url)
{
    CString uuid;
    GetRepositoryRootAndUUID (url, uuid);
    return uuid;
}

CString CRepositoryInfo::GetRepositoryRootAndUUID ( const CTSVNPath& url
                                                  , CString& sUUID)
{
    TData::const_iterator iter = Lookup (url);

	// get time stamps and maximum uuid info age (default: 1 min)
	// we use the same setting as for the HEAD revision timeout, but with a
	// one minute default instead of 0 minutes
	__time64_t now = CTime::GetCurrentTime().GetTime();
	CRegStdWORD ageLimit (_T("Software\\TortoiseSVN\\HeadCacheAgeLimit"), 60);

    if (iter == data.end() || (now - iter->second.headLookupTime > ageLimit))
    {
		// TODO: if we found an UUID but the cache timed out, we should
		// compare the UUIDs and if they differ (i.e., another repository
		// is now at the URL we cached), we should abandon/clear the cache
		// completely.
        SPerRepositoryInfo info;
        info.root = svn.GetRepositoryRootAndUUID (url, info.uuid);
        info.headRevision = (revision_t)NO_REVISION;
        info.headLookupTime = -1;
        info.connectionState = online;

        if (!info.root.IsEmpty())
        {
            data [info.root] = info;
            modified = true;
        }

        sUUID = info.uuid;
        return info.root;
    }

    sUUID = iter->second.uuid;
    return iter->first;
}

revision_t CRepositoryInfo::GetHeadRevision (const CTSVNPath& url)
{
    // get the entry for that repository

    TData::iterator iter = Lookup (url);
    if (iter == data.end())
    {
        GetRepositoryRoot (url);
        iter = Lookup (url);
    }

    if (iter == data.end())
    {
        // there was some problem connecting to the repository

        return (revision_t)NO_REVISION;
    }

    // get time stamps and maximum head info age (default: 0 mins)

    __time64_t now = CTime::GetCurrentTime().GetTime();
    CRegStdWORD ageLimit (_T("Software\\TortoiseSVN\\HeadCacheAgeLimit"), 0);

    // is there a valid cached entry?

    if (   (now - iter->second.headLookupTime > ageLimit)
        || (   url.GetSVNPathString().Left (iter->second.headURL.GetLength())
            != iter->second.headURL)
        || (iter->second.headRevision == NO_REVISION))
    {
        // entry outdated or for not valid for this path

        if (iter->second.connectionState == online)
        {
            // we ain't off-line -> contact the server

            iter->second.headLookupTime = now;
            iter->second.headURL = url.GetSVNPathString();
            iter->second.headRevision = svn.GetHEADRevision (url);
        }

        // if we couldn't connect to the server, ask the user

        if (  (svn.Err && (svn.Err->apr_err != SVN_ERR_CANCELLED)) && (iter->second.headRevision == NO_REVISION)
            && IsOffline (iter->second))
        {
            // user wants to go off-line

            SetHeadFromCache (iter->second);

            // we just ignore our latest error

        	svn_error_clear (svn.Err);
            svn.Err = NULL;
        }

        modified = true;
    }

    // ready

    return iter->second.headRevision;
}

// make sure, we will ask the repository for the HEAD

void CRepositoryInfo::ResetHeadRevision (const CTSVNPath& url)
{
    // get the entry for that repository

    TData::iterator iter = Lookup (url);
    if (iter != data.end())
    {
        // there is actually a cache for this URL.
        // Invalidate the HEAD info and make sure we will
        // connect the server for an update the next time
        // we want to get connect.

        iter->second.headLookupTime = 0;
        iter->second.connectionState = online;
    }
}


// find the root folder to a given UUID (e.g. URL for given cache file).
// Returns an empty string, if no suitable entry has been found.

CString CRepositoryInfo::GetRootFromUUID (const CString& sUUID) const
{
    // scan all data

    for ( TData::const_iterator iter = data.begin(), end = data.end()
        ; iter != end
        ; ++iter)
    {
        if (iter->second.uuid == sUUID)
            return iter->first;
    }

    // not found

    return CString();
}

// do multiple URLs use this UUID?

bool CRepositoryInfo::HasMultipleURLs (const CString& uuid) const
{
    size_t urlCount = 0;

    for ( TData::const_iterator iter = data.begin(), end = data.end()
		; iter != end
		; ++iter)
	{
        if (iter->second.uuid == uuid)
            ++urlCount;
	}

    return urlCount > 1;
}

// get one of the many URLs that for the repository given by the UUID

CString CRepositoryInfo::GetFirstURL (const CString& uuid) const
{
    for ( TData::const_iterator iter = data.begin(), end = data.end()
		; iter != end
		; ++iter)
	{
        if (iter->second.uuid == uuid)
            return iter->second.root;
	}

    return CString();
}

// is the repository offline? 
// Don't modify the state if autoSet is false.

bool CRepositoryInfo::IsOffline (const CString& url, bool autoSet)
{
	// find the info

    TData::iterator iter = Lookup (url);

	// no info -> assume online (i.e. just try to reach the server)

	if (iter == data.end())
		return false;

	// update the online/offline state by contacting the user?
	// (the dialog will only be shown if online and no 
	// offline-defaults have been set)

	if (autoSet)
		IsOffline (iter->second);

    // return state

	return iter->second.connectionState != online;
}

// get the connection state (uninterpreted)

CRepositoryInfo::ConnectionState 
CRepositoryInfo::GetConnectionState (const CString& uuid)
{
	// find the info

    TData::iterator iter = Lookup (GetRootFromUUID (uuid));

	// no info -> assume online (i.e. just try to reach the server)

	return iter == data.end()
        ? online
        : iter->second.connectionState;
}

// remove a specific entry

void CRepositoryInfo::DropEntry (const CString& sUUID)
{
    TData::iterator iter = data.begin();
    while (iter != data.end())
    {
        if (iter->second.uuid == sUUID)
        {
			iter = data.erase (iter);
            modified = true;
        }
        else
            ++iter;
    }
}

// write all changes to disk

void CRepositoryInfo::Flush()
{
    if (!modified || !IsPermanent())
    {
        modified = false;
        return;
    }

	CString filename = GetFileName();
	CPathUtils::MakeSureDirectoryPathExists(filename.Left(filename.ReverseFind('\\')));
	CFile file (filename, CFile::modeWrite | CFile::modeCreate);
    CArchive stream (&file, CArchive::store);

    stream << static_cast<int>(VERSION);
    stream << static_cast<int>(data.size());

    for ( TData::const_iterator iter = data.begin(), end = data.end()
        ; iter != end
        ; ++iter)
    {
        // temp offline -> be online the next time

        ConnectionState connectionState 
            = static_cast<ConnectionState>(iter->second.connectionState & offline);

        stream << iter->second.root 
               << iter->second.uuid 
               << iter->second.headURL 
               << iter->second.headRevision 
               << iter->second.headLookupTime
               << connectionState;
    }

    modified = false;
}

// clear cache

void CRepositoryInfo::Clear()
{
    data.clear();
}

// get the owning SVN instance

SVN& CRepositoryInfo::GetSVN() const
{
	return svn;
}

// access to the result of the last SVN operation

svn_error_t* CRepositoryInfo::GetLastError() const
{
    return svn.Err;
}

// is this only temporary data?

bool CRepositoryInfo::IsPermanent() const
{
	CRegStdWORD ambiguousURL (_T("Software\\TortoiseSVN\\SupportAmbiguousURL"), FALSE);
	return ambiguousURL == FALSE;
}

// end namespace LogCache

}
