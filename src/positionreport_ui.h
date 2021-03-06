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

#ifndef _POSITIONREPORTUI_H_
#define _POSITIONREPORTUI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/treectrl.h>
#include <wx/listctrl.h>
#include <wx/fileconf.h>
#include <wx/notebook.h>
#include <wx/textfile.h>
#include <wx/arrstr.h>
#include <wx/dynarray.h>

#include "folder.xpm"
#include "../../../include/ocpn_plugin.h"
#include "positionreport_pi.h"
#include "positionreportfile.h"

class positionreport_pi;

enum {
  ID_BUTTONCLOSE = 11001,
  ID_CHOOSEPOSITIONREPORTDIR,
  ID_FILELIST,
  ID_STATIONLIST,
  ID_NOTEBOOK,
  ID_RAWTEXT
};

class PositionReportUIDialog: public wxDialog
{
  DECLARE_CLASS(PositionReportUIDialog)
        DECLARE_EVENT_TABLE()

  public:
    PositionReportUIDialog(void);
    ~PositionReportUIDialog(void);
    bool Create(wxWindow *parent, positionreport_pi *ppi, wxWindowID id = wxID_ANY,
                const wxString& caption = _("PositionReport Display Control"), const wxString initial_dir = wxT(""),
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU);
    void CreateControls();
    void OnClose(wxCloseEvent& event);
    void OnButtonCloseClick(wxCommandEvent& event);
    void OnMove(wxMoveEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnChooseDirClick(wxCommandEvent& event);
    void OnFileSelect(wxListEvent& event);
    void OnStationSelect(wxListEvent& event);

    void OnDataChanged(void);
    void OnStationDataChanged(void);

    wxString GetCurrentFileName(void) { return m_currentFileName; }
    wxString GetCurrentStationName(void) { return m_currentStationName; }
    void SetCurrentStationName(wxString& stationName);

  private:
    void Invalidate(void);
    void updateFileList(void);
    void updateStationList(void);
    void updatePositionReportList(void);
    void updateStationListSelection(void);
    void updateRawPanel(wxString &awData);
    void updateTextPanel(void);

    private:
    wxWindow          *m_parentWindow;
    positionreport_pi *m_plugin;
    wxString           m_currentDir;
    wxBitmap          *m_folderBitmap;
    FileDescriptionArray *m_fileDescriptions;
    wxString           m_currentFileName;
    wxArrayString      m_stationNameArray;
    wxString           m_currentStationName;

    // the Contols that will get updated       
    wxTextCtrl        *m_pitemCurrentDirectoryCtrl;
    wxListCtrl        *m_pFileListCtrl;
    wxListCtrl        *m_pStationListCtrl;
    wxListCtrl        *m_pPositionReportListCtrl;
    wxTextCtrl        *m_pTextCtrl;
    wxTextCtrl        *m_pRawCtrl;
};

class StationList : wxListCtrl
{
  public:
    wxString GetCurrentStationName(void) { return m_currentStationName; }

 private:
    wxArrayString      m_stationNameArray;
    wxString           m_currentStationName;
};

class RendererConfiguration
{
  public:
    bool  ShowTracks;
    int   MaxPositions;

    bool  LabelCallsign;
    bool  LabelDateTime;
};

class PositionReportRenderer
{
  public:
    PositionReportRenderer();
    bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp, Stations *stations);

  private:
    RendererConfiguration *m_configuration;

    bool DrawTrack(wxDC &dc, PlugIn_ViewPort *vp, Station *station);
    bool DrawPositions(wxDC &dc, PlugIn_ViewPort *vp, Station *station);
};

#endif