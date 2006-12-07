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
/*! @file Note.cpp
    @brief Implementation file for class lmNote
    @ingroup score_kernel
*/
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

#include "wx/image.h"
#include "Score.h"
#include "wx/debug.h"
#include "../ldp_parser/AuxString.h"

#include "Glyph.h"

//implementation of the Notes List
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(NotesList);

const bool m_fDrawSmallNotesInBlock = false;    //! @todo option depending on used font

lmNote::lmNote(lmVStaff* pVStaff, bool fAbsolutePitch,
        wxString sStep, wxString sOctave, wxString sAlter,
        EAccidentals nAccidentals,
        ENoteType nNoteType, float rDuration,
        bool fDotted, bool fDoubleDotted,
        int nStaff,
        lmContext* pContext, 
        bool fBeamed, lmTBeamInfo BeamInfo[],
        bool fInChord,
        bool fTie,
        EStemType nStem)  : 
    lmNoteRest(pVStaff, DEFINE_NOTE, nNoteType, rDuration, fDotted, fDoubleDotted, nStaff)
{
    /*
    Diatonic Pitch is determined by Step
    This constructor is used for two kind of pitch information:
      -    MusicXML style: sound and look information totally independent; pitch is absolute, and
        is implied by combining Step, Octave and Alter. Accidentals are just for rendering.
      - LDP style: notes defined as in hand writing; pitch depends on context (key signature
        and accidentals introduced by a previous note in the same measure) and its own
        accidentals. Therefore pitch is computred from Step, Octave, Accidentals and Context.
        Alter is not used.
    To support both kinds of definitions, parameter fAbsolutePitch indicates that pitch
    information is absolute (MusicXML style) or relative (LDP style).
    */

//        nCalderon As ECalderon, _
//        cAnotaciones As Collection, _
//        ByRef anContext() As Long, _
//        fCabezaX As Boolean, _
//        nVoz As Long, _

    //shapes initialization
    m_pNoteheadShape = (lmShapeGlyph*)NULL;
    m_pStemLine = (lmShapeLine*)NULL;
    m_pFlagShape = (lmShapeGlyph*)NULL;
    
    // stem information
    m_xStem = 0;            // will be updated in lmNote::DrawObject, in measurement phase
    m_yStem = 0;            // will be updated in lmNote::DrawObject, in measurement phase
    m_nStemType = nStem;    
    m_nStemLength = GetDefaultStemLength();     //default. If beamed note, this value will
                                                //be updated in lmBeam::ComputeStemsDirection
    ////DBG
    //if (GetID() == 13) {
    //    // break here
    //    int nDbg = 0;
    //}

    // accidentals
    if (nAccidentals == eNoAccidentals) {
        m_pAccidentals = (lmAccidental*)NULL;
    } else {
        m_pAccidentals = new lmAccidental(this, nAccidentals);
    }

    //context information
    m_pContext = pContext;
    m_pContext->StartUsing();
    m_nClef = (m_pContext->GetClef())->GetType();
//    m_nVoz = nVoz;        //! @todo parameter nVoz
    //for (int i=0; i < 7; i++) {
    //    m_anContext[i] = 0;    //! @todo parameter anContext[i]
    //}
    
    //get context accidentals for this note step
    m_nStep = LetterToStep(sStep);
    int nNewAlter = m_pContext->GetAccidentals(m_nStep);

    //update context with displayed accidentals or with alterations
    if (fAbsolutePitch) {
        //update context with alterations
    }
    else {
        //update context with accidentals
        switch (nAccidentals) {
            case eNoAccidentals:
                //do not modify context
                break;
            case eNatural:
                nNewAlter = 0;    //! @todo review: shouldn't be the accidentals of the key signature?
                break;
            case eFlat:
                nNewAlter = -1;
                break;
            case eSharp:
                nNewAlter = 1;
                break;
            case eFlatFlat:
                nNewAlter = -2;
                break;
            case eSharpSharp: 
            case eDoubleSharp:
                nNewAlter = 2;
                break;
            case eNaturalFlat:
                nNewAlter = 1;
                break;
            case eNaturalSharp:
                nNewAlter = -1;
                break;
            default:
                ;
        }
    }

    // if needed, update context for following notes
    if (nNewAlter != m_pContext->GetAccidentals(m_nStep)) {
        m_pVStaff->UpdateContext(this, nStaff, m_nStep, nNewAlter, m_pContext);
    }

    //store all pitch related information
    long nAux;
    sOctave.ToLong(&nAux);
    m_nOctave = (int)nAux;
    sAlter.ToLong(&nAux);
    m_nAlter = (fAbsolutePitch ? (int)nAux : nNewAlter);
    m_nPitch = StepAndOctaveToPitch(m_nStep, m_nOctave);
    m_nMidiPitch = PitchToMidi(m_nPitch, m_nAlter);

    SetUpStemDirection();       // and standard lenght

//    Set m_cPentobjs = new Collection
//    
//    //Analiza los flags
//    m_fCalderon = (nCalderon = eC_ConCalderon)
    
//    //analiza las anotaciones
//    Dim sDato As String, oGrafObj As CPOGrafObj
//    for (i = 1 To cAnotaciones.Count
//        sDato = cAnotaciones.Item(i)
//        switch (sDato) {
//            case "AMR":     //marca de pausa
//                Set oGrafObj = new CPOGrafObj
//                oGrafObj.ConstructorGrafObj eGO_Respiracion, oStaff, true
//                m_cPentobjs.Add oGrafObj
//            default:
//                Debug.Assert false
//        }
//    }
    
    //if the note is part of a chord find the base note and take some values from it
    m_fIsNoteBase = true;        //by default all notes are base notes
    m_pChord = (lmChord*)NULL;    //by defaul note is not in chord
    m_fNoteheadReversed = false;
    if (fInChord) {
        if (!g_pLastNoteRest || g_pLastNoteRest->IsRest()) {
            ; //! @todo
            //MsgBox "[lmNoteRest.ConstructorNota] Se pide nota en acorde pero no hay ninguna nota" & _
            //    " base definida. Se ignora acorde"
        }
        else {
            m_fIsNoteBase = false;
            lmNote* pLastNote = (lmNote*)g_pLastNoteRest;
            if (pLastNote->IsInChord()) {
                m_pChord = pLastNote->GetChord();
            } else {
                m_pChord = pLastNote->StartChord();
            }
            m_pChord->AddNote(this);
        }
    }
    
    
    //lmTie information ----------------------------------------------------
    m_fNeedToBeTied = fTie;
    m_pTiePrev = (lmTie*)NULL;        //assume no tied
    m_pTieNext = (lmTie*)NULL;        //assume no tied

    // verify if a previous note is tied to this one and if so, build the tie
    lmNote* pNtPrev = m_pVStaff->FindPossibleStartOfTie(m_nMidiPitch, m_nStep);
    if (pNtPrev && pNtPrev->NeedToBeTied()) {

        //do the tie between previous note and this one
        m_pTiePrev = new lmTie(pNtPrev, this);
        pNtPrev->SetTie(m_pTiePrev);
        
        //if stem direction is not forced, choose it to be as that of the start of tie note
        if (nStem == eDefaultStem)
            m_fStemDown = pNtPrev->StemGoesDown();
    }
        
//    //Par�metros gr�ficos independientes del papel.
////    m_rEspacioPost = 0#     //ya se calcular�
//    m_fCabezaX = fCabezaX

    
    // Generate beaming information -----------------------------------------------------
    CreateBeam(fBeamed, BeamInfo);
    
    
    if (!IsInChord() || IsBaseOfChord()) g_pLastNoteRest = this;

    //initializations for renderization
    m_nSpacePrev = 0;

}

