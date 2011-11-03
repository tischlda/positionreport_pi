/******************************************************************************
 * $Id: PositionReportFile.cpp, v1.0 2010/08/05 SethDart Exp $
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

#include <wx/filename.h>
#include <wx/txtstrm.h>
#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!

#include <ctype.h>

#include "PositionReportFile.h"

WX_DEFINE_OBJARRAY(GeoPoints);

// Is the given point in the vp ??
static bool PointInLLBox(PlugIn_ViewPort *vp, double x, double y)
{
    if(  x >= (vp->lon_min) && x <= (vp->lon_max) &&
            y >= (vp->lat_min) && y <= (vp->lat_max) )
            return TRUE;
    return FALSE;
}

PositionReport::PositionReport(void) {
}

PositionReports::PositionReports(void) {
}

PositionReports::~PositionReports() {
  if(m_positionReport) delete m_positionReport;
}

//---------------------------------------------
// PositionReportFileReader implementation
//---------------------------------------------
PositionReportFileReader::PositionReportFileReader(void) {
}

PositionReportsHash* PositionReportFileReader::Read(wxString& filename) {
  wxFileName file(filename);
  
  if(file.FileExists() && (file.GetSize() < MaxFileSize)){
    wxFileInputStream stream(filename); 
    return Read(stream);
  } else {
    wxLogMessage(_T("Can't open file"));
    return NULL;
  }
}

PositionReportsHash* PositionReportFileReader::Read(wxInputStream &stream) {
  wxString line;
  wxString callsign;
  double distance;
  PositionReportsHash *positionReportsHash;
  PositionReports *positionReports;
  PositionReport *positionReport;

  positionReportsHash = new PositionReportsHash();
  bool headerRead = false;

  wxTextInputStream text(stream);
  
  while(stream.IsOk() && !stream.Eof()) {
    line = text.ReadLine();
    
    if(!headerRead) {
      wxLogMessage(_T("Looking for header..."));
      if(line.StartsWith(_T("CALL ")))
        headerRead = true;
    }
    else if(headerRead) {
      if(!line.IsEmpty()) {
        callsign = line.Mid(0, 10);
        if(!line.Mid(10, 8).ToDouble(&distance)) distance = -1;

        positionReports = new PositionReports();
        positionReports->m_callsign = callsign;

        positionReport = new PositionReport();
        positionReport->m_distance = distance;

        positionReports->m_positionReport = positionReport;

        positionReportsHash->operator [](callsign) = positionReports;
      }
    }
  }

  return positionReportsHash;
}