/******************************************************************************
 * $Id: PositionReport.h, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  PositionReport Plugin
 * Author:   David Tischler
 *
 ***************************************************************************
 *   Copyright (C) 2011 by David Tischler                               *
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

    wxString GetCurrentFileName(void) { return m_currentFileName; }
    wxString GetCurrentStationName(void) { return m_currentStationName; }

  private:
    void Invalidate(void);
    void updateFileList(void);
    void updateStationList(void);
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

class PositionReportRenderer
{
  public:
    bool RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp, StationHash *stationHash);
};

#endif