lmNote::~lmNote()
{
    //remove the note from the beam and if beam is empty delete the beam
    if (m_pBeam) {
        m_pBeam->Remove(this);
        if (m_pBeam->NumNotes() == 0) {
            delete m_pBeam;
            m_pBeam = (lmBeam*)NULL;
        }
    }

    //if note in chord, remove it from the chord. Delete the chord when only one note left in it.
    if (IsInChord()) {
        m_pChord->RemoveNote(this);
        if (m_pChord->GetNumNotes() == 1) {
            delete m_pChord;
            m_pChord = (lmChord*)NULL;
        }
    }

    //delete the Ties.
    if (m_pTiePrev) {
        m_pTiePrev->Remove(this);
        delete m_pTiePrev;
    }
    m_pTiePrev = (lmTie*)NULL;
    if (m_pTieNext) {
        m_pTieNext->Remove(this);
        delete m_pTieNext;
    }
    m_pTieNext = (lmTie*)NULL;

    //delete accidentals
    if (m_pAccidentals) {
        delete m_pAccidentals;
        m_pAccidentals = (lmAccidental*)NULL;
    }

    //delete the shapes
    if (m_pNoteheadShape) {
        delete m_pNoteheadShape;
        m_pNoteheadShape = (lmShapeGlyph*)NULL;
    }
    if (m_pStemLine) {
        delete m_pStemLine;
        m_pStemLine = (lmShapeLine*)NULL;
    }
    if (m_pFlagShape) {
        delete m_pFlagShape;
        m_pFlagShape = (lmShapeGlyph*)NULL;
    }

}

bool lmNote::UpdateContext(int nStep, int nNewAccidentals, lmContext* pNewContext)
{
    /*
    A previous note has updated the contex. It is necessary to verify if this note is
    affected and, if it is affected, to update context dependent information.
    Returns true if no context modification was needed.
    */

    //get context accidentals for the modified step
    int nAccidentals = m_pContext->GetAccidentals(nStep);

    //compare context accidentals for modified step with new accidentals
    if (nAccidentals == nNewAccidentals) {
        //are equal. No need to update context for this note. 
        //Verify if this note alters current context
        //! @todo
        return true;
    }
    else {
        //are different. Update contex information
        m_pContext->StopUsing();    //inform old context that is not longer used by this note
        m_pContext = pNewContext;    //update context for this note
        m_pContext->StartUsing();    //and inform new context that it is being used by this note
        //update pitch
        m_nAlter = nNewAccidentals;
        m_nMidiPitch = PitchToMidi(m_nPitch, m_nAlter);
        return false;
    }

}

void lmNote::RemoveTie(lmTie* pTie)
{
    if (m_pTiePrev == pTie) {
        m_pTiePrev = (lmTie*)NULL;
    }
    else if (m_pTieNext == pTie) {
        m_pTieNext = (lmTie*)NULL;
    }
}

void lmNote::ClearChordInformation()
{
    // invoked only from lmChord destructor
    m_fIsNoteBase = true;        //by default all notes are base notes
    m_pChord = (lmChord*)NULL;    //by defaul note is not in chord
}


//====================================================================================================
// implementation of virtual methods defined in base abstract class lmNoteRest
//====================================================================================================

// Create the drag image.
// Under wxGTK the DC logical function (ROP) is used by DrawText() but it is ignored by
// wxMSW. Thus, it is not posible to do dragging just by redrawing the lmStaffObj using ROPs.
// For portability it is necessary to implement dragging by means of bitmaps and wxDragImage
wxBitmap* lmNote::GetBitmap(double rScale)
{

    lmEGlyphIndex nGlyph = GLYPH_EIGHTH_NOTE_UP;
    switch (m_nNoteType) {
        case eEighth :
            nGlyph = (m_fStemDown ? GLYPH_EIGHTH_NOTE_DOWN : GLYPH_EIGHTH_NOTE_UP);
            break;
        case e16th :
            nGlyph = (m_fStemDown ? GLYPH_16TH_NOTE_DOWN : GLYPH_16TH_NOTE_UP);
            break;
        case e32th :
            nGlyph = (m_fStemDown ? GLYPH_32ND_NOTE_DOWN : GLYPH_32ND_NOTE_UP);
            break;
        case e64th :
            nGlyph = (m_fStemDown ? GLYPH_64TH_NOTE_DOWN : GLYPH_64TH_NOTE_UP);
            break;
        case e128th :
            nGlyph = (m_fStemDown ? GLYPH_128TH_NOTE_DOWN : GLYPH_128TH_NOTE_UP);
            break;
        case e256th :
            nGlyph = (m_fStemDown ? GLYPH_256TH_NOTE_DOWN : GLYPH_256TH_NOTE_UP);
            break;
        case eLonga:
            nGlyph = GLYPH_LONGA_NOTE;
            break;
        case eBreve:
            nGlyph = GLYPH_BREVE_NOTE;
            break;
        case eWhole:
            nGlyph = GLYPH_WHOLE_NOTE;
            break;
        case eHalf:
            nGlyph = GLYPH_NOTEHEAD_HALF;
            break;
        case eQuarter:
            nGlyph = GLYPH_NOTEHEAD_QUARTER;
            break;
        //case enh_Cross:                    /// @todo enh_Cross is a notehead type not a note type
        //    nGlyph = GLYPH_NOTEHEAD_CROSS;
        //    break;
        default:
            wxASSERT(false);
    }

    wxString sGlyph( aGlyphsInfo[nGlyph].GlyphChar );
    return PrepareBitMap(rScale, sGlyph);

}

