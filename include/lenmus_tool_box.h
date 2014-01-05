//---------------------------------------------------------------------------------------
//    LenMus Phonascus: The teacher of music
//    Copyright (c) 2002-2014 LenMus project
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
//---------------------------------------------------------------------------------------

#ifndef __LENMUS_TOOL_BOX_H__        //to avoid nested includes
#define __LENMUS_TOOL_BOX_H__

//lenmus
#include "lenmus_tool_box_theme.h"
#include "lenmus_tool_group.h"
#include "lenmus_injectors.h"
#include "lenmus_events.h"

//wxWidgets
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
class wxBoxSizer;

//lomse
#include <lomse_selections.h>
#include <lomse_document_cursor.h>
#include <lomse_pitch.h>                //enum EAccidentals
#include <lomse_im_note.h>              //enum ENoteHead
using namespace lomse;

//other
#include <vector>
using namespace std;


namespace lenmus
{

class CheckButton;
class ToolPageNotes;
class ToolPage;

//---------------------------------------------------------------------------------------
//available tool pages
enum EToolPageID
{
	k_page_none = -1,
	k_page_clefs = 0,
	k_page_notes,
	k_page_barlines,
    k_page_symbols,
    k_page_rhythmic_dictation,
    //TO_ADD: Add, before this line, a new k_page_xxxxxx code for the new page
	k_page_max,     //this one MUST BE the last one
};

//---------------------------------------------------------------------------------------
// Group for mouse mode
//---------------------------------------------------------------------------------------

//enum to define function/tool assigned to the mouse
enum EMouseMode
{
    k_mouse_mode_undefined=0,
    k_mouse_mode_pointer,       //pointer mode: select, drag
    k_mouse_mode_data_entry,    //i.e.: insert note
};

//forward declarations
class ToolBox;

//---------------------------------------------------------------------------------------
// ToolsInfo
//      responsible for providing info about toolbox current selections
class ToolsInfo
{
private:
    //current values selected in ToolBox
	ToolBox*      m_pToolBox;

public:
//    EToolPageID   pageID;
//    EToolGroupID  groupID;
    EToolID       toolID;

    //current options for notes (page Notes)
    ENoteType     noteType;
	int           dots;
	ENoteHeads    notehead;
	EAccidentals  acc;
	int           octave;
	int           voice;
    bool          fIsNote;

    //current options for clef, key, barline and time signature
    EClef         clefType;
    EBarline      barlineType;
    EKeySignature  keyType;
    int     timeBeatType;
    int     timeNumBeats;

    //mouse mode
    int     mouseMode;
    int     clickCmd;       //command associated to a mouse click when drag mode

//    //to save options selected by user in ToolBox
//    bool            m_fToolBoxSavedOptions;
//    int             m_nTbAcc;
//    int             m_nTbDots;
//    int             m_nTbDuration;

public:
    ToolsInfo();
    ~ToolsInfo() {}

    //get current toolbox selections: page, group, tool and mouse mode
    void update_toolbox_info(ToolBox* pToolBox);

    //enable/disable toolbox
    void enable_tools(bool fEnable);

    //access to ToolBox values
    inline bool is_mouse_data_entry_mode() { return mouseMode == k_mouse_mode_data_entry; }


protected:
    void get_toolbox_info();

};

//---------------------------------------------------------------------------------------
class GrpMouseMode : public ToolButtonsGroup
{
public:
    GrpMouseMode(wxPanel* pParent, wxBoxSizer* pMainSizer, ToolboxTheme* pColours);
    ~GrpMouseMode() {}

    //mandatory overrides
    void update_tools_info(ToolsInfo* pInfo);
    void create_controls_in_group(wxBoxSizer* pMainSizer);
    EToolGroupID get_group_id() { return k_grp_MouseMode; }
    void synchronize_with_cursor(bool fEnable, DocCursor* pCursor) {}
    void synchronize_with_selection(bool fEnable, SelectionSet* pSelection) {}

	//access to options
	int get_mouse_mode();
	void SetMouseMode(int nMouseMode);

};


//---------------------------------------------------------------------------------------
// ToolBoxConfiguration: Data defining ToolBox state and available options
//---------------------------------------------------------------------------------------
class ToolBoxConfiguration
{
protected:

    friend class ToolBox;       //to avoid defining accessors

    //optional blocks that are visible
    bool m_fMouseModeVisible;
    bool m_fPageSelectorsVisible;

    //pages that can be activated
    int m_fIsPageActivable[k_page_max];

