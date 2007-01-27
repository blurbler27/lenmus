//--------------------------------------------------------------------------------------
//    LenMus Phonascus: The teacher of music
//    Copyright (c) 2002-2007 Cecilio Salmeron
//
//    This program is free software; you can redistribute it and/or modify it under the 
//    terms of the GNU General Public License as published by the Free Software Foundation;
//    either version 2 of the License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but WITHOUT ANY 
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
//    PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along with this 
//    program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, 
//    Fifth Floor, Boston, MA  02110-1301, USA.
//
//    For any comment, suggestion or feature request, please contact the manager of 
//    the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifdef __GNUG__
// #pragma implementation
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "../score/score.h"
#include "ScoreAuxCtrol.h"
#include "../app/DlgDebug.h"

// lmScoreAuxCtrol
//	  A window on which a music score is rendered.
//
//    ScoreControl is a window on which a music score is drawn. No direct interaction
//    with the score is allowed, that is, it is is just a display control.
//
//    It combines the behaviour of classes lmScoreCanvas, lmScoreView and Doc, so that scores
//    can be displayed/printed without having to use the doc/view model.


BEGIN_EVENT_TABLE(lmScoreAuxCtrol, wxWindow)
    EVT_PAINT(lmScoreAuxCtrol::OnPaint)
    EVT_SIZE(lmScoreAuxCtrol::OnSize)
    LM_EVT_SCORE_HIGHLIGHT(lmScoreAuxCtrol::OnVisualHighlight)
END_EVENT_TABLE()


lmScoreAuxCtrol::lmScoreAuxCtrol(wxWindow* parent, wxWindowID id, lmScore* pScore,
                                 const wxPoint& pos, 
                                 const wxSize& size, EScoreStyles style) :
    wxWindow(parent, id, pos, size, style)
{
    SetBackgroundColour(*wxWHITE);
    m_pScore = pScore;
    m_sMsg = wxEmptyString;
    m_fDisplayMessage = false;

    SetMargins(lmToLogicalUnits(10, lmMILLIMETERS),
               lmToLogicalUnits(10, lmMILLIMETERS),
               lmToLogicalUnits(10, lmMILLIMETERS));    //right=1cm, left=1cm, top=1cm
    SetScale(1.0);

    m_fHidden = false;

}

lmScoreAuxCtrol::~lmScoreAuxCtrol()
{
}

void lmScoreAuxCtrol::SetMargins(lmLUnits nLeft, lmLUnits nRight, lmLUnits nTop)
{
    m_nTopMargin = nTop;
    m_nLeftMargin = nLeft;
    m_nRightMargin = nRight;
}

void lmScoreAuxCtrol::SetScale(float rScale)
{
    //Scale is the zooming factor
    m_rZoom = rScale;
    m_rScale = rScale * lmSCALE;
    //wxLogMessage(_T("[lmScoreAuxCtrol::SetScale]rScale=%f, lmSCALE=%f, m_rScale=%f"), rScale, lmSCALE, m_rScale);
    ResizePaper();
}

void lmScoreAuxCtrol::ResizePaper()
{
    //adjust paper size to match window size
    wxClientDC dc(this);
    wxCoord xPixels, yPixels;
    dc.GetSize(&xPixels, &yPixels);
    dc.SetMapMode(lmDC_MODE);
    dc.SetUserScale( m_rScale, m_rScale );
    lmLUnits xLU = (lmLUnits)dc.DeviceToLogicalXRel(xPixels);
    lmLUnits yLU = (lmLUnits)dc.DeviceToLogicalYRel(yPixels);
    m_Paper.SetPageSize(xLU, yLU);

    //save new DC scaling factor to be used later for message positioning
    m_yScalingFactor =(float)yPixels / (float)yLU;

    m_Paper.SetPageTopMargin(m_nTopMargin);
    m_Paper.SetPageLeftMargin(m_nLeftMargin);
    m_Paper.SetPageRightMargin(m_nRightMargin);

    //wxLogMessage(_T("[lmScoreAuxCtrol::ResizePaper] :\n")
    //    _T("Paper size: px = (%d, %d), LU= (%.2f, %.2f)\n")
    //    _T("m_rScale=%f, scaling factor=%f, margins: left=%.2f, right=%.2f, top=%.2f"),
    //    xPixels, yPixels,
    //    xLU, yLU, m_rScale, m_yScalingFactor,
    //    m_nLeftMargin, m_nRightMargin, m_nTopMargin );
}