void lmNote::DrawObject(bool fMeasuring, lmPaper* pPaper, wxColour colorC, bool fHighlight)
{
    /*
    This method is invoked by the base class (lmStaffObj). When reaching this point 
    paper cursor variable (m_paperPos) has been updated. This value must be used
    as the base for any measurement / drawing operation.

    DrawObject() method is responsible for:
    1. In DO_MEASURE phase (fMeasuring == true):
        - Compute the surrounding rectangle, the glyph position and other measurements
    2. In DO_DRAW phase (fMeasuring == false):
        - Render the object

    */


    bool fDrawStem = true;            // assume stem down
    bool fInChord = IsInChord();

    //prepare DC
    pPaper->SetFont(*m_pFont);

    // move to right staff
    lmLUnits yStaffTopLine = m_paperPos.y + GetStaffOffset();   // staff y position (top line)
    lmLUnits nxLeft=0, nyTop=0;    // current pos. as positioning computation takes place
    int nPosOnStaff = GetPosOnStaff();
    lmLUnits yPitchShift = GetPitchShift();
    nyTop = yStaffTopLine - yPitchShift;
    nxLeft = m_paperPos.x;


    //If drawing phase do first MakeUp phase
    if (!fMeasuring) MakeUpPhase(pPaper);

    //In measuring phase, if this is the first note of a chord, give lmChord the
    //responsibility for computing chord layout (notes and rests' positions). If it is 
    //any other note of a chord, do nothing and mark the note as 'already measured'.
    bool fMeasured = false;
    if (fMeasuring) {
        if (IsBaseOfChord()) {
            //wxLogMessage( m_pChord->Dump() );
            m_pChord->ComputeLayout(pPaper, m_paperPos, colorC);
            fMeasured = true;
        }
        else if (IsInChord()) {
            fMeasured = true;
        }
    }

    //if this is the first note/rest of a beam, measure beam
    //@attention This must be done before using stem information, as the beam could
    //change stem direction if it is not determined for some/all the notes in the beam
    //During measurement phase all computations, except final trimming of stem' lengths, is
    //done. Final trimming of stems' length is delayed to MakeUp phase because it is not
    //posible to adjust lentghts until the x position of the notes is finally established, and
    //this takes place AFTER the measurement phase, once the lines are justified.
    if (fMeasuring && m_fBeamed && m_BeamInfo[0].Type == eBeamBegin) {
        m_pBeam->ComputeStemsDirection();
    }

    //render accidental signs if exist
    //@aware:
    //  Accidentals shape can not be part of note shape. This is required to allow
    //  independent positioning when user select the accidental and moves it.
    //  On the other hand, the note and its accidentals are not taken into account by the
    //  system justification process, as they are considered part of the note
    if (m_pAccidentals) {
        if ((!fMeasuring && !fHighlight) || !fMeasured && fMeasuring) {
            DrawAccidentals(pPaper, fMeasuring, nxLeft - m_paperPos.x, nyTop - m_paperPos.y, colorC);
        }
        nxLeft += m_pAccidentals->GetWidth();
    }

    //advance space before note
    nxLeft += m_nSpacePrev;

    //render the notehead (or the full note if single glyph)
    if (!fMeasuring || !fMeasured && fMeasuring) {
        fDrawStem = DrawNote(pPaper, fMeasuring, nxLeft - m_paperPos.x, nyTop - m_paperPos.y, colorC);
    }
    nxLeft += m_noteheadRect.width;
    lmLUnits xNote = nxLeft;

    if (!fMeasuring && fHighlight) return;  //highlight done. Finish


 //   nxCalderon = nxLeft + m_noteheadRect.width / 2 ;
    
    //draw dots
    //------------------------------------------------------------
    if (m_fDotted || m_fDoubleDotted) {
        //! @todo user selectable
        lmLUnits nSpaceBeforeDot = m_pVStaff->TenthsToLogical(5, m_nStaffNum);
        nxLeft += nSpaceBeforeDot;
        lmLUnits yPos = nyTop;
        if (nPosOnStaff % 2 == 0) {
            // notehead is over a line. Shift up the dots by half line
            yPos -= m_pVStaff->TenthsToLogical(5, m_nStaffNum);
        }
        nxLeft += DrawDot(fMeasuring, pPaper, nxLeft, yPos, colorC, true);
        if (m_fDoubleDotted) {
            nxLeft += nSpaceBeforeDot;
            nxLeft += DrawDot(fMeasuring, pPaper, nxLeft, yPos, colorC, true);
        }
    }

    
    //render lmTie to previous note
    //--------------------------------------------------------------------------------------
    if (!fMeasuring && m_pTiePrev) {
        m_pTiePrev->Draw(DO_DRAW, pPaper, colorC);
    }
    
    // draw the stem
    // In measurement phase the stem is measured even if it must not be drawn. This is
    // necessary to have information for positioning tuplets' bracket
    // @aware x position was set in DrawNote()
    //-----------------------------------------------------------------------------------
    if (m_nStemType != eStemNone) {
        if (fMeasuring)
        {
            //! @todo move thickness to user options
            #define STEM_WIDTH   12     //stem line width (cents = tenths x10)
            m_nStemThickness = m_pVStaff->TenthsToLogical(STEM_WIDTH, m_nStaffNum) / 10;
            // compute and store start position of stem
            if (m_fStemDown) {
                //stem down: line down on the left of the notehead
                nyTop += m_pVStaff->TenthsToLogical(51, m_nStaffNum);       
                m_yStem = nyTop - m_paperPos.y;
            } else {
                //stem up: line up on the right of the notehead
                m_xStem -= m_nStemThickness;
                nyTop += m_pVStaff->TenthsToLogical(49, m_nStaffNum);       
                m_yStem = nyTop - m_paperPos.y;
            }
            //adjust stem size if flag to be drawn
            if (!m_fDrawSmallNotesInBlock && !m_fBeamed && m_nNoteType > eQuarter) {
                int nGlyph = DrawFlag(fMeasuring, pPaper, lmUPoint(0, 0), colorC);
                lmLUnits nStem = GetStandardStemLenght();
                lmLUnits nFlag, nMinStem;
                if (m_fStemDown) {
                    nFlag = abs((int)( (float)(2048.0-aGlyphsInfo[nGlyph].Bottom) / 51.2 + 0.5));
                    nMinStem = (int)( (float)(aGlyphsInfo[nGlyph].Top - 2048.0 +128.0) / 51.2 + 0.5);
                }
                else {
                    nFlag = (int)( (float)aGlyphsInfo[nGlyph].Top / 51.2 + 0.5);
                    nMinStem = abs((int)( (float)aGlyphsInfo[nGlyph].Bottom / 51.2 + 0.5));
                }
                nFlag = (lmLUnits)m_pVStaff->TenthsToLogical(nFlag, m_nStaffNum);
                nMinStem = (lmLUnits)m_pVStaff->TenthsToLogical(nMinStem, m_nStaffNum);
                nStem = wxMax((nStem > nFlag ? nStem-nFlag : 0), nMinStem);
                m_yFlag = GetYStem() + (m_fStemDown ? nStem : -nStem);
                SetStemLength(nStem + nFlag);
            }

        } else {
            // not measuring. Do draw stem unless it is a note in chord. In this case
            // the stem will be drawn later, when drawing the last note of the chord
            if (fDrawStem && ! fInChord) {
                lmLUnits xPos = GetXStemLeft() + m_nStemThickness/2;     //SolidLine centers line width on given coordinates
                pPaper->SolidLine(xPos, GetYStem(), xPos, GetFinalYStem()+1,
                                    m_nStemThickness, eEdgeNormal, colorC);
            }
        }
    }

    //Draw flag
    //-----------------------------------------------------------------------------
    if (!m_fDrawSmallNotesInBlock && !fMeasuring) {
        if (!m_fBeamed && m_nNoteType > eQuarter && !IsInChord()) {
            int xPos = (m_fStemDown ? GetXStemLeft() : GetXStemRight());
            DrawFlag(fMeasuring, pPaper, lmUPoint(xPos, m_yFlag), colorC);
        }
    }

    // set total width
    #define NOTE_AFTERSPACE     10      //one line space     @todo user options
    lmLUnits nAfterSpace = m_pVStaff->TenthsToLogical(NOTE_AFTERSPACE, m_nStaffNum);
    if (fMeasuring) m_nWidth = nxLeft + nAfterSpace - m_paperPos.x;

    // draw leger lines if necessary
    //--------------------------------------------
    if (!fMeasuring) {
        lmLUnits xLine = m_noteheadRect.x - m_pVStaff->TenthsToLogical(4, m_nStaffNum);
        //lmLUnits xLine = m_pNoteheadShape->GetBoundsRectangle().x - m_pVStaff->TenthsToLogical(4, m_nStaffNum);
        lmLUnits widthLine = m_noteheadRect.width + 
                             m_pVStaff->TenthsToLogical(8, m_nStaffNum);
        //lmLUnits widthLine = m_pNoteheadShape->GetBoundsRectangle().width + 
        //                     m_pVStaff->TenthsToLogical(8, m_nStaffNum);
        DrawLegerLines(pPaper, nPosOnStaff, yStaffTopLine, xLine, widthLine, m_nStaffNum);
    }
    
    // render associated notations ------------------------------------------------------
    if (m_pNotations) {
        lmNoteRestObj* pNRO;
        wxAuxObjsListNode* pNode = m_pNotations->GetFirst();
        for (; pNode; pNode = pNode->GetNext() ) {
            pNRO = (lmNoteRestObj*)pNode->GetData();
            if (fMeasuring) {
                lmLUnits xPos = 0;
                lmLUnits yPos = 0;
                switch(pNRO->GetSymbolType()) {
                    case eST_Fermata:
                        // set position (relative to paperPos)
                        xPos = m_noteheadRect.x + m_noteheadRect.width / 2;
                        yPos = yStaffTopLine - m_paperPos.y;
                        pNRO->SetSizePosition(pPaper, m_pVStaff, m_nStaffNum, xPos, yPos);
                        pNRO->UpdateMeasurements();
                        break;
                    default:
                        wxASSERT(false);
                }
            }
            else
                pNRO->Draw(DO_DRAW, pPaper, colorC);
        }
    }

    // render associated lmLyric objects -------------------------------------------------
    if (m_pLyrics) {
        lmLyric* pLyric;
        wxAuxObjsListNode* pNode = m_pLyrics->GetFirst();
        for (; pNode; pNode = pNode->GetNext() ) {
            pLyric = (lmLyric*)pNode->GetData();
            if (fMeasuring) {
                // set position (relative to paperPos)
                lmLUnits xPos = m_noteheadRect.x;
                lmLUnits yPos = yStaffTopLine - m_paperPos.y;
                pLyric->SetSizePosition(pPaper, m_pVStaff, m_nStaffNum, xPos, yPos);
                pLyric->UpdateMeasurements();
            }
            else
                pLyric->Draw(DO_DRAW, pPaper, colorC);
        }
    }

    //if this last note of a chord draw the stem of the chord
    //--------------------------------------------------------------------------------------
    if (!fMeasuring && IsInChord() && m_pChord->IsLastNoteOfChord(this) 
        && m_nNoteType >= eHalf)
    {
        m_pChord->DrawStem(fMeasuring, pPaper, colorC, m_pFont, m_pVStaff, m_nStaffNum);
    }

    //if this is the last note of a beamed group draw the beam lines
    //--------------------------------------------------------------------------------------
    if (!fMeasuring && m_fBeamed && m_BeamInfo[0].Type == eBeamEnd) {
        lmLUnits nThickness = m_pVStaff->TenthsToLogical(5, m_nStaffNum);
        //DOC: Beam spacing
        // ----------------
        //according to http://www2.coloradocollege.edu/dept/mu/Musicpress/engraving.html
        //distance between primary and secondary beams should be 1/4 space (2.5 tenths) 
        //But if I use 3 tenths (2.5 up rounding) beam spacing is practicaly
        //invisible. In pictures displayed in the above mentioned www page, spacing
        //is about 1/2 space, not 1/4 space. So I will use 5 tenths.
        //So the number to put in next statement is 9:
        //  4 for beam thikness + 5 for beams spacing
        lmLUnits nBeamSpacing = m_pVStaff->TenthsToLogical(9, m_nStaffNum);
        m_pBeam->DrawBeamLines(pPaper, nThickness, nBeamSpacing, colorC);
    }

    //if this is the last note of a tuplet draw the tuplet bracket
    //--------------------------------------------------------------------------------------
    if (m_pTupletBracket && (m_pTupletBracket->GetEndNote())->GetID() == m_nId) {
        m_pTupletBracket->Draw(fMeasuring, pPaper, colorC);
    }

    ////DBG
    //if (fMeasuring) wxLogMessage( m_pNoteheadShape->Dump() );
    
}

