/***************************************************************************
 * Project:  OpenCPN PositionReport Plugin
 * Author:   David Tischler
 *
 ***************************************************************************
 *   Copyright (C) 2011 by David Tischler                                  *
 *   david.tischler@gmx.at                                                 *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
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

static int SortFiles(FileDescription *fd1, FileDescription *fd2)
{
  return
    fd1->m_date.IsEqualTo(fd2->m_date) ? 0 :
    fd1->m_date.IsLaterThan(fd2->m_date) ? -1 : 1;
}

PositionReportUIDialog::PositionReportUIDialog(void)
{
  Init();
  m_fileDescriptions = new FileDescriptionArray(SortFiles);
}

PositionReportUIDialog::~PositionReportUIDialog(void)
{
  delete m_fileDescriptions;
}

bool PositionReportUIDialog::Create(wxWindow *parent, positionreport_pi *ppi, wxWindowID id,
                              const wxString& caption, const wxString initial_dir,
                              const wxPoint& pos, const wxSize& size, long style )
{
  m_parentWindow = parent;
  m_plugin = ppi;

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

  m_folderBitmap = new wxBitmap ( folder );   // comes from XPM include

  CreateControls();

  // This ensures that the dialog cannot be sized smaller than the minimum size
  //      GetSizer()->SetSizeHints ( this );

  //Fit();
  SetMinSize(GetBestSize());

  return true;
}

void PositionReportUIDialog::OnClose(wxCloseEvent& event)
{
  m_plugin->SetDir(m_currentDir);
  RequestRefresh(m_parentWindow);
  Destroy();
  m_plugin->OnDialogClose();
}

void PositionReportUIDialog::Invalidate(void)
{
}

void PositionReportUIDialog::OnButtonCloseClick(wxCommandEvent& event)
{
  Close(); // this will call OnClose() later...
}

void PositionReportUIDialog::OnMove(wxMoveEvent& event)
{
  wxPoint p = event.GetPosition();
  m_plugin->SetDialogX(p.x);
  m_plugin->SetDialogY(p.y);

  event.Skip();
}

void PositionReportUIDialog::OnSize(wxSizeEvent& event)
{
  wxSize p = event.GetSize();
  m_plugin->SetDialogSizeX(p.x);
  m_plugin->SetDialogSizeY(p.y);

  event.Skip();
}



void PositionReportUIDialog::OnChooseDirClick(wxCommandEvent& event)
{
  wxString new_dir  = ::wxDirSelector(_T( "Select PositionReport Directory" ), m_currentDir);
  if(!new_dir.empty())
  {
    m_currentDir = new_dir;
    m_pitemCurrentDirectoryCtrl->ChangeValue ( m_currentDir );
    m_pitemCurrentDirectoryCtrl->SetInsertionPoint(0);
    updateFileList();

    Refresh();

    m_plugin->SetDir(m_currentDir);
  }
}

static void AutoSizeColumns(wxListCtrl *listCtrl)
{
  int columnWidth = listCtrl->GetItemCount() > 0 ? wxLIST_AUTOSIZE : wxLIST_AUTOSIZE_USEHEADER;
  
  for(int i = 0; i < listCtrl->GetColumnCount(); i++)
  {
    listCtrl->SetColumnWidth(i, columnWidth);
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
  wxStaticBox* itemStaticBoxSizer11Static = new wxStaticBox (this, wxID_ANY, _T("Position Report Directory"));
  wxStaticBoxSizer *itemStaticBoxSizer11 = new wxStaticBoxSizer (itemStaticBoxSizer11Static, wxHORIZONTAL);
  topSizer->Add (itemStaticBoxSizer11, 0, wxEXPAND);

  m_pitemCurrentDirectoryCtrl = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  itemStaticBoxSizer11->Add(m_pitemCurrentDirectoryCtrl, 1, wxALIGN_LEFT|wxALL, 5);

  m_pitemCurrentDirectoryCtrl->AppendText(m_currentDir);

  wxButton* bChooseDir = new wxBitmapButton(this, ID_CHOOSEPOSITIONREPORTDIR, *m_folderBitmap);
  itemStaticBoxSizer11->Add(bChooseDir, 0, wxALIGN_RIGHT|wxALL, 5);

  // file panel
  wxPanel *filepanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
  topSizer->Add(filepanel, 1, wxGROW);

  wxBoxSizer* fpsizer = new wxBoxSizer(wxVERTICAL);
  filepanel->SetSizer(fpsizer);

  wxStaticBox* filePanelStaticBox = new wxStaticBox(filepanel, wxID_ANY, _T("File"));
  wxStaticBoxSizer* filePanelStaticBoxSizer = new wxStaticBoxSizer(filePanelStaticBox, wxVERTICAL);
  fpsizer->Add(filePanelStaticBoxSizer, 1, wxEXPAND|wxALL);

  m_pFileListCtrl = new wxListCtrl(filepanel, ID_FILELIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
  m_pFileListCtrl->InsertColumn(0, _T("Name"));
  m_pFileListCtrl->InsertColumn(1, _T("Date"));
  filePanelStaticBoxSizer->Add(m_pFileListCtrl, 1, wxGROW);

  
  // station panel
  wxPanel *stationpanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
  topSizer->Add(stationpanel, 1, wxGROW);

  wxBoxSizer* spsizer = new wxBoxSizer(wxVERTICAL);
  stationpanel->SetSizer(spsizer);

  wxStaticBox* stationPanelStaticBox = new wxStaticBox(stationpanel, wxID_ANY, _T("Latest Position Reports"));
  wxStaticBoxSizer* stationPanelStaticBoxSizer = new wxStaticBoxSizer(stationPanelStaticBox, wxVERTICAL);
  spsizer->Add(stationPanelStaticBoxSizer, 1, wxEXPAND|wxALL);

  m_pStationListCtrl = new wxListCtrl(stationpanel, ID_STATIONLIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
  m_pStationListCtrl->InsertColumn(0, _T("Station"));
  m_pStationListCtrl->InsertColumn(1, _T("Date"));
  m_pStationListCtrl->InsertColumn(2, _T("Latitude"));
  m_pStationListCtrl->InsertColumn(3, _T("Longitude"));
  m_pStationListCtrl->InsertColumn(4, _T("Comment"));
  stationPanelStaticBoxSizer->Add(m_pStationListCtrl, 1, wxGROW);


  // position report panel
  wxPanel *positionReportpanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
  topSizer->Add(positionReportpanel, 1, wxGROW);

  wxBoxSizer* ppsizer = new wxBoxSizer(wxVERTICAL);
  positionReportpanel->SetSizer(ppsizer);

  wxStaticBox* positionReportPanelStaticBox = new wxStaticBox(positionReportpanel, wxID_ANY, _T("All Position Reports"));
  wxStaticBoxSizer* positionReportPanelStaticBoxSizer = new wxStaticBoxSizer(positionReportPanelStaticBox, wxVERTICAL);
  ppsizer->Add(positionReportPanelStaticBoxSizer, 1, wxEXPAND|wxALL);
    
  m_pPositionReportListCtrl = new wxListCtrl(positionReportpanel, ID_STATIONLIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
  m_pPositionReportListCtrl->InsertColumn(0, _T("Date"));
  m_pPositionReportListCtrl->InsertColumn(1, _T("Latitude"));
  m_pPositionReportListCtrl->InsertColumn(2, _T("Longitude"));
  m_pPositionReportListCtrl->InsertColumn(3, _T("Comment"));
  positionReportPanelStaticBoxSizer->Add(m_pPositionReportListCtrl, 1, wxGROW);

  
  // A horizontal box sizer to contain OK
  wxBoxSizer* ackBox = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(ackBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  // The Close button
  wxButton* bClose = new wxButton(this, ID_BUTTONCLOSE, _T("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
  ackBox->Add(bClose, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  updateFileList();
}

void PositionReportUIDialog::updateFileList(void)
{
  wxDateTime access, mod, create;

  m_pFileListCtrl->DeleteAllItems();

  m_fileDescriptions->Clear();

  wxArrayString fileNames;

  size_t res = wxDir::GetAllFiles(m_currentDir, &fileNames, wxEmptyString, wxDIR_FILES);

  for(size_t i = 0; i < fileNames.GetCount(); i++)
  {
    wxFileName file(fileNames[i]);
    file.GetTimes(&access, &mod, &create);
    m_fileDescriptions->Add(new FileDescription(file.GetFullName(), create));
  }

  for(size_t i = 0; i < m_fileDescriptions->GetCount(); i++)
  {
    m_pFileListCtrl->InsertItem(i, m_fileDescriptions->Item(i)->m_name);
    m_pFileListCtrl->SetItemData(i, i);
    m_pFileListCtrl->SetItem(i, 1, m_fileDescriptions->Item(i)->m_date.FormatDate() + _T(" ") + m_fileDescriptions->Item(i)->m_date.FormatTime());
  }

  AutoSizeColumns(m_pFileListCtrl);

  m_currentFileName = wxEmptyString;
}

void PositionReportUIDialog::updateStationList(void)
{
  Station *station;
  PositionReport *positionReport;
  
  m_pStationListCtrl->DeleteAllItems();

  m_stationNameArray.Empty();

  Stations *stations = m_plugin->GetStations();
  
  if(stations)
  {
    for(size_t i = 0; i < stations->Count(); i++)
    {
      station = stations->Item(i);
      positionReport = station->m_positionReports->Item(0);

      i = m_stationNameArray.Add(station->m_callsign);

      m_pStationListCtrl->InsertItem(i, station->m_callsign);
      m_pStationListCtrl->SetItemData(i, i);
      m_pStationListCtrl->SetItem(i, 1, positionReport->m_dateTime.FormatDate() + _T(" ") + positionReport->m_dateTime.FormatTime());
      m_pStationListCtrl->SetItem(i, 2, wxString::Format(_T("%f"), positionReport->m_latitude));
      m_pStationListCtrl->SetItem(i, 3, wxString::Format(_T("%f"), positionReport->m_longitude));
      m_pStationListCtrl->SetItem(i, 4, positionReport->m_comment);

      //if(station->m_isSelected)
      //{
      //  m_pStationListCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      //}
    }
  }

  AutoSizeColumns(m_pStationListCtrl);

  m_currentStationName = wxEmptyString;
  updatePositionReportList();
}

void PositionReportUIDialog::updatePositionReportList(void)
{
  Stations *stations;
  Station *station;
  PositionReport *positionReport;
  
  m_pPositionReportListCtrl->DeleteAllItems();

  stations = m_plugin->GetStations();

  if(stations)
  {
    station = stations->Find(m_currentStationName);
    
    if(station)
    {
      for(size_t i = 0; i < station->m_positionReports->Count(); i++)
      {
        positionReport = station->m_positionReports->Item(i);

        m_pPositionReportListCtrl->InsertItem(i, positionReport->m_dateTime.FormatDate() + _T(" ") + positionReport->m_dateTime.FormatTime());
        m_pPositionReportListCtrl->SetItemData(i, i);
        m_pPositionReportListCtrl->SetItem(i, 1, wxString::Format(_T("%f"), positionReport->m_latitude));
        m_pPositionReportListCtrl->SetItem(i, 2, wxString::Format(_T("%f"), positionReport->m_longitude));
        m_pPositionReportListCtrl->SetItem(i, 3, positionReport->m_comment);
      }
    }
  }

  AutoSizeColumns(m_pPositionReportListCtrl);
}

void PositionReportUIDialog::updateStationListSelection(void)
{
 /* Station *station;
 
  Stations *stations = m_plugin->GetStations();
  
  if(stations)
  {
    for(size_t i = 0; i < stations->Count(); i++)
    {
      station = stations->Item(i);
      
      if(station->m_isSelected)
      {
        m_pStationListCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      }
      else
      {
        m_pStationListCtrl->SetItemState(i, 0, wxLIST_STATE_SELECTED);
      }
    }
  }
  
  m_currentStationName = wxEmptyString;*/
}