    //current active page
    EToolPageID m_activePage;

    //the state of current active page

    //the state of mouse group, if visible


public:
    ToolBoxConfiguration()
        : m_fMouseModeVisible(true)
        , m_fPageSelectorsVisible(false)
        , m_activePage(k_page_none)

    {
        for (int i=0; i < k_page_max; ++i)
            m_fIsPageActivable[i] = true;
    }

    ~ToolBoxConfiguration() {}

    //configuration
    inline ToolBoxConfiguration& mouse_mode_selector(bool value)
    {
        m_fMouseModeVisible = value;
        return *this;
    }

    inline ToolBoxConfiguration& page_selectors(bool value)
    {
        m_fPageSelectorsVisible = value;
        return *this;
    }

    inline ToolBoxConfiguration& active_page(int pageID)
    {
        m_activePage = EToolPageID(pageID);
        return *this;
    }

    inline ToolBoxConfiguration& disable_page(int pageID)
    {
        m_fIsPageActivable[pageID] = false;
        return *this;
    }
};


//---------------------------------------------------------------------------------------
// ToolBox
//---------------------------------------------------------------------------------------

class ToolBox : public wxPanel
{
   DECLARE_DYNAMIC_CLASS(ToolBox)

protected:

	enum {
		NUM_BUTTONS = 16,
	};

    ApplicationScope& m_appScope;

    //controls
    GrpMouseMode*   m_pMouseModeGroup;      //mouse mode group
	wxPanel*        m_pSelectPanel;         //page selector block
    wxPanel*		m_pEmptyPage;           //an empty page
    wxPanel*		m_pCurPage;             //currently displayed page
    wxBoxSizer*     m_pPageSizer;           //the sizer for the pages
    EToolPageID     m_nCurPageID;           //currently displayed page ID
	CheckButton*	m_pButton[NUM_BUTTONS];

	//tool pages
	vector<ToolPage*>   m_cPages;                       // defined pages
	bool                m_fIsPageActivable[k_page_max]; // true if page can be used

	ToolboxTheme	m_colors;               //colors to use in this toolbox

public:
    ToolBox(wxWindow* parent, wxWindowID id, ApplicationScope& appScope);
    virtual ~ToolBox();

    //enable/disable toolbox
    void enable_tools(bool fEnable);

    //event handlers
    void OnButtonClicked(wxCommandEvent& event);
    void OnResize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void on_update_UI(lmUpdateUIEvent& event);

    //configuration
    void set_mode(int mode);
    void save_configuration(ToolBoxConfiguration* pConfig);
    void load_configuration(const ToolBoxConfiguration& config);
    void synchronize_tools(SelectionSet* pSelection, DocCursor* pCursor);

	//info
	int GetWidth() { return 150; }      //in pixels
	inline ToolboxTheme* GetColors() { return &m_colors; }
    inline ApplicationScope& get_app_scope() { return m_appScope; }

	//current tool and its options
    EToolGroupID GetCurrentGroupID();
    EToolID get_selected_tool_id();
//    wxString GetToolShortDescription();
    void update_tools_info(ToolsInfo* pInfo);


    ToolPage* get_selected_page() { return (ToolPage*)m_pCurPage; }
	void SelectToolPage(EToolPageID nPageID);

//    //Contextual menus
//    wxMenu* GetContextualMenuForSelectedPage();
//    void OnPopUpMenuEvent(wxCommandEvent& event);

    //operations
    void AddPage(ToolPage* pPage, int nPageID);
    void mark_page_as_activable(int nPageID);
    bool process_key(wxKeyEvent& event);
    int translate_key(int key, unsigned keyFlags);

    //interface with mouse mode group
	int get_mouse_mode();
    inline void SetMouseMode(int nMouseMode) { m_pMouseModeGroup->SetMouseMode(nMouseMode); }


private:
	void CreateControls();
	void SelectButton(int nTool);
    ToolPage* CreatePage(EToolPageID nPageID);
	inline EToolPageID get_selected_page_id() const { return m_nCurPageID; }

    void synchronize_with_cursor(bool fEnable, DocCursor* pCursor=NULL);
    void synchronize_with_selection(bool fEnable, SelectionSet* pSelection=NULL);

    void enable_mouse_mode_buttons(bool fEnable);
    void enable_current_page(bool fEnable);
    void enable_page_selectors(bool fEnable);

    DECLARE_EVENT_TABLE()
};



}   // namespace lenmus

#endif    // __LENMUS_TOOL_BOX_H__