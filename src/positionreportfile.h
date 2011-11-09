/******************************************************************************
 * $Id: PositionReportFile .h, v1.0 2010/08/05 SethDart Exp $
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

#ifndef _POSITIONREPORTFILE_H_
#define _POSITIONREPORTFILE_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/dynarray.h>
#include <wx/object.h>
#include <wx/gdicmn.h>
#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!

#include "../../../include/ocpn_plugin.h"

class FileDescription
{
  public:
    FileDescription(wxString& name, wxDateTime& date) { m_name = name; m_date = date; }

    wxString   m_name;
    wxDateTime m_date;
};

WX_DEFINE_SORTED_ARRAY(FileDescription*, FileDescriptionArray);

class PositionReport
{
  public:
    PositionReport(void);

    wxDateTime   m_dateTime;
    double       m_latitude;
    double       m_longitude;
    double       m_distance; // in nautical miles
    double       m_bearing; // in deg
    unsigned int m_course; // in deg
    unsigned int m_speed;  // in knots, 99=unknown
    wxString     m_comment;
};

WX_DEFINE_SORTED_ARRAY(PositionReport*, PositionReports);

class Station
{
  public:
    Station(void);
    ~Station(void);

    wxString        m_callsign;
    PositionReports *m_positionReports;

    bool m_isSelected;
};

WX_DEFINE_SORTED_ARRAY(Station*, StationArray);

class Stations
{
  public:
    Stations(void);
    ~Stations(void);

    void Add(Station* station);
    Station* Find(wxString& callsign);

    Station* Item(size_t uiIndex) { return m_stationArray->Item(uiIndex); }
    size_t Count(void) { return m_stationArray->Count(); }

  private:
    StationArray* m_stationArray;
};

class PositionReportFileReader
{
public:
      PositionReportFileReader(void);
      
      Stations* Read(wxString& filename);
      Stations* Read(wxInputStream &stream);

      static const size_t MaxFileSize = 10000;
};
#endif