lmLUnits lmNote::DrawAccidentals(lmPaper* pPaper, bool fMeasuring,
                        lmLUnits xOffset, lmLUnits yOffset, wxColour colorC)
{
    if (fMeasuring) {
        m_pAccidentals->Measure(pPaper, m_pVStaff->GetStaff(m_nStaffNum),
                                lmUPoint(xOffset, yOffset));
    }
    else {
        m_pAccidentals->Render(pPaper, m_paperPos, colorC);
        m_pAccidentals->Draw(DO_DRAW, pPaper, colorC);  //compatibility: to draw selrect
    }

    return m_pAccidentals->GetWidth();

}

bool lmNote::DrawNote(lmPaper* pPaper, bool fMeasuring,
                      lmLUnits xOffset, lmLUnits yOffset, wxColour colorC)
{
    // draws the notehead (or the full note if single glyph) and returns flag to 
    // draw or not the stem

    lmLUnits nxLeft = xOffset + m_paperPos.x;
    lmLUnits nyTop = yOffset + m_paperPos.y;
    bool fDrawStem = true;

    //initially set anchor point at left side of notehead
    if (fMeasuring) { m_xAnchor = xOffset; }

    if (m_fDrawSmallNotesInBlock && !m_fBeamed && m_nNoteType > eQuarter && !IsInChord()) {
        // It is a single note with flag: draw it in one step with a glyph
        DrawSingleNote(pPaper, fMeasuring, m_nNoteType, m_fStemDown, 
                       nxLeft, nyTop, (m_fSelected ? g_pColors->ScoreSelected() : colorC) );
        fDrawStem = false;

    } else {
        // either the note is part of a group of beamed notes, is in a chord, or doesn't
        // have stem: it must be drawn in parts
        // Draw the notehead
        ENoteHeads nNotehead;
        //if (! m_fCabezaX) {
            if (m_nNoteType > eHalf) {
                nNotehead = enh_Quarter;
            } else if (m_nNoteType == eHalf) {
                nNotehead = enh_Half;
            } else if (m_nNoteType == eWhole) {
                nNotehead = enh_Whole;
                fDrawStem = false;
            } else if (m_nNoteType == eBreve) {
                nNotehead = enh_Breve;
                fDrawStem = false;
            } else if (m_nNoteType == eLonga) {
                nNotehead = enh_Longa;
                fDrawStem = false;
            } else {
                wxLogMessage(_T("[lmNote::DrawNote] Unknown note type."));
                wxASSERT(false);
            }
        //} else {
        //    nNotehead = enh_Cross;
        //}
        
        DrawNoteHead(pPaper, fMeasuring, nNotehead, nxLeft, nyTop, 
                     (m_fSelected ? g_pColors->ScoreSelected() : colorC) );
    }

    //set stem x position
    //x position is set even if stem must not be drawn. This is
    //necessary to have information for positioning tuplets' bracket
    if (fMeasuring) {
        if (m_fStemDown) {
            //stem down: line down on the left of the notehead unless notehead reversed
            m_xStem = m_noteheadRect.x;
            if (m_fNoteheadReversed) m_xStem += m_noteheadRect.width;
        } else {
            //stem up: line up on the right of the notehead unless notehead reversed
            m_xStem = m_noteheadRect.x;
            if (!m_fNoteheadReversed) m_xStem += m_noteheadRect.width;
        }
    }

    //in chords, if nothehead is reversed the anchor point have to be at the other side
    if (fMeasuring && m_fNoteheadReversed) {
        m_xAnchor += (m_fStemDown ? m_noteheadRect.width : -m_noteheadRect.width);
    }


    return fDrawStem;

}

lmEGlyphIndex lmNote::DrawFlag(bool fMeasuring, lmPaper* pPaper, lmUPoint pos, wxColour colorC)
{
    //
    //Draws the flag using a glyph. Returns the glyph index
    //

    lmEGlyphIndex nGlyph = GLYPH_EIGHTH_FLAG_DOWN;
    switch (m_nNoteType) {
        case eEighth :
            nGlyph = (m_fStemDown ? GLYPH_EIGHTH_FLAG_DOWN : GLYPH_EIGHTH_FLAG_UP);
            break;
        case e16th :
            nGlyph = (m_fStemDown ? GLYPH_16TH_FLAG_DOWN : GLYPH_16TH_FLAG_UP);
            break;
        case e32th :
            nGlyph = (m_fStemDown ? GLYPH_32ND_FLAG_DOWN : GLYPH_32ND_FLAG_UP);
            break;
        case e64th :
            nGlyph = (m_fStemDown ? GLYPH_64TH_FLAG_DOWN : GLYPH_64TH_FLAG_UP);
            break;
        case e128th :
            nGlyph = (m_fStemDown ? GLYPH_128TH_FLAG_DOWN : GLYPH_128TH_FLAG_UP);
            break;
        case e256th :
            nGlyph = (m_fStemDown ? GLYPH_256TH_FLAG_DOWN : GLYPH_256TH_FLAG_UP);
            break;
        default:
            wxLogMessage(_T("[lmNote::DrawFlag] Error: invalid note type %d."),
                        m_nNoteType);
        }

    wxString sGlyph( aGlyphsInfo[nGlyph].GlyphChar );
  
    if (!fMeasuring) {
        // drawing phase: do the draw
        pPaper->SetTextForeground(colorC);
        //wxLogMessage(_T("[lmNote::DrawFlag] x=%d, y=%d"), pos.x, pos.y + m_pVStaff->TenthsToLogical( aGlyphsInfo[nGlyph].GlyphOffset, m_nStaffNum ));
        pPaper->DrawText(sGlyph, pos.x, pos.y + m_pVStaff->TenthsToLogical( aGlyphsInfo[nGlyph].GlyphOffset, m_nStaffNum ) );
    }

    return nGlyph;
        
}

void lmNote::ShiftNoteShape(lmLUnits xShift)
{
    //reposition the note shape and all directly related measurements: stem x pos,
    //anchor pos,
    //This method does not change the accidentals position, only the note itself.

    m_pNoteheadShape->Shift(xShift);    // shift notehead
    m_xAnchor += xShift;                // shift anchor line
    m_noteheadRect.x += xShift;
    m_selRect.x += xShift;
    m_xStem += xShift;

}

