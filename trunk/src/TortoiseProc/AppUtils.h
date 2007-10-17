// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2007 - TortoiseSVN

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
#pragma once
#include "HistoryCombo.h"
#include "SVNRev.h"

class CTSVNPath;

/**
 * \ingroup TortoiseProc
 * An utility class with static functions.
 */
class CAppUtils
{
public:
	/**
	* Flags for StartExtDiff function.
	*/
	struct DiffFlags
	{
		bool bWait;
		bool bBlame;
		bool bReadOnly;
		bool bAlternativeTool; // If true, invert selection of TortoiseMerge vs. external diff tool

		DiffFlags(): bWait(false), bBlame(false), bReadOnly(false), bAlternativeTool(false)	{}
		DiffFlags& Wait(bool b = true) { bWait = b; return *this; }
		DiffFlags& Blame(bool b = true) { bBlame = b; return *this; }
		DiffFlags& ReadOnly(bool b = true) { bReadOnly = b; return *this; }
		DiffFlags& AlternativeTool(bool b = true) { bAlternativeTool = b; return *this; }
	};

	CAppUtils(void);
	~CAppUtils(void);

	/**
	 * Launches the external merge program if there is one.
	 * \return TRUE if the program could be started
	 */
	static BOOL StartExtMerge(
		const CTSVNPath& basefile, const CTSVNPath& theirfile, const CTSVNPath& yourfile, const CTSVNPath& mergedfile,
		const CString& basename = CString(), const CString& theirname = CString(), const CString& yourname = CString(),
		const CString& mergedname = CString(), bool bReadOnly = false);

	/**
	 * Starts the external patch program (currently always TortoiseMerge)
	 */
	static BOOL StartExtPatch(const CTSVNPath& patchfile, const CTSVNPath& dir, 
			const CString& sOriginalDescription = CString(), const CString& sPatchedDescription = CString(), 
			BOOL bReversed = FALSE, BOOL bWait = FALSE);

	/**
	 * Starts the external unified diff viewer (the app associated with *.diff or *.patch files).
	 * If no app is associated with those filetypes, the default text editor is used.
	 */
	static BOOL StartUnifiedDiffViewer(const CTSVNPath& patchfile, const CString& title, BOOL bWait = FALSE);

	/**
	 * Starts the external diff application
	 */
	static bool StartExtDiff(
		const CTSVNPath& file1, const CTSVNPath& file2, 
		const CString& sName1, const CString& sName2, const DiffFlags& flags);

	/**
	 * Starts the external diff application for properties
	 */
	static BOOL StartExtDiffProps(const CTSVNPath& file1, const CTSVNPath& file2, 
			const CString& sName1 = CString(), const CString& sName2 = CString(),
			BOOL bWait = FALSE, BOOL bReadOnly = FALSE);

	/**
	 * Launches the standard text viewer/editor application which is associated
	 * with txt files.
	 * \return TRUE if the program could be started.
	 */
	static BOOL StartTextViewer(CString file);

	/**
	 * Checks if the given file has a size of less than four, which means
	 * an 'empty' file or just newlines, i.e. an empty diff.
	 */
	static BOOL CheckForEmptyDiff(const CTSVNPath& sDiffPath);

	/**
	* Launch an external application (usually the diff viewer)
	*/
	static bool LaunchApplication(const CString& sCommandLine, UINT idErrMessageFormat, bool bWaitForStartup);

	/**
	* Launch the external blame viewer
	*/
	static bool LaunchTortoiseBlame(
		const CString& sBlameFile, const CString& sLogFile, const CString& sOriginalFile, const CString& sParams = CString());
	
	/**
	 * Resizes all columns in a list control. Considers also icons in columns
	 * with no text.
	 */
	static void ResizeAllListCtrlCols(CListCtrl * pListCtrl);

	static bool BrowseRepository(CHistoryCombo& combo, CWnd * pParent, SVNRev& rev);

	static bool FileOpenSave(CString& path, UINT title, UINT filter, bool bOpen, HWND hwndOwner = NULL);
private:
	static CString PickDiffTool(const CTSVNPath& file1, const CTSVNPath& file2);
	static bool GetMimeType(const CTSVNPath& file, CString& mimetype);
};
