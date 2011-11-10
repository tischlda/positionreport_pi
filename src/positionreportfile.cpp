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
#include <math.h>
#include "../../../include/georef.h"

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
  m_isSelected = false;
}

Station::~Station()
{
  for(size_t i = 0; i < m_positionReports->Count(); i++)
  {
    delete m_positionReports->Item(i);
  }
  delete m_positionReports;
}

static int CompareStations(Station *s1, Station *s2)
{
  return s1->m_callsign < s2->m_callsign ? -1 : 1;
}

Stations::Stations(void)
{
  m_stationArray = new StationArray(CompareStations);
}

Stations::~Stations(void)
{
  for(size_t i = 0; i < m_stationArray->Count(); i++)
  {
    delete m_stationArray->Item(i);
  }
  delete m_stationArray;
}

void Stations::Add(Station* station)
{
  m_stationArray->Add(station);
}

Station* Stations::Find(wxString& callsign)
{
  for(size_t i = 0; i < m_stationArray->Count(); i++)
  {
    if(m_stationArray->Item(i)->m_callsign == callsign)
    {
      return m_stationArray->Item(i);
    }
  }
  return NULL;
}

bool Stations::Select(double latitude, double longitude, double delta)
{
  bool changed = false;

  Station* station;
  PositionReport* positionReport;

  for(size_t i = 0; i < m_stationArray->Count(); i++)
  {
    station = m_stationArray->Item(i);

    positionReport = station->m_positionReports->Item(0);

    if(DistGreatCircle(positionReport->m_latitude, positionReport->m_longitude, latitude, longitude) <= delta)
    {
      if(!station->m_isSelected) changed = true;
      station->m_isSelected = true;
    }
    else
    {
      if(station->m_isSelected) changed = true;
      station->m_isSelected = false;
    }
  }

  return changed;
}

PositionReportFileReader::PositionReportFileReader(void)
{
}

Stations* PositionReportFileReader::Read(wxString& filename)
{
  wxFileName file(filename);
  
  if(file.FileExists() && (file.GetSize() < MaxFileSize)){
    wxFileInputStream stream(filename); 
    return Read(stream);
  } else {
    return NULL;
  }
}

Stations* PositionReportFileReader::Read(wxInputStream &stream)
{
  wxString line;
  wxString callsign;
  double distance, bearing;
  double latDeg, latMin, lat, lonDeg, lonMin, lon;
  wxDateTime dateTime;
  wxString comment;
  Stations *stations;
  Station *station;
  PositionReport *positionReport;

  stations = new Stations();
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

        station = stations->Find(callsign);
        if(!station)
        {
          station = new Station();
          station->m_callsign = callsign;
          stations->Add(station);
        }

        positionReport = new PositionReport();
        positionReport->m_distance = distance;
        positionReport->m_bearing = bearing;
        positionReport->m_comment = comment;
        positionReport->m_dateTime = dateTime;
        positionReport->m_latitude = lat;
        positionReport->m_longitude = lon;

        station->m_positionReports->Add(positionReport);
      }
    }
  }

  return stations;
};