void PositionReportUIDialog::OnFileSelect(wxListEvent& event)
{
  long selectedItemIndex = m_pFileListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  
  if(selectedItemIndex != -1)
  {
    wxFileName fn(m_currentDir, m_fileDescriptions->Item(m_pFileListCtrl->GetItemData(selectedItemIndex))->m_name);
    m_currentFileName = fn.GetFullPath();
  }
  else
  {
    m_currentFileName = wxEmptyString;
  }

  m_plugin->FileSelected();
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

  updatePositionReportList();

  m_plugin->StationSelected();
}

void PositionReportUIDialog::OnDataChanged(void)
{
  updateStationList();
}

void PositionReportUIDialog::OnStationDataChanged(void)
{
  updateStationListSelection();
}

void PositionReportUIDialog::SetCurrentStationName(wxString& stationName)
{
  for(size_t i = 0; i < m_stationNameArray.Count(); i++)
  {
    if(m_stationNameArray[i] == stationName)
    {
      m_pStationListCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      wxListEvent ev(wxEVT_NULL, 0);
      OnStationSelect(ev);
      return;
    }
  }
}

bool PointInLLBox(PlugIn_ViewPort *vp, double latitude, double longitude)
{
    if(longitude >= (vp->lon_min) && longitude <= (vp->lon_max) &&
       latitude >= (vp->lat_min) && latitude <= (vp->lat_max))
            return true;
    
    if((longitude - 360.0) >= (vp->lon_min) && (longitude - 360.0) <= (vp->lon_max) &&
       latitude >= (vp->lat_min) && latitude <= (vp->lat_max))
            return true;

    return false;
}

