/******************************************************************************
 * $Id: positionreport_pi.h, v1.0 2010/08/05 SethDart Exp $
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

#ifndef _POSITIONREPORTPI_H_
#define _POSITIONREPORTPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#define     PLUGIN_VERSION_MAJOR    0
#define     PLUGIN_VERSION_MINOR    1

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    5

#include "../../../include/ocpn_plugin.h"
#include "positionreportfile.h"
#include "positionreport_ui.h"

#define POSITIONREPORT_TOOL_POSITION -1          // Request default positioning of toolbar tool

class PositionReportUIDialog;
class PositionReportRenderer;

class positionreport_pi : public opencpn_plugin
{
  public:
    positionreport_pi(void *ppimgr);

    int Init(void);
    bool DeInit(void);

    int GetAPIVersionMajor();
    int GetAPIVersionMinor();
    int GetPlugInVersionMajor();
    int GetPlugInVersionMinor();
    wxBitmap *GetPlugInBitmap();
    wxString GetCommonName();
    wxString GetShortDescription();
    wxString GetLongDescription();

    bool RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp);
    void SetCursorLatLon(double lat, double lon);

    int GetToolbarToolCount(void);
    void ShowPreferencesDialog(wxWindow* parent);
    void OnToolbarToolCallback(int id);
    void SetDialogX    (int x) { m_dialog_x = x; };
    void SetDialogY    (int x) { m_dialog_y = x; }
    void SetDialogSizeX(int x) { m_dialog_sx = x; }
    void SetDialogSizeY(int x) { m_dialog_sy = x; }

    void OnDialogClose();
    void SetDir(wxString dir){ m_dir = dir; }
    void FileSelected(void);
    void StationSelected(void);
    Stations* GetStations(void) { return m_stations; }

  private:
    bool LoadConfig(void);
    bool SaveConfig(void);

  private:
    wxWindow         *m_parentWindow;
    int               m_leftclick_tool_id;
    
    bool              m_showIcon;
    int               m_dialog_x, m_dialog_y;
    int               m_dialog_sx, m_dialog_sy;
    wxString          m_dir;
    
    PositionReportUIDialog *m_dialog;
    PositionReportRenderer *m_positionReportRenderer;
    
    Stations  *m_stations;
};

#endif