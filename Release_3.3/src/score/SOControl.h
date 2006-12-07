//--------------------------------------------------------------------------------------
//    LenMus Phonascus: The teacher of music
//    Copyright (c) 2002-2006 Cecilio Salmeron
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
/*! @file SOControl.h
    @brief Header file for class lmSOControl
    @ingroup score_kernel
*/
#ifdef __GNUG__
// #pragma interface
#endif

#ifndef __SOCONTROL_H__        //to avoid nested includes
#define __SOCONTROL_H__

enum ESOCtrolType
{
    lmTIME_SHIFT = 1,       // forward / backwards
    lmNEW_SYSTEM,           // force a new system
};

class lmSOControl:  public lmSimpleObj
{
public:
    //constructors and destructor
    lmSOControl(ESOCtrolType nType, lmVStaff* pVStaff, float rTimeShift);   //lmTIME_SHIFT
    lmSOControl(ESOCtrolType nType, lmVStaff* pVStaff);                     //lmNEW_SYSTEM
    ~lmSOControl() {}

    //implementation of virtual methods defined in abstract base class lmStaffObj
    void DrawObject(bool fMeasuring, lmPaper* pPaper, wxColour colorC, bool fHighlight) {};
    wxBitmap* GetBitmap(double rScale) { return (wxBitmap*)NULL; };

    //SOControl specfic methods
    ESOCtrolType GetCtrolType() { return m_nCtrolType; }

    //    debugging
    wxString Dump();
    wxString SourceLDP();
    wxString SourceXML();


private:
    ESOCtrolType    m_nCtrolType;
    float           m_rTimeShift;        //the time shift (positive or negative) applied

};

#endif    // __SOCONTROL_H__