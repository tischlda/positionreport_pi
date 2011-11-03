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


IMPLEMENT_CLASS(PositionReportUIDialog, wxDialog)

BEGIN_EVENT_TABLE( PositionReportUIDialog, wxDialog)
  EVT_CLOSE   (PositionReportUIDialog::OnClose)
  EVT_BUTTON  (ID_BUTTONCLOSE, PositionReportUIDialog::OnButtonCloseClick)
  EVT_MOVE    (PositionReportUIDialog::OnMove)
  EVT_SIZE    (PositionReportUIDialog::OnSize)
  EVT_BUTTON  (ID_CHOOSEPOSITIONREPORTDIR, PositionReportUIDialog::OnChooseDirClick )
  EVT_LIST_ITEM_SELECTED (ID_FILELIST, PositionReportUIDialog::OnFileSelect)
  EVT_LIST_ITEM_SELECTED (ID_STATIONLIST, PositionReportUIDialog::OnStationSelect)
END_EVENT_TABLE()

PositionReportUIDialog::PositionReportUIDialog(void)
{
  Init();
}

PositionReportUIDialog::~PositionReportUIDialog(void)
{

}
bool PositionReportUIDialog::Create (wxWindow *parent, positionreport_pi *ppi, wxWindowID id,
                              const wxString& caption, const wxString initial_dir,
                              const wxPoint& pos, const wxSize& size, long style )
{
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

void PositionReportUIDialog::Invalidate(void)
{
}

void PositionReportUIDialog::OnButtonCloseClick ( wxCommandEvent& event )
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
  wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(topSizer);


  // The Position Report directory
  wxStaticBox* itemStaticBoxSizer11Static = new wxStaticBox (this, wxID_ANY, _T("PositionReport File Directory"));
  wxStaticBoxSizer *itemStaticBoxSizer11 = new wxStaticBoxSizer (itemStaticBoxSizer11Static, wxHORIZONTAL);
  topSizer->Add (itemStaticBoxSizer11, 0, wxEXPAND);

  m_pitemCurrentDirectoryCtrl = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  itemStaticBoxSizer11->Add(m_pitemCurrentDirectoryCtrl, 1, wxALIGN_LEFT|wxALL, 5);

  m_pitemCurrentDirectoryCtrl->AppendText(m_currentDir);

  wxButton* bChooseDir = new wxBitmapButton(this, ID_CHOOSEPOSITIONREPORTDIR, *m_pfolder_bitmap);
  itemStaticBoxSizer11->Add(bChooseDir, 0, wxALIGN_RIGHT|wxALL, 5);

  // file panel
  wxPanel *filepanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
  topSizer->Add(filepanel, 1, wxGROW);

  wxBoxSizer* fpsizer = new wxBoxSizer(wxVERTICAL);
  filepanel->SetSizer(fpsizer);
        
  m_pFileListCtrl = new wxListCtrl(filepanel, ID_FILELIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
  m_pFileListCtrl->InsertColumn(0, _T("Name"));
  m_pFileListCtrl->InsertColumn(1, _T("Date"));
  fpsizer->Add(m_pFileListCtrl, 1, wxGROW);

  
  // station panel
  wxPanel *stationpanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
  topSizer->Add(stationpanel, 1, wxGROW);

  wxBoxSizer* spsizer = new wxBoxSizer(wxVERTICAL);
  stationpanel->SetSizer(spsizer);

  m_pStationListCtrl = new wxListCtrl(stationpanel, ID_STATIONLIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
  m_pStationListCtrl->InsertColumn(0, _T("Station"));
  m_pStationListCtrl->InsertColumn(1, _T("Date"));
  m_pStationListCtrl->InsertColumn(2, _T("Latitude"));
  m_pStationListCtrl->InsertColumn(3, _T("Longitue"));
  m_pStationListCtrl->InsertColumn(4, _T("Comment"));
  spsizer->Add(m_pStationListCtrl, 1, wxGROW);

  m_pStationListCtrl->

  // A horizontal box sizer to contain OK
  wxBoxSizer* ackBox = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(ackBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  // The Close button
  wxButton* bClose = new wxButton(this, ID_BUTTONCLOSE, _T("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
  ackBox->Add(bClose, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  updateFileList();
}

//---------------------------------------------------------
void PositionReportUIDialog::updateFileList(void){
  wxDateTime access, mod, create;

  m_pFileListCtrl->DeleteAllItems();

  m_fileNameArray.Empty();

  size_t res = wxDir::GetAllFiles(m_currentDir, &m_fileNameArray, wxEmptyString, wxDIR_FILES);

  for(unsigned int i = 0; i < m_fileNameArray.GetCount(); i++)
  {
    // remove dirname from file
    wxFileName file(m_fileNameArray[i]);
    m_fileNameArray[i] = file.GetFullName();
    file.GetTimes(&access, &mod, &create);

    m_pFileListCtrl->InsertItem(i, file.GetFullName());
    m_pFileListCtrl->SetItemData(i, i);
    m_pFileListCtrl->SetItem(i, 1, create.FormatDate());
  }
  
  m_currentFileName = wxEmptyString;
}

void PositionReportUIDialog::updateStationList(void){
  int i;
  Station *station;
  PositionReport *positionReport;
  
  m_pStationListCtrl->DeleteAllItems();

  m_stationNameArray.Empty();

  StationHash *stationHash = pPlugIn->GetStation();
  
  if(stationHash) {
    for(StationHash::iterator it = stationHash->begin(); it != stationHash->end(); ++it) {
      station = it->second;
      positionReport = station->m_positionReport;

      i = m_stationNameArray.Add(station->m_callsign);

      m_pStationListCtrl->InsertItem(i, station->m_callsign);
      m_pStationListCtrl->SetItemData(i, i);
      m_pStationListCtrl->SetItem(i, 1, positionReport->m_dateTime.FormatDate());
      m_pStationListCtrl->SetItem(i, 2, wxString::Format(_T("%f"), positionReport->m_latitude));
      m_pStationListCtrl->SetItem(i, 3, wxString::Format(_T("%f"), positionReport->m_longitude));
      m_pStationListCtrl->SetItem(i, 4, positionReport->m_comment);
    }
  }
  
  m_currentStationName = wxEmptyString;
}

void PositionReportUIDialog::OnFileSelect(wxListEvent& event)
{
  long selectedItemIndex = m_pFileListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  
  if(selectedItemIndex != -1)
  {
    wxFileName fn(m_currentDir, m_fileNameArray[m_pFileListCtrl->GetItemData(selectedItemIndex)]);
    m_currentFileName = fn.GetFullPath();
  }
  else
  {
    m_currentFileName = wxEmptyString;
  }

  pPlugIn->FileSelected();
}

void PositionReportUIDialog::OnStationSelect(wxListEvent& event)
{
  long selectedItemIndex = m_pStationListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

  if(selectedItemIndex != -1)
  {
    m_currentStationName = m_stationNameArray[m_pStationListCtrl->GetItemData(selectedItemIndex)];
  }
  else
  {
    m_currentStationName = wxEmptyString;
  }

  pPlugIn->StationSelected();
}

void PositionReportUIDialog::OnDataChanged(void)
{
  updateStationList();
}

bool PositionReportRenderer::RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp, StationHash *stationHash)
{
  bool hasDrawn = false;
  wxPoint point;
  wxCoord radius(5);
  wxColour colour;

  if(stationHash)
  {
    for(StationHash::iterator it = stationHash->begin(); it != stationHash->end(); ++it)
    {
      GetCanvasPixLL(vp, &point, it->second->m_positionReport->m_latitude, it->second->m_positionReport->m_longitude);

      if(it->second->m_isSelected)
      {
        GetGlobalColor(_T("RED1"), &colour);
      }
      else
      {
        GetGlobalColor(_T("UBLCK"), &colour);
      }
      
      wxPen pen(colour, 2, wxSOLID);
      pmdc->SetPen(pen);

      pmdc->DrawCircle(point, radius);
      pmdc->DrawText(it->second->m_callsign, point);

      hasDrawn = true;
    }
  }

  return hasDrawn;
}