PositionReportRenderer::PositionReportRenderer()
{
  m_configuration = new RendererConfiguration();
  m_configuration->ShowTracks = true;
  m_configuration->MaxPositions = 10;
  m_configuration->LabelCallsign = true;
  m_configuration->LabelDateTime = true;
}

bool PositionReportRenderer::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp, Stations *stations)
{
  bool hasDrawn = false;
  Station *station;

  if(stations)
  {
    for(size_t i = 0; i < stations->Count(); i++)
    {
      station = stations->Item(i);

      if(m_configuration->ShowTracks) hasDrawn |= DrawTrack(dc, vp, station);
      hasDrawn |= DrawPositions(dc, vp, station);
    }
  }

  return hasDrawn;
}

bool PositionReportRenderer::DrawTrack(wxDC &dc, PlugIn_ViewPort *vp, Station *station)
{
  bool hasDrawn = false;
  bool firstPosition = true;
  wxPoint point;
  wxPoint prevPoint;
  bool visible;
  bool prevVisible;
  PositionReport *positionReport;
  wxColour colour;

  for(size_t i = 0; i < station->m_positionReports->Count(); i++)
  {
    positionReport = station->m_positionReports->Item(i);
    GetCanvasPixLL(vp, &point, positionReport->m_latitude, positionReport->m_longitude);
    visible = PointInLLBox(vp, positionReport->m_latitude, positionReport->m_longitude);

    if(!firstPosition)
    {
      if(visible || prevVisible)
      {
        if(positionReport->m_isSelected)
          GetGlobalColor(_T("URED"), &colour);
        else
          GetGlobalColor(_T("GREY1"), &colour);
        
        wxPen pen(colour, 2, wxSOLID);
        dc.SetPen(pen);
        dc.DrawLine(prevPoint, point);
        hasDrawn = true;
      }
    }
    else
    {
      firstPosition = false;
    }

    prevPoint = point;
    prevVisible = visible;
  }

  return hasDrawn;
}