void lmNote::MakeUpPhase(lmPaper* pPaper)
{

    //Beams. If this is the first note/rest of a beam, do final trimming of stems' length
    if (m_fBeamed && m_BeamInfo[0].Type == eBeamBegin) {
        m_pBeam->TrimStems();
    }

    //Ties. If this note is the end of a tie, store tie end point
    if (m_pTiePrev) {
        // compute end point, relative to end note paperPos (this note) 
        lmLUnits xPos = m_noteheadRect.x + m_noteheadRect.width / 2;
        lmLUnits yPos = m_pVStaff->TenthsToLogical(5, m_nStaffNum);
        yPos = (m_pTiePrev->IsUnderNote() ? 
                m_noteheadRect.y + m_noteheadRect.height + yPos :
                m_noteheadRect.y - yPos );
        m_pTiePrev->SetEndPoint(xPos, yPos, pPaper->GetLeftMarginXPos());
    }
    
    //Ties. If this note is start of tie, store tie start point. Tie position is fixed
    //by start note stem direction
    if (m_pTieNext) {
        // compute start point, relative to start note paperPos (this note) 
        lmLUnits xPos = m_noteheadRect.x + m_noteheadRect.width / 2;
        lmLUnits yPos = m_pVStaff->TenthsToLogical(5, m_nStaffNum);
        yPos = (m_fStemDown ? 
                m_noteheadRect.y - yPos : m_noteheadRect.y + m_noteheadRect.height + yPos);
        m_pTieNext->SetStartPoint(xPos, yPos, pPaper->GetRightMarginXPos(), !m_fStemDown);
    }

}

void lmNote::DrawLegerLines(lmPaper* pPaper, int nPosOnStaff, lmLUnits yStaffTopLine,
                            lmLUnits xPos, lmLUnits width, int nStaff, int nROP)
{
    if (nPosOnStaff > 0 && nPosOnStaff < 12) return;

    if (nROP != wxCOPY) pPaper->SetLogicalFunction(nROP);

    xPos += m_paperPos.x;        // make it absolute
    lmLUnits nThick = m_pVStaff->GetStaffLineThick(nStaff);

    int i;
    lmLUnits yPos, nTenths;
    if (nPosOnStaff > 11) {
        // lines at top
        for (i=12; i <= nPosOnStaff; i++) {
            if (i % 2 == 0) {
                nTenths = 5 * (i - 10);
                yPos = yStaffTopLine - m_pVStaff->TenthsToLogical(nTenths, m_nStaffNum);
                pPaper->SolidLine(xPos, yPos, xPos + width, yPos, nThick,
                                   eEdgeNormal, *wxBLACK);
            }
        }

    } else {
        // nPosOnStaff < 1: lines at bottom
        for (i=nPosOnStaff; i <= 0; i++) {
            if (i % 2 == 0) {
                nTenths = 5 * (10 - i);
                yPos = yStaffTopLine + m_pVStaff->TenthsToLogical(nTenths, m_nStaffNum);
                pPaper->SolidLine(xPos, yPos, xPos + width, yPos, nThick,
                                   eEdgeNormal, *wxBLACK);
            }
        }
    }

    if (nROP != wxCOPY) pPaper->SetLogicalFunction(wxCOPY);    // restore DC logical function

}

void lmNote::DrawSingleNote(lmPaper* pPaper, bool fMeasuring, ENoteType nNoteType,
        bool fStemAbajo, lmLUnits nxLeft, lmLUnits nyTop, wxColour colorC)
{
    /*
    Draws a note by using a glyph. 
    In DO_MEASURE mode also stores measurements.
    */
        
    lmEGlyphIndex nGlyph = GLYPH_EIGHTH_NOTE_UP;
    switch (nNoteType) {
        case eEighth :
            nGlyph = (m_fStemDown ? GLYPH_EIGHTH_NOTE_DOWN : GLYPH_EIGHTH_NOTE_UP);
            break;
        case e16th :
            nGlyph = (m_fStemDown ? GLYPH_16TH_NOTE_DOWN : GLYPH_16TH_NOTE_UP);
            break;
        case e32th :
            nGlyph = (m_fStemDown ? GLYPH_32ND_NOTE_DOWN : GLYPH_32ND_NOTE_UP);
            break;
        case e64th :
            nGlyph = (m_fStemDown ? GLYPH_64TH_NOTE_DOWN : GLYPH_64TH_NOTE_UP);
            break;
        case e128th :
            nGlyph = (m_fStemDown ? GLYPH_128TH_NOTE_DOWN : GLYPH_128TH_NOTE_UP);
            break;
        case e256th :
            nGlyph = (m_fStemDown ? GLYPH_256TH_NOTE_DOWN : GLYPH_256TH_NOTE_UP);
            break;
        default:
            wxASSERT(false);
    }

    wxString sGlyph( aGlyphsInfo[nGlyph].GlyphChar );
  
    if (fMeasuring) {
        // store glyph position
        m_glyphPos.x = nxLeft - m_paperPos.x;
        //m_glyphPos.y = nyTop - m_paperPos.y + m_pVStaff->TenthsToLogical( (fStemAbajo ? 30 : -10), m_nStaffNum );
        m_glyphPos.y = nyTop - m_paperPos.y + m_pVStaff->TenthsToLogical( aGlyphsInfo[nGlyph].GlyphOffset , m_nStaffNum );


        // define selection rectangle (relative to m_paperPos)
        lmLUnits nWidth, nHeight;
        pPaper->GetTextExtent(sGlyph, &nWidth, &nHeight);
        m_selRect.width = nWidth;
        m_selRect.height = m_pVStaff->TenthsToLogical( aGlyphsInfo[nGlyph].SelRectHeight, m_nStaffNum );
        m_selRect.x = m_glyphPos.x;
        m_selRect.y = m_glyphPos.y + m_pVStaff->TenthsToLogical( aGlyphsInfo[nGlyph].SelRectShift, m_nStaffNum );

        // store notehead position and size
        m_noteheadRect = m_selRect;        //! @todo compute notehead rectangle

    } else {
        // else (drawing phase) do the draw
        lmUPoint pos = GetGlyphPosition();
        pPaper->SetTextForeground(colorC);
        pPaper->DrawText(sGlyph, pos.x, pos.y );
    }

}

void lmNote::DrawNoteHead(lmPaper* pPaper, bool fMeasuring, ENoteHeads nNoteheadType,
    lmLUnits nxLeft, lmLUnits nyTop, wxColour colorC)
{
    // draws a notehead of type nNoteheadType on position (nxLeft, nyTop) with color colorC.
    // In DO_MEASURE mode also stores measurements.
    
    lmEGlyphIndex nGlyph = GLYPH_NOTEHEAD_QUARTER;
    switch (nNoteheadType) {
        case enh_Longa:
            nGlyph = GLYPH_LONGA_NOTE;
            break;
        case enh_Breve:
            nGlyph = GLYPH_BREVE_NOTE;
            break;
        case enh_Whole:
            nGlyph = GLYPH_WHOLE_NOTE;
            break;
        case enh_Half:
            nGlyph = GLYPH_NOTEHEAD_HALF;
            break;
        case enh_Quarter:
            nGlyph = GLYPH_NOTEHEAD_QUARTER;
            break;
        case enh_Cross:
            nGlyph = GLYPH_NOTEHEAD_CROSS;
            break;
        default:
            wxASSERT(false);
    }

    wxString sGlyph( aGlyphsInfo[nGlyph].GlyphChar );

    if (fMeasuring) {
        lmUPoint offset(nxLeft - m_paperPos.x, nyTop - m_paperPos.y);
        if (!m_pNoteheadShape) m_pNoteheadShape = new lmShapeGlyph(this, nGlyph, m_pFont);
        m_pNoteheadShape->Measure(pPaper, m_pVStaff->GetStaff(m_nStaffNum), offset);

        //COMPATIBILITY

        // store positions
        m_glyphPos.x = nxLeft - m_paperPos.x;
        m_glyphPos.y = nyTop - m_paperPos.y - m_pVStaff->TenthsToLogical(aGlyphsInfo[nGlyph].GlyphOffset, m_nStaffNum);

        // store selection rectangle position and size
        m_selRect = m_pNoteheadShape->GetSelRectangle();

        // store notehead position and size. selRect bounds the notehead, so just copy it
        m_noteheadRect = m_selRect;

        //fix for width. It is not correctly computed by DC->GetTextExtent
        //m_noteheadRect.width -= m_pVStaff->TenthsToLogical(10, m_nStaffNum)/10;


    } else {
        //wxLogMessage(_T("[lmNote::DrawNoteHead]"));
        m_pNoteheadShape->Render(pPaper, m_paperPos, colorC);
    }

}

