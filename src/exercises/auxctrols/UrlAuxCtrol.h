//--------------------------------------------------------------------------------------
//    LenMus Phonascus: The teacher of music
//    Copyright (c) 2002-2009 Cecilio Salmeron
//
//    This program is free software; you can redistribute it and/or modify it under the
//    terms of the GNU General Public License as published by the Free Software Foundation,
//    either version 3 of the License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//    PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along with this
//    program. If not, see <http://www.gnu.org/licenses/>.
//
//    For any comment, suggestion or feature request, please contact the manager of
//    the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LM_URLAUXCTROL_H__        //to avoid nested includes
#define __LM_URLAUXCTROL_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "UrlAuxCtrol.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

DECLARE_EVENT_TYPE(lmEVT_URL_CLICK, 7777)        // any number is OK. It doesn' matter

// it is convenient to define an event table macro for this event type
#define LM_EVT_URL_CLICK(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        lmEVT_URL_CLICK, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),


class lmUrlAuxCtrol : public wxStaticText
{
       DECLARE_DYNAMIC_CLASS(lmUrlAuxCtrol)

public:

    // constructor and destructor
    lmUrlAuxCtrol(wxWindow* parent, wxWindowID id, double rScale,
                const wxString& sNormalLabel, const wxString& sAltLabel = _T("??????"),
                const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                long style = 0, const wxString& name = _T("lmUrlAuxCtrol"));

    ~lmUrlAuxCtrol() {}

    // event handlers
    void OnClick(wxMouseEvent& event);

    // operations
    void SetNormalLabel() { SetLabel(m_sNormalLabel); }
    void SetAlternativeLabel() { SetLabel(m_sAltLabel); }

private:
    wxString    m_sNormalLabel;     //Label displayed normally (i.e. "play")
    wxString    m_sAltLabel;        //Alternative label (i.e. "stop playing")

    DECLARE_EVENT_TABLE()
};


#endif  // __LM_URLAUXCTROL_H__