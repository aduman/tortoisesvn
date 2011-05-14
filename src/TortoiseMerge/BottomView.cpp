// TortoiseMerge - a Diff/Patch program

// Copyright (C) 2006-2011 - TortoiseSVN

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
#include "Resource.h"
#include "AppUtils.h"

#include "bottomview.h"

IMPLEMENT_DYNCREATE(CBottomView, CBaseView)

CBottomView::CBottomView(void)
{
    m_pwndBottom = this;
    m_pState = &m_AllState.bottom;
    m_nStatusBarID = ID_INDICATOR_BOTTOMVIEW;
}

CBottomView::~CBottomView(void)
{
}


void CBottomView::AddContextItems(CIconMenu& popup, DiffStates state)
{
    const bool bShow = HasSelection() && (state != DIFFSTATE_UNKNOWN);
    if (!bShow)
        return;

    popup.AppendMenuIcon(POPUPCOMMAND_USETHEIRBLOCK, IDS_VIEWCONTEXTMENU_USETHEIRBLOCK);
    popup.AppendMenuIcon(POPUPCOMMAND_USEYOURBLOCK, IDS_VIEWCONTEXTMENU_USEYOURBLOCK);
    popup.AppendMenuIcon(POPUPCOMMAND_USEYOURANDTHEIRBLOCK, IDS_VIEWCONTEXTMENU_USEYOURANDTHEIRBLOCK);
    popup.AppendMenuIcon(POPUPCOMMAND_USETHEIRANDYOURBLOCK, IDS_VIEWCONTEXTMENU_USETHEIRANDYOURBLOCK);

    CBaseView::AddContextItems(popup, state);
}


void CBottomView::UseBlock(CBaseView * pwndView, int nFirstViewLine, int nLastViewLine)
{
    CUndo::GetInstance().BeginGrouping(); // start group undo

    for (int viewLine = nFirstViewLine; viewLine <= nLastViewLine; viewLine++)
    {
        viewdata lineData = pwndView->GetViewData(viewLine);
        lineData.ending = lineendings;
        lineData.state = ResolveState(lineData.state);
        SetViewData(viewLine, lineData);
    }

    int nRemovedLines = CleanEmptyLines();
    SaveUndoStep();	
    UpdateViewLineNumbers();
    SaveUndoStep();

    CUndo::GetInstance().EndGrouping();

    // final clean up
    ClearSelection();
    SetupAllViewSelection(nFirstViewLine, nLastViewLine - nRemovedLines);
    BuildAllScreen2ViewVector();
    SetModified();
    RefreshViews();
}

void CBottomView::UseBothBlocks(CBaseView * pwndFirst, CBaseView * pwndLast)
{
    int nFirstViewLine; // first view line in selection
    int nLastViewLine; // last view line in selection

    if (!GetViewSelection(nFirstViewLine, nLastViewLine))
        return;

    int nNextViewLine = nLastViewLine + 1; // first view line after selected block

    CUndo::GetInstance().BeginGrouping(); // start group undo

    // use (copy) first block
    for (int viewLine = nFirstViewLine; viewLine <= nLastViewLine; viewLine++)
    {
        viewdata lineData = pwndFirst->GetViewData(viewLine);
        lineData.ending = lineendings;
        lineData.state = ResolveState(lineData.state);
        SetViewData(viewLine, lineData);
        if (!IsStateEmpty(pwndFirst->GetViewState(viewLine)))
        {
            pwndFirst->SetViewState(viewLine, DIFFSTATE_YOURSADDED); // this is improper (may be DIFFSTATE_THEIRSADDED) but seems not to produce any visible bug
        }
    }
    SaveUndoStep();

    // use (insert) last block
    int nViewIndex = nNextViewLine;
    for (int viewLine = nFirstViewLine; viewLine <= nLastViewLine; viewLine++, nViewIndex++)
    {
        viewdata lineData = pwndLast->GetViewData(viewLine);
        lineData.state = ResolveState(lineData.state);
        InsertViewData(nViewIndex, lineData);
        if (!IsStateEmpty(pwndLast->GetViewState(viewLine)))
        {
            pwndLast->SetViewState(viewLine, DIFFSTATE_THEIRSADDED); // this is improper but seems not to produce any visible bug
        }
    }
    SaveUndoStep();

    // adjust line numbers in target
    // we fix all line numbers to handle exotic cases
    UpdateViewLineNumbers();
    SaveUndoStep();

    // now insert an empty block in both first and last
    int nCount = nLastViewLine - nFirstViewLine + 1;
    pwndLast->InsertViewEmptyLines(nFirstViewLine, nCount);
    pwndFirst->InsertViewEmptyLines(nNextViewLine, nCount);
    SaveUndoStep();

    int nRemovedLines = CleanEmptyLines();
    SaveUndoStep();	

    CUndo::GetInstance().EndGrouping();

    // final clean up
    ClearSelection();
    SetupAllViewSelection(nFirstViewLine, 2*nLastViewLine - nFirstViewLine - nRemovedLines + 1);
    BuildAllScreen2ViewVector();
    SetModified();
    pwndLast->SetModified();
    pwndFirst->SetModified();
    RefreshViews();
}

void CBottomView::UseViewBlock(CBaseView * pwndView)
{
    int nFirstViewLine; // first view line in selection
    int nLastViewLine; // last view line in selection

    if (!GetViewSelection(nFirstViewLine, nLastViewLine))
        return;

    return UseBlock(pwndView, nFirstViewLine, nLastViewLine);
}

void CBottomView::UseViewFile(CBaseView * pwndView)
{
    int nFirstViewLine = 0;
    int nLastViewLine = GetViewCount()-1;

    return UseBlock(pwndView, nFirstViewLine, nLastViewLine);
}