double lmScoreAuxCtrol::GetPixelsPerLU()
{
    wxClientDC dc(this);
    dc.SetMapMode(lmDC_MODE);
    dc.SetUserScale( m_rScale, m_rScale );

    double xPixelsPerLU = (double)dc.LogicalToDeviceXRel(100000) / 100000.0;
    double yPixelsPerLU = (double)dc.LogicalToDeviceYRel(100000) / 100000.0;
    wxLogMessage(_T("[lmScoreAuxCtrol::GetPixelsPerLU] m_rScale=%f, DisplayPixelsPerLU=(%f, %f)"),
        m_rScale, xPixelsPerLU, yPixelsPerLU );

    // screen resolution (Pixels per inch)
    wxSize sizePPI = dc.GetPPI(); 
    // control size, in mm and pixels
    wxCoord widthMM, heightMM, widthPx, heightPx;
    dc.GetSizeMM(&widthMM, &heightMM);       // mm
    dc.GetSize(&widthPx, &heightPx);         // pixels
    SetBaseScale();

    // In order to adjust staff lines to real size I will use only yPixelsPerLU
    return yPixelsPerLU;

}

void lmScoreAuxCtrol::SetBaseScale()
{
    //compute scaling factor needed for real display size (real scale 1:1)

    wxClientDC dc(this);
    dc.SetMapMode(lmDC_MODE);
    dc.SetUserScale( m_rScale, m_rScale );

    // screen resolution (Pixels per inch)
    wxSize sizePPI = dc.GetPPI(); 

    // control size, in mm and pixels
    wxCoord widthMM, heightMM, widthPx, heightPx;
    dc.GetSizeMM(&widthMM, &heightMM);       // mm
    dc.GetSize(&widthPx, &heightPx);         // pixels
    //lmLUnits oneMM = lmToLogicalUnits(1, lmMILLIMETERS);
    //double yTargetPixelsPerLU = oneMM * heightPx / heightMM;

    int wScreenMM, hScreenMM, wScreenPx, hScreenPx;
    ::wxDisplaySize(&wScreenPx, &hScreenPx);
    ::wxDisplaySizeMM(&wScreenMM, &hScreenMM);

    // current internal DC scale is 1.0
    double yCurPixelsPerLU = (double)dc.LogicalToDeviceYRel(100000) / 100000.0;
    //m_rBaseScale = yTargetPixelsPerLU * yCurPixelsPerLU;

    wxLogMessage(_T("[lmScoreAuxCtrol::SetBaseScale] : \n")
                _T("PPI=(%d, %d) / control size px=(%d, %d), mm=(%d, %d) \n")
                _T("display size px=(%d, %d), mm=(%d, %d) \n")
                _T("yCurPixelsPerLU = %f \n")
                _T("Computed values for screen size:\n")
                _T("     using PPI: mm = (%.2f, %.2f)\n")
                _T("     using control:  mm = (%.2f, %.2f)\n")
                _T("     using display:  mm = (%d, %d)\n"),
                sizePPI.GetWidth(), sizePPI.GetHeight(), widthPx, heightPx, widthMM, heightMM, 
                wScreenPx, hScreenPx, wScreenMM, hScreenMM,
                yCurPixelsPerLU,
                25.4 * (double)wScreenPx / (double)sizePPI.GetWidth(), 25.4 * (double)hScreenPx / (double)sizePPI.GetHeight(),
                (double)wScreenPx * (double)widthMM / (double)widthPx, (double)wScreenPx * (double)heightMM / (double)heightPx,
                wScreenMM, hScreenMM );
}



void lmScoreAuxCtrol::OnSize(wxSizeEvent& WXUNUSED(event))
{
    //The window has been resized. Adjust paper size and scale

    ////DBG ---------------------------------------------------------------------
    //int xPixels, yPixels;
    //wxWindow::GetClientSize(&xPixels, &yPixels);
    //wxLogMessage(wxString::Format(
    //    _T("[lmScoreAuxCtrol::OnSize]New window size = (%d, %d)"), xPixels, yPixels));
    ////END DBG -----------------------------------------------------------------

    ResizePaper();
    Refresh();
}


// Repainting behaviour
void lmScoreAuxCtrol::OnPaint(wxPaintEvent &WXUNUSED(event))
{
    bool fClear = true;

    // In a paint event handler, the application must always create a wxPaintDC object, 
    // even if it is not used. Otherwise, under MS Windows, refreshing for this and 
    // other windows will go wrong.
    wxPaintDC dc(this);

    if (m_pScore && !m_fHidden) {
        // Get size of window
        int dxBitmap, dyBitmap;
        dc.GetSize(&dxBitmap, &dyBitmap);        // size of the DC in pixels

        //wxLogMessage(wxString::Format(
        //    _T("[lmScoreAuxCtrol::OnPaint]dxyBitmap = (%d, %d)"), dxBitmap, dyBitmap));

        // allocate a DC in memory for using the offscreen bitmaps
        wxMemoryDC memoryDC;
        m_Paper.SetDrawer(new lmDirectDrawer(&memoryDC));
        m_graphMngr.Prepare(m_pScore, dxBitmap, dyBitmap, m_rScale, &m_Paper,
                            lmFORCE_RELAYOUT);
        wxBitmap* pPageBitmap = m_graphMngr.Render(lmUSE_BITMAPS, 1);

        wxASSERT(pPageBitmap && pPageBitmap->Ok());
        memoryDC.SelectObject(*pPageBitmap);

        // Copy the offscreen bitmap onto the DC
        dc.Blit(0, 0, dxBitmap, dyBitmap, &memoryDC, 0, 0);

        // deselect the las bitmap
        memoryDC.SelectObject(wxNullBitmap);
        fClear = false;
    }

    if (m_fDisplayMessage) {
        fClear = false;
        dc.DrawText(m_sMsg,
                    lmToLogicalUnits(5, lmMILLIMETERS) * m_yScalingFactor,
                    m_yMsg * m_yScalingFactor);
    }

    if (fClear) {
        dc.Clear();
    }

}

