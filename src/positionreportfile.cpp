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

#include <ctype.h>

#include "PositionReportFile.h"

WX_DEFINE_OBJARRAY(GeoPoints);

static int ComparePositionReports(PositionReport *p1, PositionReport *p2)
{
  return
    p1->m_dateTime.IsEqualTo(p2->m_dateTime) ? 0 :
    p1->m_dateTime.IsEarlierThan(p2->m_dateTime) ? -1 : 1;
}

PositionReport::PositionReport(void)
{
}

Station::Station(void)
{
  m_positionReports = new PositionReports(ComparePositionReports);
}

Station::~Station()
{
  if(m_positionReports) delete m_positionReports;
}

PositionReportFileReader::PositionReportFileReader(void)
{
}

StationHash* PositionReportFileReader::Read(wxString& filename)
{
  wxFileName file(filename);
  
  if(file.FileExists() && (file.GetSize() < MaxFileSize)){
    wxFileInputStream stream(filename); 
    return Read(stream);
  } else {
    return NULL;
  }
}

StationHash* PositionReportFileReader::Read(wxInputStream &stream)
{
  wxString line;
  wxString callsign;
  double distance, bearing;
  double latDeg, latMin, lat, lonDeg, lonMin, lon;
  wxDateTime dateTime;
  wxString comment;
  StationHash *stationHash;
  Station *station;
  PositionReport *positionReport;

  stationHash = new StationHash();
  bool headerRead = false;

  wxTextInputStream text(stream);
  
  while(stream.IsOk() && !stream.Eof()) {
    line = text.ReadLine();
    
    if(!headerRead)
    {
      if(line.StartsWith(_T("CALL ")))
        headerRead = true;
    }
    else if(headerRead)
    {
      if(line.length() > 68)
      {
        callsign = line.Mid(0, 10);
        if(!line.Mid(10, 8).ToDouble(&distance)) distance = -1;
        if(!line.Mid(21, 3).ToDouble(&bearing)) bearing = -1;

        if(line.Mid(27, 2).ToDouble(&latDeg) &&
           line.Mid(30, 5).ToDouble(&latMin))
        {
             lat = (latDeg + (latMin / 60)) * (line.Mid(35, 1) == _T("N") ? 1 : -1);
        }

        if(line.Mid(37, 3).ToDouble(&lonDeg) &&
           line.Mid(41, 5).ToDouble(&lonMin))
        {
             lon = (lonDeg + (lonMin / 60)) * (line.Mid(46, 1) == _T("E")  ? 1 : -1);
        }

        dateTime.ParseDate(line.Mid(49, 16));
        comment = (line.Mid(66));

        StationHash::iterator it = stationHash->find(callsign);

        if(it != stationHash->end())
        {
          station = it->second;
        }
        else
        {
          station = new Station();
          station->m_callsign = callsign;
        }

        positionReport = new PositionReport();
        positionReport->m_distance = distance;
        positionReport->m_bearing = bearing;
        positionReport->m_comment = comment;
        positionReport->m_dateTime = dateTime;
        positionReport->m_latitude = lat;
        positionReport->m_longitude = lon;

        station->m_positionReports->Add(positionReport);

        stationHash->operator [](callsign) = station;
      }
    }
  }

  return stationHash;
}