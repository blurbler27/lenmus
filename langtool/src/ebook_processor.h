//--------------------------------------------------------------------------------------
//    LenMus project: free software for music theory and language
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
#ifdef __GNUG__
// #pragma interface
#endif

#ifndef __EBOOK_PROCESSOR_H__        //to avoid nested includes
#define __EBOOK_PROCESSOR_H__

#include "wx/wfstream.h"
#include "wx/hashmap.h"

// declare a hash map with string keys and int values
WX_DECLARE_STRING_HASH_MAP( int, ltPagesTable );

class wxXml2Node;
class wxZipOutputStream;
class wxTextOutputStream;
class wxFFileOutputStream;

enum {      //processing options
    lmPO_FILE = 1,      //generate PO file
};

class lmEbookProcessor
{
public:
    lmEbookProcessor();
    ~lmEbookProcessor();

    bool GenerateLMB(wxString sfilename, int nOptions=0);

    static wxString GetLibxml2Version();

private:
  
    // Tags' processors
    bool BookTag(const wxXml2Node& oNode);
    bool BookinfoTag(const wxXml2Node& oNode);
    bool ChapterTag(const wxXml2Node& oNode);
    bool EmphasisTag(const wxXml2Node& oNode);
    bool ExerciseTag(const wxXml2Node& oNode);
    bool ExerciseParamTag(const wxXml2Node& oNode, bool fTranslate);
    bool ItemizedlistTag(const wxXml2Node& oNode);
    bool LinkTag(const wxXml2Node& oNode);
    bool ListitemTag(const wxXml2Node& oNode);
    bool ParaTag(const wxXml2Node& oNode);
    bool PartTag(const wxXml2Node& oNode);
    bool ScoreTag(const wxXml2Node& oNode);
    bool SectionTag(const wxXml2Node& oNode);
    bool ThemeTag(const wxXml2Node& oNode);
    bool TitleTag(const wxXml2Node& oNode);


    // Parsing methods
    bool ProcessChildAndSiblings(const wxXml2Node& oNode, int nWriteOptions=0);
    bool ProcessChildren(const wxXml2Node& oNode, int nWriteOptions=0);
    bool ProcessTag(const wxXml2Node& oNode);

    // File generation methods
        // TOC
    bool StartTocFile(wxString sFilename);
    void TerminateTocFile();
    void WriteToToc(wxString sText, bool fIndent=true);
        // HTML
    bool StartHtmlFile(wxString sFilename, wxString sId);
    void TerminateHtmlFile();
    void WriteToHtml(wxString sText);
        // PO
    bool StartPoFile(wxString sFilename);
    void WriteToPo(wxString& sText);
        // LMB
    bool StartLmbFile(wxString sFilename);
    void TerminateLmbFile();
    void CopyToLmb(wxString sFilename);


    // Debug methods
    void DumpXMLTree(const wxXml2Node& oNode);
    void DumpNodeAndSiblings(const wxXml2Node& oNode, wxString& sTree, int n);
    void DumpNode(const wxXml2Node& oNode, wxString& sTree, int n);

    // Other methods
    void CreateLinksTable(wxXml2Node& oRoot);
    void FindThemeNode(const wxXml2Node& oNode);

    // member variables

    //files
    wxString        m_sFilename;            // full path & name of xml file being processed
    wxFile*         m_pTocFile;
    wxFile*         m_pHtmlFile;
    wxFile*         m_pPoFile;

    // variables for toc processing
    int             m_nTocIndentLevel;      // to indent output
    bool            m_fTitleToToc;          // write title to toc
    wxString        m_sTocFilename;

    // variables for html processing
    int             m_nHtmlIndentLevel;     // to indent output
    int             m_nNumHtmlPage;         // to generate html file number
    wxString        m_sHtmlPagename;        
    int             m_nHeaderLevel;

    // variables for idx processing

    // variables for PO files processing
    bool            m_fOnlyPoFile;          //only generate the PO file

    // variables for LMB (zip file) generation
    bool                    m_fGenerateLmb;
    wxTextOutputStream*     m_pLmbFile;
    wxZipOutputStream*      m_pZipFile;
    wxFFileOutputStream*    m_pZipOutFile;


    // bookinfo data
    bool            m_fProcessingBookinfo;
    wxString        m_sBookTitle;

    // page/id cross-reference table
    ltPagesTable    m_PagesIds;

};


#endif    // __EBOOK_PROCESSOR_H__