void lmScoreAuxCtrol::DisplayMessage(wxString sMsg, lmLUnits posMsg, bool fClearScore)
{
    if (m_pScore && !m_fHidden && fClearScore) {
        m_pScore = (lmScore*)NULL;
    }
    m_sMsg = sMsg;
    m_yMsg = posMsg;
    m_fDisplayMessage = true;
    Refresh();
}

void lmScoreAuxCtrol::DisplayScore(lmScore* pScore, bool fClearMessage)
{
    //wxLogMessage(_T("[lmScoreAuxCtrol::DisplayScore] ScoreID=%d"), pScore->GetID() );
    SetScore(pScore, false);        //false: not hidden
    if (fClearMessage) m_fDisplayMessage = false;
    Refresh();
}

void lmScoreAuxCtrol::SetScore(lmScore* pScore, bool fHidden)
{
    m_fHidden = fHidden;
    m_pScore = pScore;
}

void lmScoreAuxCtrol::HideScore(bool fHide)
{
    m_fHidden = fHide;
    Refresh();
}

void lmScoreAuxCtrol::PlayScore(bool fVisualTracking, bool fMarcarCompasPrevio,
        EPlayMode nPlayMode, long nMM)
{
    if (m_pScore) {
        m_pScore->Play(fVisualTracking && !m_fHidden, fMarcarCompasPrevio, nPlayMode, nMM, this);
    }
}

void lmScoreAuxCtrol::PlayMeasure(int nMeasure, bool fVisualTracking, EPlayMode nPlayMode,
        long nMM)
{
    if (m_pScore) {
        m_pScore->PlayMeasure(nMeasure, fVisualTracking && !m_fHidden, nPlayMode, nMM, this);
    }
}

void lmScoreAuxCtrol::Stop()
{
    if (m_pScore) {
        m_pScore->Stop();
    }
}

void lmScoreAuxCtrol::Pause()
{
    if (m_pScore) {
        m_pScore->Pause();
    }
}

void lmScoreAuxCtrol::OnVisualHighlight(lmScoreHighlightEvent& event)
{
    if (!m_pScore) return;

    EHighlightType nHighlightType = event.GetHighlightType();
    if (nHighlightType == ePrepareForHighlight) {
        m_graphMngr.PrepareForHighlight();
        return;
    }

    //prepare paper DC
    wxClientDC dc(this);
    dc.SetMapMode(lmDC_MODE);
    dc.SetUserScale( m_rScale, m_rScale );
    //m_Paper.SetDC(&dc);
    m_Paper.SetDrawer(new lmDirectDrawer(&dc));

    /*! @todo
        Position DC origing according to current scrolling and page position
        For now it is assumed that all score is displayed
    */

    //do the highlight / unhighlight
    lmStaffObj* pSO = event.GetStaffObj();
    m_pScore->ScoreHighlight(pSO, &m_Paper, nHighlightType);

}


    //
    // Debug methods
    //

void lmScoreAuxCtrol::Dump()
{
    if (!m_pScore) return;
    lmDlgDebug dlg(this, _T("lmStaff objects dump"), m_pScore->Dump());
    dlg.ShowModal();

}

void lmScoreAuxCtrol::SourceLDP()
{
    if (!m_pScore) return;
    lmDlgDebug dlg(this, _T("Generated source code"), m_pScore->SourceLDP());
    dlg.ShowModal();

}

void lmScoreAuxCtrol::SourceXML()
{
    if (!m_pScore) return;
    lmDlgDebug dlg(this, _T("Generated MusicXML code"), m_pScore->SourceXML());
    dlg.ShowModal();

}

void lmScoreAuxCtrol::DumpMidiEvents()
{
    if (!m_pScore) return;
    lmDlgDebug dlg(this, _T("MIDI events table"), m_pScore->DumpMidiEvents() );
    dlg.ShowModal();

}