void lmNote::MoveDragImage(lmPaper* pPaper, wxDragImage* pDragImage, lmDPoint& offsetD, 
                         const lmUPoint& pagePosL, const lmUPoint& dragStartPosL, const lmDPoint& canvasPosD)
{

    /*
    A note must stay on staff lines or spaces
    */

    lmLUnits dyHalfLine = m_pVStaff->TenthsToLogical(5, m_nStaffNum );
    lmUPoint nShiftVector = pagePosL - dragStartPosL;    // the displacement
    int nSteps = (nShiftVector.y / dyHalfLine);        // trim the displacement to half line steps
    nShiftVector.y -= nSteps;
    lmUPoint newPaperPos = m_paperPos + nShiftVector;
    // then the shape must be drawn at:
    lmDPoint ptNewD;
    ptNewD.x = pPaper->LogicalToDeviceX(newPaperPos.x + m_glyphPos.x) + offsetD.x;
    ptNewD.y = pPaper->LogicalToDeviceY(newPaperPos.y + m_glyphPos.y) + offsetD.y;
    pDragImage->Move(ptNewD);

    /*
    // compute new pitch
    int nNewPitch = PosOnStaffToPitch(nSteps);
    SetUpPitchRelatedVariables(nNewPitch);
    */
}

lmUPoint lmNote::EndDrag(const lmUPoint& pos)
{
    lmUPoint oldPos(m_paperPos + m_glyphPos);

    /*
    Notes can not freely moved. They must stay on staff lines or spaces
    */
    lmLUnits dyHalfLine = m_pVStaff->TenthsToLogical(5, m_nStaffNum );
    lmLUnits nShift = - (pos.y - GetGlyphPosition().y);
    int nSteps = nShift / dyHalfLine;        // trim the displacement to half line steps

    // compute new pitch
    int nNewPitch = PosOnStaffToPitch(nSteps);
    SetUpPitchRelatedVariables(nNewPitch);

    //wxLogMessage( wxString::Format(wxT("EndDrag: nShift=%d, nSteps=%d, nNewPitch=%d"), 
    //    nShift, nSteps, nNewPitch ) );

    //ojo: estas dos l�neas son el comportamiento de la clase base. Hay que dejarlas
    //de momento porque el flag m_fFixedPos impide que se actualice la posici�n
    // �Llevarlo a SetUpPitchRelatedVariables() ?
    //m_paperPos.x = pos.x - m_glyphPos.x;
    //m_paperPos.y = pos.y - m_glyphPos.y;

    //SetFixed(false);

    return lmUPoint(oldPos);

}

int lmNote::PosOnStaffToPitch(int nSteps)
{
    /*
    When the note is dragged it is necessary to compute the new pitch from the
    its new position on the paper. From the paper displacement it is computed how
    many half line steps the note has been moved. This method receives the steps
    and computes the new pitch
    */

    int nPos = GetPosOnStaff() + nSteps;
    switch (m_nClef) {
        case eclvSol :
            return nPos + lmC4PITCH;
        case eclvFa4 :
            return nPos + lmC4PITCH - 12;
        case eclvFa3 :
            return nPos + lmC4PITCH - 10;
        case eclvDo1 :
            return nPos + lmC4PITCH - 2;
        case eclvDo2 :
            return nPos + lmC4PITCH - 4;
        case eclvDo3 :
            return nPos + lmC4PITCH - 6;
        case eclvDo4 :
            return nPos + lmC4PITCH - 8;
        default:
            wxASSERT(false);
            return 0;    //to get the compiler happy
    } 

}

void lmNote::SetUpPitchRelatedVariables(lmPitch nNewPitch)
{
    m_nPitch = nNewPitch;
    //! @todo compute new m_nAlter 
    int nAlter = m_nAlter;
    m_nMidiPitch = PitchToMidi(nNewPitch, nAlter);
    SetUpStemDirection();
}

void lmNote::SetUpStemDirection()
{
    switch (m_nStemType) {
        case eDefaultStem:
            m_fStemDown = (GetPosOnStaff() >= 6);
            break;
        case eStemDouble:
            /*! @todo
                I understand that "eStemDouble" means two stems: one up and one down.
                This is not yet implemented and is treated as eDefaultStem
            */
            m_fStemDown = (GetPosOnStaff() >= 6);
            break;
        case eStemUp:
            m_fStemDown = false;
            break;
        case eStemDown:
            m_fStemDown = true;
            break;
        case eStemNone:
            m_fStemDown = false;       //false or true. The value doesn't matter.
            break;
        default:
            wxASSERT(false);
    }

    m_nStemLength = GetStandardStemLenght();

    //wxLogMessage( wxString::Format(wxT("SetUpPitchRelatedVariables: tipoStem=%d, plica=%s, nPosOnStaff %d"), 
    //    m_nStemType, (m_fStemDown ? _T("Abajo") : _T("Arriba")), GetPosOnStaff() ) );

}

lmLUnits lmNote::GetDefaultStemLength()
{
    // Returns the standard default value for stem lengths. This value is used as
    // starting point for stem computations, for example, to compute the stem in beams.

    // According to engraving rules, normal length is one octave (3.5 spaces)
    #define DEFAULT_STEM_LEGHT      35      //! in tenths. @todo move to user options

    return m_pVStaff->TenthsToLogical(DEFAULT_STEM_LEGHT, m_nStaffNum);
}

lmLUnits lmNote::GetStandardStemLenght()
{
    // Returns the stem lenght that this note should have, according to engraving
    // rules. It takes into account the `posiotion of the note on the staff.
    //
    // a1 - Normal length is one octave (3.5 spaces), but only for notes between the spaces
    //      previous to first leger lines (b3 and b5, in Sol key, both inclusive).
    //
    // a2 - Notes with stems upwards from c5 inclusive, or with stems downwards from
    //      g4 inclusive have a legth of 2.5 spaces.
    //
    // a3 - If a note is on or above the second leger line above the staff, or
    //      on or below the second leger line below the staff: the end of stem
    //      have to touch the middle staff line. 

    int nPos = GetPosOnStaff();     //0 = first leger line below staff
    int nTenths;

    // rule a3
    if (nPos >= 14 && m_fStemDown) {
        nTenths = 5 * (nPos-6);     // touch middle line
    }
    else if (nPos <= -2 && !m_fStemDown) {
        nTenths = 5 *(6-nPos);     // touch middle line
    }

    // rule a2
    else if ((nPos >= 7 && !m_fStemDown) || (nPos <= 4 && m_fStemDown)) {
        nTenths = 25;     // 2.5 spaces
    }

    // rule a1 and any other case not covered (I did not analyze completness)
    else {
        nTenths = 35;     // 3.5 spaces
    }

    return (lmLUnits)m_pVStaff->TenthsToLogical(nTenths, m_nStaffNum);

}

void lmNote::SetStemDirection(bool fStemDown)
{
    m_fStemDown = fStemDown;
    m_nStemLength = GetStandardStemLenght();

    if (IsBaseOfChord()) {
        //propagate change to max and min notes of chord
        m_pChord->SetStemDirection(fStemDown);
    }

}

int lmNote::GetPosOnStaff()
{
    // Returns the position on the staff (line/space) referred to the first ledger line of
    // the staff. Depends on clef:
    //        0 - on first ledger line (C note in G clef)
    //        1 - on next space (D in G clef)
    //        2 - on first line (E not in G clef)
    //        3 - on first space
    //        4 - on second line
    //        5 - on second space
    //        etc.

    switch (m_nClef) {
        case eclvSol :
            return m_nPitch - lmC4PITCH;
        case eclvFa4 :
            return m_nPitch - lmC4PITCH + 12;
        case eclvFa3 :
            return m_nPitch - lmC4PITCH + 10;
        case eclvFa5 :
            return m_nPitch - lmC4PITCH + 14;
        case eclvDo1 :
            return m_nPitch - lmC4PITCH + 2;
        case eclvDo2 :
            return m_nPitch - lmC4PITCH + 4;
        case eclvDo3 :
            return m_nPitch - lmC4PITCH + 6;
        case eclvDo4 :
            return m_nPitch - lmC4PITCH + 8;
        case eclvDo5 :
            return m_nPitch - lmC4PITCH + 10;
        default:
            // no key, assume eclvSol
            return m_nPitch - lmC4PITCH;
    } 
}

// Rectangle that bounds the image. Absolute position referred to page origin
lmLUnits lmNote::GetBoundsTop()
{
    return (m_fStemDown ? m_selRect.GetTop() + m_paperPos.y : GetFinalYStem() );
}

