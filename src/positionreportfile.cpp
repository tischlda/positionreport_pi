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
#include <wx/tokenzr.h>

#include <ctype.h>

#include "PositionReportFile.h"
#include <math.h>
#include "../../../include/georef.h"

static int ComparePositionReports(PositionReport *p1, PositionReport *p2)
{
  return
    p1->m_dateTime.IsEqualTo(p2->m_dateTime) ? 0 :
    p1->m_dateTime.IsEarlierThan(p2->m_dateTime) ? 1 : -1;
}

PositionReport::PositionReport(void)
{
  m_isSelected = false;
}

Station::Station(void)
{
  m_positionReports = new PositionReports(ComparePositionReports);
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

    for(size_t j = 0; j < station->m_positionReports->Count(); j++)
    {
      positionReport = station->m_positionReports->Item(j);

      if(DistGreatCircle(positionReport->m_latitude, positionReport->m_longitude, latitude, longitude) <= delta)
      {
        if(!positionReport->m_isSelected) changed = true;
        positionReport->m_isSelected = true;
      }
      else
      {
        if(positionReport->m_isSelected) changed = true;
        positionReport->m_isSelected = false;
      }
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
  wxTextInputStream text(stream);
  wxString line;

  while(stream.IsOk() && !stream.Eof())
  {
    line = text.ReadLine();

    if(line.StartsWith(_T("List of users nearby")))
    {
      return ReadNearbyFile(stream, text);
    }

    if(line.StartsWith(_T("Automated Reply Message from Winlink 2000 Position Report Processor")))
    {
      return ReadPositionRequestResponseFile(stream, text);
    }
  }

  return NULL;
}

Stations* PositionReportFileReader::ReadNearbyFile(wxInputStream &stream, wxTextInputStream &text)
{
  wxString line;
  bool headerRead = false;

  Stations *stations;
  Station *station;
  PositionReport *positionReport;

  wxString callsign;
  double distance, bearing;
  double latDeg, latMin, lat, lonDeg, lonMin, lon;
  wxDateTime dateTime;
  wxString comment;


  stations = new Stations();
  
  while(stream.IsOk() && !stream.Eof())
  {
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
}

Stations* PositionReportFileReader::ReadPositionRequestResponseFile(wxInputStream &stream, wxTextInputStream &text)
{
  wxString line;

  Stations *stations;

  stations = new Stations();
  
  text.ReadLine();
  text.ReadLine();

  while(stream.IsOk() && !stream.Eof())
  {
    ReadNextPositionFromPositionRequestResponseFile(stream, text, stations);
  }

  return stations;
}

void PositionReportFileReader::ReadNextPositionFromPositionRequestResponseFile(wxInputStream &stream, wxTextInputStream &text, Stations* stations)
{
  wxString line;
  bool headerRead = false;

  Station *station;
  PositionReport *positionReport;

  wxString callsign;
  double distance = -1, bearing = -1;
  double latDeg, latMin, lat, lonDeg, lonMin, lon;
  wxDateTime dateTime;
  wxString comment = _T("");

  while(stream.IsOk() && !stream.Eof())
  {
    line = text.ReadLine();

    if(line.IsEmpty())
    {
      if(headerRead)
      {
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

      return;
    }

    if(!headerRead)
    {
      headerRead = true;

      wxArrayString tokens = ::wxStringTokenize(line, _T(" "));

      callsign = tokens.Item(0);

      if(tokens.Item(3).Mid(0, 2).ToDouble(&latDeg) &&
         tokens.Item(3).Mid(3, 5).ToDouble(&latMin))
      {
           lat = (latDeg + (latMin / 60)) * (line.Mid(35, 1) == _T("N") ? 1 : -1);
      }

      if(tokens.Item(4).Mid(0, 3).ToDouble(&lonDeg) &&
         tokens.Item(4).Mid(4, 5).ToDouble(&lonMin))
      {
           lon = (lonDeg + (lonMin / 60)) * (line.Mid(46, 1) == _T("E")  ? 1 : -1);
      }

      dateTime.ParseDate(tokens.Item(1) + _T(" ") + tokens.Item(2));
    }
    else if(line.StartsWith(_T("Comment: ")))
    {
      comment = line.Mid(9);
    }
  }
}