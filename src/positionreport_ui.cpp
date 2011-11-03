/******************************************************************************
 * $Id: PositionReport.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  PositionReport Plugin
 * Author:   David Tischler
 *
 ***************************************************************************
 *   Copyright (C) 2011 by David Tischler                                  *
 *   me@here.com                                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/mstream.h>
#include <typeinfo>
#include <wx/arrimpl.cpp>

#include "positionreport_pi.h"
#include "positionreport_ui.h"


IMPLEMENT_CLASS ( PositionReportUIDialog, wxDialog )

BEGIN_EVENT_TABLE ( PositionReportUIDialog, wxDialog )
  EVT_CLOSE ( PositionReportUIDialog::OnClose )
  EVT_BUTTON ( ID_OK, PositionReportUIDialog::OnIdOKClick )
  EVT_MOVE ( PositionReportUIDialog::OnMove )
  EVT_SIZE ( PositionReportUIDialog::OnSize )
  EVT_BUTTON ( ID_CHOOSEPOSITIONREPORTDIR, PositionReportUIDialog::OnChooseDirClick )
  EVT_LIST_ITEM_SELECTED ( ID_FILESELECTED,PositionReportUIDialog::OnFileSelect)
END_EVENT_TABLE()

PositionReportUIDialog::PositionReportUIDialog(void)
{
//      printf("GRIBUIDialog ctor\n");
      Init();
}

PositionReportUIDialog::~PositionReportUIDialog(void)
{

}
bool PositionReportUIDialog::Create ( wxWindow *parent, positionreport_pi *ppi, wxWindowID id,
                              const wxString& caption, const wxString initial_dir,
                              const wxPoint& pos, const wxSize& size, long style )
{
//      printf("GRIBUIDialog::Create\n");

      pParent = parent;
      pPlugIn = ppi;

      m_currentDir = initial_dir;

      //    As a display optimization....
      //    if current color scheme is other than DAY,
      //    Then create the dialog ..WITHOUT.. borders and title bar.
      //    This way, any window decorations set by external themes, etc
      //    will not detract from night-vision

      long wstyle = wxDEFAULT_FRAME_STYLE;
//      if( ( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY ) && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ) )
//            wstyle |= ( wxNO_BORDER );

      if( !wxDialog::Create ( parent, id, caption, pos, size, wstyle ) )
            return false;


//      wxColour back_color = GetGlobalColor ( _T ( "UIBDR" ) );
//      SetBackgroundColour ( back_color );

//      m_dFont = pFontMgr->GetFont ( _T ( "GRIBUIDialog" ), 10 );
//      SetFont ( *m_dFont );

//      SetForegroundColour ( pFontMgr->GetFontColor ( _T ( "GRIBUIDialog" ) ) );

      m_pfolder_bitmap = new wxBitmap ( folder );   // comes from XPM include

      CreateControls();

// This ensures that the dialog cannot be sized smaller than the minimum size
//      GetSizer()->SetSizeHints ( this );

      //Fit();
      SetMinSize(GetBestSize());

      return true;
}

void PositionReportUIDialog::OnClose ( wxCloseEvent& event )
{
      pPlugIn->SetDir(m_currentDir);
      RequestRefresh(pParent);
      Destroy();
      pPlugIn->OnDialogClose();
}

void PositionReportUIDialog::Invalidate(void){
}

void PositionReportUIDialog::OnIdOKClick ( wxCommandEvent& event )
{
      Close(); // this will call OnClose() later...
}

void PositionReportUIDialog::OnMove ( wxMoveEvent& event )
{
      //    Record the dialog position
      wxPoint p = event.GetPosition();
      pPlugIn->SetDialogX(p.x);
      pPlugIn->SetDialogY(p.y);

      event.Skip();
}

void PositionReportUIDialog::OnSize ( wxSizeEvent& event )
{
      //    Record the dialog size
      wxSize p = event.GetSize();
      pPlugIn->SetDialogSizeX(p.x);
      pPlugIn->SetDialogSizeY(p.y);

      event.Skip();
}



void PositionReportUIDialog::OnChooseDirClick ( wxCommandEvent& event )
{
      wxString new_dir  = ::wxDirSelector ( _( "Select PositionReport Directory" ), m_currentDir );
      if( !new_dir.empty() )
      {
            m_currentDir = new_dir;
            m_pitemCurrentDirectoryCtrl->ChangeValue ( m_currentDir );
            m_pitemCurrentDirectoryCtrl->SetInsertionPoint(0);
            updateFileList();

            Refresh();

            pPlugIn->SetDir(m_currentDir);
      }
}

void PositionReportUIDialog::CreateControls()
{
      int border_size = 4;
      int group_item_spacing = 1;           // use for items within one group, with Add(...wxALL)


// A top-level sizer
      wxBoxSizer* topSizer = new wxBoxSizer ( wxVERTICAL );
      SetSizer ( topSizer );


//    The Fleetode directory
      wxStaticBox* itemStaticBoxSizer11Static = new wxStaticBox ( this, wxID_ANY,_ ( "PositionReport File Directory" ) );
      wxStaticBoxSizer *itemStaticBoxSizer11 = new wxStaticBoxSizer ( itemStaticBoxSizer11Static, wxHORIZONTAL );
      topSizer->Add ( itemStaticBoxSizer11, 0, wxEXPAND );

      m_pitemCurrentDirectoryCtrl = new wxTextCtrl (this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
      itemStaticBoxSizer11->Add ( m_pitemCurrentDirectoryCtrl, 1, wxALIGN_LEFT|wxALL, 5 );

      m_pitemCurrentDirectoryCtrl->AppendText( m_currentDir );

      wxButton* bChooseDir = new wxBitmapButton ( this, ID_CHOOSEPOSITIONREPORTDIR, *m_pfolder_bitmap );
      itemStaticBoxSizer11->Add ( bChooseDir, 0, wxALIGN_RIGHT|wxALL, 5 );

      // file panel
      wxPanel *filepanel = new wxPanel( this, wxID_ANY,
            wxDefaultPosition, wxDefaultSize);
      topSizer->Add(filepanel, 1, wxGROW);

      wxBoxSizer* fpsizer = new wxBoxSizer(wxVERTICAL);
      filepanel->SetSizer(fpsizer);
            
      m_pFileListCtrl = new wxListCtrl(filepanel,ID_FILESELECTED,
            wxDefaultPosition,wxDefaultSize,wxLC_REPORT|wxLC_SINGLE_SEL);
      fpsizer->Add(m_pFileListCtrl, 1, wxGROW);

      // panels
      wxNotebook *itemNotebook = new wxNotebook( this, ID_NOTEBOOK, wxDefaultPosition,
      wxDefaultSize, wxNB_TOP );
      topSizer->Add(itemNotebook,1,wxGROW);
      
      // Text panel
      wxPanel *textpanel = new wxPanel( itemNotebook, wxID_ANY, 
            wxDefaultPosition, wxSize(-1, -1));
      itemNotebook->AddPage(textpanel, _("Text"));            

      wxBoxSizer* tpsizer = new wxBoxSizer(wxVERTICAL);
      textpanel->SetSizer(tpsizer);

      m_pTextCtrl = new wxTextCtrl( textpanel, 
            wxID_ANY, wxEmptyString,
            wxDefaultPosition,wxDefaultSize,
            wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL
                 );
      tpsizer->Add(m_pTextCtrl, 1, wxGROW);
      
      // Raw panel
      wxPanel *rawpanel = new wxPanel( itemNotebook, wxID_ANY, 
            wxDefaultPosition, wxSize(-1, -1));
      itemNotebook->AddPage(rawpanel, _("Raw"));            

      wxBoxSizer* rpsizer = new wxBoxSizer(wxVERTICAL);
      rawpanel->SetSizer(rpsizer);

      m_pRawCtrl = new wxTextCtrl( rawpanel, 
            ID_RAWTEXT, wxEmptyString,
            wxDefaultPosition,wxDefaultSize,
            wxTE_MULTILINE|wxHSCROLL
                 );
      rpsizer->Add(m_pRawCtrl, 1, wxGROW);

// A horizontal box sizer to contain OK
      wxBoxSizer* AckBox = new wxBoxSizer ( wxHORIZONTAL );
      topSizer->Add ( AckBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

// The OK button
      wxButton* bOK = new wxButton ( this, ID_OK, _( "&Close" ),
                                     wxDefaultPosition, wxDefaultSize, 0 );
      AckBox->Add ( bOK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

      updateFileList();
}

//---------------------------------------------------------
void PositionReportUIDialog::updateFileList(void){
      wxDateTime access, mod, create;
      m_pFileListCtrl->ClearAll();
      m_pFileListCtrl->InsertColumn(0, _T("Name"));
      m_pFileListCtrl->InsertColumn(1, _T("Date"));
      m_FilenameArray.Empty();
      size_t res = wxDir::GetAllFiles(m_currentDir,
            &m_FilenameArray,
            wxEmptyString,
            wxDIR_FILES);
      for(unsigned int i=0;i<m_FilenameArray.GetCount();i++){
            // remove dirname from file
            wxFileName file(m_FilenameArray[i]);
            m_FilenameArray[i]=file.GetFullName();
            file.GetTimes(&access, &mod, &create);
            m_pFileListCtrl->InsertItem(i, file.GetFullName());
            m_pFileListCtrl->SetItemData(i, i);
            m_pFileListCtrl->SetItem(i, 1, create.FormatDate());
            m_currentFileName=wxEmptyString;
      }
}

// a new file is selected, read and display this file
void PositionReportUIDialog::OnFileSelect(wxListEvent& event){
  // use the one first selected file
  long selectedItemIndex = m_pFileListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if(selectedItemIndex != -1) {
    wxFileName fn(m_currentDir, m_FilenameArray[m_pFileListCtrl->GetItemData(selectedItemIndex)]);
    m_currentFileName = fn.GetFullPath();
  } else {
    m_currentFileName = wxEmptyString;
  }

  // read File and redisplay it
  pPlugIn->FileSelected();
}

void PositionReportUIDialog::OnDataChanged(void)
{
  updateTextPanel();
}

wxString PositionReportUIDialog::GetCurrentFileName(void)
{
  return m_currentFileName;
}

void PositionReportUIDialog::updateRawPanel(wxString &rawData){
      m_pRawCtrl->Clear();
      m_pRawCtrl->AppendText(rawData);

      // if the control is visible during the update it scrolls to the end of the text
      m_pRawCtrl->ShowPosition(0);
}

void PositionReportUIDialog::updateTextPanel(void){
  m_pTextCtrl->Clear();

  PositionReportsHash *positionReportsHash = pPlugIn->GetPositionReports();

  if(positionReportsHash) {
    for(PositionReportsHash::iterator it = positionReportsHash->begin(); it != positionReportsHash->end(); ++it) {
      m_pTextCtrl->AppendText(wxString::Format(_T("%s %f"), it->second->m_callsign, it->second->m_positionReport->m_distance));
      m_pTextCtrl->AppendText(_T("\n"));
    }
  }
  
  // if the control is visible during the update it scrolls to the end of the text
  m_pTextCtrl->ShowPosition(0);
}

bool PositionReportRenderer::RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp, PositionReportsHash *positionReports)
{
  wxLogMessage(_T("RenderOverlay"));

  wxPoint point;
  wxCoord radius(5);

  for(PositionReportsHash::iterator it = positionReports->begin(); it != positionReports->end(); ++it)
  {
    GetCanvasPixLL(vp, &point, it->second->m_positionReport->m_latitude, it->second->m_positionReport->m_longitude);
    pmdc->DrawCircle(point, radius);
    pmdc->DrawText(it->second->m_callsign, point);
  }

  return true;
}