lmLUnits lmNote::GetBoundsBottom()
{
    return (m_fStemDown ? GetFinalYStem() : m_selRect.GetBottom() + m_paperPos.y );
}

lmLUnits lmNote::GetBoundsLeft()
{
    return (m_fStemDown ? GetXStemLeft() : m_selRect.GetLeft() + m_paperPos.x );
}

lmLUnits lmNote::GetBoundsRight()
{
    return (m_fStemDown ? m_selRect.GetRight() + m_paperPos.x : GetXStemLeft() );
}

void lmNote::SetLeft(lmLUnits nLeft)
{
    //
    // The SetLeft() method in StaffObj is overriden to take into account the possible
    // existence of associated AuxObjs, such as Ties and Accidentals. These AuxObjs have
    // also to be moved when the note is moved
    //

    m_paperPos.x = nLeft;

    //move Ties
    if (m_pTieNext) m_pTieNext->UpdateMeasurements();
    if (m_pTiePrev) m_pTiePrev->UpdateMeasurements();

    //move associated AuxObjs
    if (m_pNotations) {
        lmNoteRestObj* pNRO;
        wxAuxObjsListNode* pNode = m_pNotations->GetFirst();
        for (; pNode; pNode = pNode->GetNext() ) {
            pNRO = (lmNoteRestObj*)pNode->GetData();
            pNRO->UpdateMeasurements();
        }
    }

    // move associated lyrics
    if (m_pLyrics) {
        lmLyric* pLyric;
        wxAuxObjsListNode* pNode = m_pLyrics->GetFirst();
        for (; pNode; pNode = pNode->GetNext() ) {
            pLyric = (lmLyric*)pNode->GetData();
            pLyric->UpdateMeasurements();
        }
    }

    //move associated accidentals
    if (m_pAccidentals) m_pAccidentals->UpdateMeasurements();

}

wxString lmNote::Dump()
{
    wxString sDump;
    sDump = wxString::Format(
        _T("%d\tNote\tType=%d, Pitch=%d, MidiPitch=%d, PosOnStaff=%d, Step=%d, Alter=%d, TimePos=%.2f, rDuration=%.2f, StemType=%d"),
        m_nId, m_nNoteType, m_nPitch, m_nMidiPitch, GetPosOnStaff(), m_nStep, m_nAlter, m_rTimePos, m_rDuration,
        m_nStemType);
    if (m_pTieNext) sDump += _T(", TiedNext");
    if (m_pTiePrev) sDump += _T(", TiedPrev");
    if (IsBaseOfChord())
        sDump += _T(", BaseOfChord");
    if (IsInChord()) {
        sDump += wxString::Format(_T(", InChord, Notehead shift = %s"), 
            (m_fNoteheadReversed ? _T("yes") : _T("no")) );
    }
    if (m_fBeamed) {
        sDump += wxString::Format(_T(", Beamed: BeamTypes(%d"), m_BeamInfo[0].Type);
        for (int i=1; i < 6; i++) {
            sDump += wxString::Format(_T(",%d"), m_BeamInfo[i].Type);
        }
        sDump += _T(")");
    }
    if (m_pTupletBracket) {
        if ((m_pTupletBracket->GetEndNote())->GetID() == m_nId) { 
            sDump += _T(", End of tuplet\n");
            sDump += m_pTupletBracket->Dump();
        }
        else if ((m_pTupletBracket->GetStartNote())->GetID() == m_nId) 
            sDump += _T(", Start of tuplet");
        else
            sDump += _T(", In tuplet");
    }
    //stem info
    if (m_nStemType != eStemNone) {
        sDump += wxString::Format(_T(", xStem=%d, yStem=%d, length=%d"), 
                    m_xStem, m_yStem,m_nStemLength );
    }
    sDump += _T("\n");

    // Dump associated lyrics
    if (m_pLyrics) {
        lmLyric* pLyric;
        wxAuxObjsListNode* pNode = m_pLyrics->GetFirst();
        for (; pNode; pNode = pNode->GetNext() ) {
            pLyric = (lmLyric*)pNode->GetData();
            sDump += pLyric->Dump();
        }
    }
                      
    return sDump;
    
}

wxString lmNote::SourceLDP()
{
    wxString sSource = _T("         (n ");    
    //(fInChord ? _T("            (NA "), _T("            (N "));
    sSource += MIDINoteToLDPPattern(PitchToMidi(m_nPitch,0), earmDo, (lmPitch*)NULL);
    sSource += _T(" ");
    sSource += GetLDPNoteType();
    if (m_fDotted) sSource += _T(".");
    if (m_fDoubleDotted) sSource += _T(".");

    //! @todo take tonal key into account

    //! @todo Finish lmNote LDP Source code generation method

    //tied
    if (IsTiedToNext()) sSource += _T(" l");

    //start or end of group
    if (m_fBeamed) {
        if (m_BeamInfo[0].Type == eBeamBegin) {
            sSource += _T(" g+");
        }
        else if (m_BeamInfo[0].Type == eBeamEnd) {
            sSource += _T(" g-");
        }
    }

    //tuplets
    if (m_pTupletBracket) {
        if ((lmNoteRest*)this == m_pTupletBracket->GetStartNote()) {
            sSource += wxString::Format(_T(" t%d"), m_pTupletBracket->GetTupletNumber());
        }
        else if((lmNoteRest*)this == m_pTupletBracket->GetEndNote()) {
            sSource += _T(" t-");
        }
    }

    //staff num
    if (m_pVStaff->GetNumStaves() > 1) {
        sSource += wxString::Format(_T(" p%d"), m_nStaffNum);
    }


//    if (nCalderon == eC_ConCalderon) m_sFuente = m_sFuente & " C";
//    for (int i=1; i <= cAnotaciones.Count; i++) {
//        m_sFuente = m_sFuente & " " & cAnotaciones.Item(i);
//    }
    sSource += _T(")\n");
    return sSource;
}

wxString lmNote::SourceXML()
{
    wxString sSource = _T("TODO: lmNote XML Source code generation methods");
    return sSource;
//    sPitch = GetNombreSajon(m_nPitch)
//    
//    sFuente = "            <note>" & sCrLf
//    sFuente = sFuente & "                <pitch>" & sCrLf
//    sFuente = sFuente & "                    <step>" & Left$(sPitch, 1) & "</step>" & sCrLf
//    sFuente = sFuente & "                    <octave>" & Mid$(sPitch, 2) & "</octave>" & sCrLf
//    sFuente = sFuente & "                </pitch>" & sCrLf
//    sFuente = sFuente & "                <duration>2</duration>" & sCrLf
//    sFuente = sFuente & "                <voice>1</voice>" & sCrLf
//    sFuente = sFuente & "                <type>quarter</type>" & sCrLf
//    sFuente = sFuente & "                <stem>up</stem>" & sCrLf
//    sFuente = sFuente & "                <notations>" & sCrLf
//    sFuente = sFuente & "                    <slur type=""start"" number=""1""/>" & sCrLf
//    sFuente = sFuente & "                </notations>" & sCrLf
//    sFuente = sFuente & "            </note>" & sCrLf
//    FuenteXML = sFuente
}

lmChord* lmNote::StartChord()
{
    //Start a chord with this note as the base note of the chord
    m_pChord = new lmChord(this);
    m_fIsNoteBase = true;
    return m_pChord;
}

//devuelve true si esta nota puede estar ligada con otra cuyos valores se reciben como argumentos
bool lmNote::CanBeTied(lmPitch nMidiPitch, int nStep)
{
    //wxLogMessage(wxString::Format(
    //    _T("[lmNote::CanBeTied(%d, %d): m_nMidiPitch=%d, m_nStep=%d"),
    //    nMidiPitch, nStep, m_nMidiPitch, m_nStep));
    if (nMidiPitch != m_nMidiPitch || nStep != m_nStep) return false;
    return true;    // can be tied
}


//======================================================================================
// lmCompositeObj virtual functions implementation
//======================================================================================

