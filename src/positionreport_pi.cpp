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

#include <wx/treectrl.h>
#include <wx/fileconf.h>

#include <typeinfo>
#include "positionreport_ui.h"
#include "positionreport_pi.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
  return (opencpn_plugin *)new positionreport_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
  delete p;
}

#include "icons.h"

positionreport_pi::positionreport_pi(void *ppimgr)
      :opencpn_plugin_18(ppimgr)
{
  initialize_images();

  m_dialog = NULL;
  m_stations = NULL;
  m_positionReportRenderer = NULL;
}

int positionreport_pi::Init(void)
{
  AddLocaleCatalog( _T("opencpn-positionreport_pi") );

  m_dialog_x = 0;
  m_dialog_y = 0;
  m_dialog_sx = 200;
  m_dialog_sy = 200;
  m_dir = wxT("");

  LoadConfig();

  m_parentWindow = GetOCPNCanvasWindow();
  
  m_leftclick_tool_id  = InsertPlugInTool(
    _T(""), _img_positionreport_pi, _img_positionreport_pi, wxITEM_NORMAL,
    _("PositionReport"), _T(""), NULL,
    POSITIONREPORT_TOOL_POSITION, 0, this);

  m_positionReportRenderer = new PositionReportRenderer();

  return (
    WANTS_OVERLAY_CALLBACK  |
    WANTS_CURSOR_LATLON     |
    WANTS_TOOLBAR_CALLBACK  |
    INSTALLS_TOOLBAR_TOOL   |
    WANTS_CONFIG);
}

bool positionreport_pi::DeInit(void)
{
  if(m_dialog) m_dialog->Close();
  if(m_stations) { delete m_stations; m_stations = NULL; }
  if(m_positionReportRenderer) { delete m_positionReportRenderer; m_positionReportRenderer = NULL; }

  return true;
}

int positionreport_pi::GetAPIVersionMajor()
{
  return MY_API_VERSION_MAJOR;
}

int positionreport_pi::GetAPIVersionMinor()
{
  return MY_API_VERSION_MINOR;
}

int positionreport_pi::GetPlugInVersionMajor()
{
  return PLUGIN_VERSION_MAJOR;
}

int positionreport_pi::GetPlugInVersionMinor()
{
  return PLUGIN_VERSION_MINOR;
}

wxBitmap *positionreport_pi::GetPlugInBitmap()
{
  return _img_positionreport_pi;
}
wxString positionreport_pi::GetCommonName()
{
  return _("PositionReport");
}

wxString positionreport_pi::GetShortDescription()
{
  return _("PositionReport PlugIn for OpenCPN");
}


wxString positionreport_pi::GetLongDescription()
{
  return _("PositionReport PlugIn for OpenCPN\n\
Display Winlink2000 position reports.\n\
");
}

int positionreport_pi::GetToolbarToolCount(void)
{
  return 1;
}

void positionreport_pi::ShowPreferencesDialog(wxWindow* parent)
{
}

void positionreport_pi::OnToolbarToolCallback(int id)
{
  if(!m_dialog)
  {
    m_dialog = new PositionReportUIDialog();
    m_dialog->Create(m_parentWindow, this, -1, 
      _("PositionReport Display Control"), m_dir,
      wxPoint(m_dialog_x, m_dialog_y), 
      wxSize(m_dialog_sx, m_dialog_sy));
  }

  m_dialog->Show();
}

void positionreport_pi::OnDialogClose()
{
  m_dialog = NULL;
  if(m_stations) { delete m_stations; m_stations = NULL; }
  SaveConfig();
}

bool positionreport_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
  wxLogMessage(_T("positionreport_pi::RenderOverlay"));
  if(m_stations)
  {
    return m_positionReportRenderer->RenderOverlay(dc, vp, m_stations);
  }
  else
  {
    return false;
  }
}

void positionreport_pi::SetCursorLatLon(double lat, double lon)
{

}

void positionreport_pi::FileSelected()
{
  if(m_stations) { delete m_stations; m_stations = NULL; }

  PositionReportFileReader reader;

  m_stations = reader.Read(m_dialog->GetCurrentFileName());

  m_dialog->OnDataChanged();

  RequestRefresh(m_parentWindow);
}

void positionreport_pi::StationSelected()
{
  for(size_t i = 0; i < m_stations->Count(); i++)
  {
    for(size_t j = 0; j < m_stations->Item(i)->m_positionReports->Count(); j++)
    {
      m_stations->Item(i)->m_positionReports->Item(j)->m_isSelected = false;
    }
  }

  Station *currentStation = m_stations->Find(m_dialog->GetCurrentStationName());

  if(currentStation)
  {
    for(size_t j = 0; j < currentStation->m_positionReports->Count(); j++)
    {
      currentStation->m_positionReports->Item(j)->m_isSelected = true;
    }
  }

  RequestRefresh(m_parentWindow);
}

bool positionreport_pi::LoadConfig(void)
{
  wxFileConfig *pConf = GetOCPNConfigObject();

  if(!pConf) return false;

  pConf->SetPath ( _T( "/Settings" ) );

  m_dialog_sx = pConf->Read ( _T ( "PositionReportDialogSizeX" ), 300L );
  m_dialog_sy = pConf->Read ( _T ( "PositionReportDialogSizeY" ), 540L );
  m_dialog_x =  pConf->Read ( _T ( "PositionReportDialogPosX" ), 20L );
  m_dialog_y =  pConf->Read ( _T ( "PositionReportDialogPosY" ), 170L );

  pConf->SetPath ( _T ( "/Directories" ) );
  pConf->Read ( _T ( "PositionReportDirectory" ), &m_dir );

  return true;
}

bool positionreport_pi::SaveConfig(void)
{
  wxFileConfig *pConf = GetOCPNConfigObject();

  if(!pConf) return false;

  pConf->SetPath ( _T ( "/Settings" ) );
  pConf->Write ( _T ( "PositionReportDialogSizeX" ),  m_dialog_sx );
  pConf->Write ( _T ( "PositionReportDialogSizeY" ),  m_dialog_sy );
  pConf->Write ( _T ( "PositionReportDialogPosX" ),   m_dialog_x );
  pConf->Write ( _T ( "PositionReportDialogPosY" ),   m_dialog_y );

  pConf->SetPath ( _T ( "/Directories" ) );
  pConf->Write ( _T ( "PositionReportDirectory" ), m_dir );

  return true;
}