bool PositionReportRenderer::DrawPositions(wxDC &dc, PlugIn_ViewPort *vp, Station *station)
{
  bool hasDrawn = false;
  wxPoint point;
  wxPoint textPoint;
  wxCoord radius;
  wxColour colour;
  wxString label;
  wxSize size;
  PositionReport *positionReport;

  wxFont *font = wxTheFontList->FindOrCreateFont(8, wxFONTFAMILY_SWISS, wxNORMAL, wxFONTWEIGHT_NORMAL, FALSE, wxString (_T("Arial"))); 

  for(size_t i = 0; i < station->m_positionReports->Count(); i++)
  {
    positionReport = station->m_positionReports->Item(i);
    GetCanvasPixLL(vp, &point, positionReport->m_latitude, positionReport->m_longitude);

    if(PointInLLBox(vp, positionReport->m_latitude, positionReport->m_longitude))
    {
      if(positionReport->m_isSelected)
        GetGlobalColor(_T("URED"), &colour);
      else
        GetGlobalColor(_T("GREY1"), &colour);

      wxPen pen(colour, 2, wxSOLID);
      dc.SetPen(pen);

      GetGlobalColor(_T("GREY2"), &colour);
      wxBrush brush(colour, wxSOLID);
      dc.SetBrush(brush);

      if(i == 0)
      {
        dc.DrawCircle(point, 8);
        dc.DrawCircle(point, 1);
      }
      else
      {
        dc.DrawCircle(point, 5);
        dc.DrawCircle(point, 1);
      }

      GetGlobalColor(_T("UBLCK"), &colour);
      dc.SetTextForeground(colour);

      wxFont prevFont = dc.GetFont();
      dc.SetFont(*font);

      textPoint = point;
      
      if(m_configuration->LabelCallsign)
      {
        label = station->m_callsign;
        size = dc.GetTextExtent(label);
        dc.DrawText(label, textPoint);
        textPoint.y += size.GetHeight();
      }
      
      if(m_configuration->LabelDateTime)
      {
        label = positionReport->m_dateTime.FormatDate() + _T(" ") +
                positionReport->m_dateTime.FormatTime();
        size = dc.GetTextExtent(label);
        dc.DrawText(label, textPoint);
        textPoint.y += size.GetHeight();
      }
      
      dc.SetFont(prevFont);

      hasDrawn = true;
    }
  }

  return hasDrawn;
}