lmScoreObj* lmNote::FindSelectableObject(lmUPoint& pt)
{
    if (IsAtPoint(pt)) return this;

    // try with ties
    if (m_pTieNext && m_pTieNext->IsAtPoint(pt)) return m_pTieNext;
    if (m_pTiePrev && m_pTiePrev->IsAtPoint(pt)) return m_pTiePrev;

    // try with associated AuxObjs
    if (m_pNotations) {
        lmNoteRestObj* pNRO;
        wxAuxObjsListNode* pNode = m_pNotations->GetFirst();
        for (; pNode; pNode = pNode->GetNext() ) {
            pNRO = (lmNoteRestObj*)pNode->GetData();
            if (pNRO->IsAtPoint(pt)) return pNRO;
        }
    }

    // try with associated lyrics
    if (m_pLyrics) {
        lmLyric* pLyric;
        wxAuxObjsListNode* pNode = m_pLyrics->GetFirst();
        for (; pNode; pNode = pNode->GetNext() ) {
            pLyric = (lmLyric*)pNode->GetData();
            if (pLyric->IsAtPoint(pt)) return pLyric;
        }
    }

    // Not found
    return (lmScoreObj*)NULL;    //none found

}



//==========================================================================================
// Global functions related to notes
//    See AuxString.cpp for details about note pitch encoding
//==========================================================================================

lmPitch StepAndOctaveToPitch(int nStep, int nOctave)
{
    return (nStep + 1) + (7 * nOctave);
}

// PitchToMidiPitch -> PitchToMidi
lmPitch PitchToMidi(lmPitch nPitch, int nAlter)
{
    int nOctave = ((nPitch - 1) / 7) + 1;    
    wxASSERT(lmC4PITCH == 29);    //@attention It's assumed that we start in C0
    lmPitch nMidi = (lmPitch)(nOctave * 12);

    switch(nPitch % 7)
    {
        case 0:  //si
            nMidi = nMidi + 11;
            break;
        case 1:  //do
            //nada. valor correcto
            break;
        case 2:  //re
            nMidi = nMidi + 2;
            break;
        case 3:  //mi
            nMidi = nMidi + 4;
            break;
        case 4:  //fa
            nMidi = nMidi + 5;
            break;
        case 5:  //sol
            nMidi = nMidi + 7;
            break;
        case 6:  //la
            nMidi = nMidi + 9;
            break;
    }
    
    return nMidi + nAlter;
    
}

wxString MIDINoteToLDPPattern(lmPitch nPitchMIDI, EKeySignatures nTonalidad, lmPitch* pPitch)
{
    /*
    Returns the LDP pattern (accidentals, note name and octave) representing the MIDI pitch
    received as parameter. It takes into account the tonal key so, for example, for MIDI 70
    (flat B4) in "F major" it returns B (pattern="B4", ntDiat=35 B), but in tonal key "A major"
    it returns La# (pattern="+A4", ntDiat=34 A).

    If pointer pPitch is not NULL stores in the variable pointed by this pointer the
    diatonic pitch.

    */

    int nOctave = (nPitchMIDI - 12) / 12;
    int nResto = nPitchMIDI % 12;
    //
    wxString sPattern, sDisplcm;
    switch(nTonalidad)
    {
        case earmDo:
        case earmLam:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  "
            sPattern = _T("c  c+ d  d+ e  f  f+ g  g+ a  a+ b  ");
            sDisplcm = _T("0  0  1  1  2  3  3  4  4  5  5  6  ");
            break;

    //    //Sostenidos ---------------------------------------
        case earmSol:
        case earmMim:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  " #F
            sPattern = _T("c  c+ d  d+ e  e+ f  g  g+ a  a+ b  ");
            sDisplcm = _T("0  0  1  1  2  2  3  4  4  5  5  6  ");
            break;

        case earmRe:
        case earmSim:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  " #FC
            sPattern = _T("b+ c  d  d+ e  e+ f  g  g+ a  a+ b  ");
            sDisplcm = _T("-1 0  1  1  2  2  3  4  4  5  5  6  ");
            break;

        case earmLa:
        case earmFasm:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  " #FCG
            sPattern = _T("b+ c  d  d+ e  e+ f  fx g  a  a+ b  ");
            sDisplcm = _T("-1 0  1  1  2  2  3  3  4  5  5  6  ");
            break;

        case earmMi:
        case earmDosm:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  " #FCGD
            sPattern = _T("b+ c  cx d  e  e+ f  fx g  a  a+ b  ");
            sDisplcm = _T("-1 0  0  1  2  2  3  3  4  5  5  6  ");
            break;

        case earmSi:
        case earmSolsm:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  " #FCGDA
            sPattern = _T("b+ c  cx d  e  e+ f  fx g  gx a  b  ");
            sDisplcm = _T("-1 0  0  1  2  2  3  3  4  4  5  6  ");
            break;

        case earmFas:
        case earmResm:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  " #FCGDAE
            sPattern = _T("b+ c  cx d  dx e  f  fx g  gx a  b  ");
            sDisplcm = _T("-1 0  0  1  1  2  3  3  4  4  5  6  ");
            break;

        case earmDos:
        case earmLasm:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  " #FCGDAEB
            sPattern = _T("b  c  cx d  dx e  f  fx g  gx a  ax ");
            sDisplcm = _T("-1 0  0  1  1  2  3  3  4  4  5  5  ");
            break;

    //    //Bemoles -------------------------------------------
        case earmFa:
        case earmRem:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  "
            sPattern = _T("c  c+ d  d+ e  f  f+ g  g+ a  b  b= ");
            sDisplcm = _T("0  0  1  1  2  3  3  4  4  5  6  6  ");
            break;

        case earmSib:
        case earmSolm:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  "
            sPattern = _T("c  c+ d  e  e= f  f+ g  g+ a  b  b= ");
            sDisplcm = _T("0  0  1  2  2  3  3  4  4  5  6  6  ");
            break;

        case earmMib:
        case earmDom:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  "
            sPattern = _T("c  c+ d  e  e= f  f+ g  a  a= b  b= ");
            sDisplcm = _T("0  0  1  2  2  3  3  4  5  5  6  6  ");
            break;

        case earmLab:
        case earmFam:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  "
            sPattern = _T("c  d  d= e  e= f  f+ g  a  a= b  b= ");
            sDisplcm = _T("0  1  1  2  2  3  3  4  5  5  6  6  ");
            break;

        case earmReb:
        case earmSibm:
             //         "C  C# D  D# E  F  F# G  G# A  A# B  "
            sPattern = _T("c  d  d= e  e= f  g  g= a  a= b  b= ");
            sDisplcm = _T("0  1  1  2  2  3  4  4  5  5  6  6  ");
            break;

        case earmSolb:
        case earmMibm:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  "
            sPattern = _T("c= d  d= e  e= f  g  g= a  a= b  c  ");
            sDisplcm = _T("0  1  1  2  2  3  4  4  5  5  6  7  ");
            break;

        case earmDob:
        case earmLabm:
            //            "C  C# D  D# E  F  F# G  G# A  A# B  "
            sPattern = _T("c= d  d= e  f  f= g  g= a  a= b  c  ");
            sDisplcm = _T("0  1  1  2  2  3  4  4  5  5  6  7  ");
            break;

        default:
            wxASSERT(false);    // Tonal key error
    }
    
    int i = 3 * nResto + 1;
    long nShift;
    wxString sShift = sDisplcm.Mid(i-1, 1);
    bool fOK = sShift.ToLong(&nShift);
    wxASSERT(fOK);

    if (pPitch)    // if requested, return the diatonic pitch
        *pPitch = (lmPitch)(nOctave * 7 + 1) + (lmPitch)nShift;
    
    if (nShift == -1)
        nOctave--;
    else if (nShift == 7)
        nOctave++;
    wxString sOctave = wxString::Format(_T("%d"), nOctave);
    wxString sAnswer = sPattern.Mid(i, 2);        // alterations
    sAnswer.Trim();
    sAnswer += sPattern.Mid(i-1, 1);            // note name
    sAnswer += sOctave;                            // octave
    return sAnswer;
    
}

/*! Returns the note name and octave in the physics namespace
*/
wxString GetNoteNamePhysicists(lmPitch nPitch)
{
    static wxString sNoteName[7] = {
        _("c%d"), _("d%d"), _("e%d"), _("f%d"), _("g%d"), _("a%d"), _("b%d")
    };

    int nOctave = (int)(nPitch - 1) / 7;
    int iNote = (nPitch - 1) % 7;

    return wxString::Format(sNoteName[iNote], nOctave);

}
