/* Copyright © 2001-2014, Canal TP and/or its affiliates. All rights reserved.
  
This file is part of Navitia,
    the software to build cool stuff with public transport.
 
Hope you'll enjoy and contribute to this project,
    powered by Canal TP (www.canaltp.fr).
Help us simplify mobility and open public transport:
    a non ending quest to the responsive locomotion way of traveling!
  
LICENCE: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
   
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.
   
You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
  
Stay tuned using
twitter @navitia 
IRC #navitia on freenode
https://groups.google.com/d/forum/navitia
www.navitia.io
*/

#include "navisu.h"
#include "visu/ui_navisu.h"

#include <iostream>
#include <QFileDialog>
#include <QErrorMessage>
using namespace navitia::type;
navisu::navisu(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::navisu)
{
    ui->setupUi(this);
  //  MyMarbleWidget * my = new MyMarbleWidget(ui->carto);
    ui->my->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
    ui->my->show();
  //  ui->my->setHome(2.36, 48.84, 2500);

}

void MyMarbleWidget::customPaint(GeoPainter* painter)
{
    GeoDataCoordinates home(8.4, 49.0, 0.0, GeoDataCoordinates::Degree);
    painter->setPen(Qt::green);
    painter->drawEllipse(home, 7, 7);
    painter->setPen(Qt::black);
    painter->drawText(home, "Hello Marble!");
}

template<class T>
std::string formatHeader(const T & t){
    std::stringstream ss;
    ss << t.name << "(ext=" << t.uri << "; id=" << t.id << "; idx=" << t.idx << ")";
    return ss.str();
}

void navisu::menuAction(QAction * action){
    if(action == ui->actionOuvrir){
        QString filename;
        try{
            filename = QFileDialog::getOpenFileName(this, "Ouvrir un fichier de données NAViTiA2");
            ui->statusbar->showMessage("Loading " + filename + "...");
            d.load_lz4(filename.toStdString());
            ui->statusbar->showMessage("Loading done " + filename);
        }catch(const std::exception &e){
            QErrorMessage * err = new QErrorMessage(this);
            err->showMessage(QString("Impossible d'ouvrir") + filename + " : " + e.what());
            ui->statusbar->showMessage("Load error " + filename);
            ui->tableWidget->clear();
        }catch(...){
            QErrorMessage * err = new QErrorMessage(this);
            err->showMessage(QString("Impossible d'ouvrir") + filename + " : exception inconnue");
            ui->statusbar->showMessage("Load error " + filename);
            ui->tableWidget->clear();
        }
    }
}

navisu::~navisu()
{
    delete ui;
}

void navisu::resetTable(int nb_cols, int nb_rows){
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(nb_cols);
    ui->tableWidget->setRowCount(nb_rows);
}

template<class T>
void navisu::setItem(int col, int row, const T & t){
    ui->tableWidget->setItem(col, row, new QTableWidgetItem(QString::fromStdString(boost::lexical_cast<std::string>(t))));
}

template<>
void navisu::setItem<bool>(int col, int row, const bool & b){
    if(b)
        setItem(col, row, "True");
    else
        setItem(col, row, "False");
}

void navisu::tableSelected(QString table){
    if(table == "StopArea") show_stop_area();
    else if(table == "ValidityPattern") show_validity_pattern();
    else if(table == "Line") show_line();
    else if(table == "JourneyPattern") show_journey_pattern();
    else if(table == "VehicleJourney") show_vehicle_journey();
    else if(table == "StopPoint") show_stop_point();
    else if(table == "StopTime") show_stop_time();
    else if(table == "Network") show_network();
    else if(table == "Mode") show_mode();
    else if(table == "ModeType") show_commercial_mode();
    else if(table == "City") show_city();
    else if(table == "Connection") show_connection();
    else if(table == "JourneyPatternPoint") show_journey_pattern_point();
    else if(table == "District") show_district();
    else if(table == "Department") show_department();
    else if(table == "Company") show_company();
    else if(table == "Vehicle") show_vehicle();
    else if(table == "Country") show_country();
}

void navisu::show_stop_area(){
    resetTable(7, d.pt_data.stop_areas.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" <<"Name" <<"City" << "id" << "X" << "Y");
    int count = 0;
    for(StopArea & sa : d.pt_data.stop_areas){

        setItem(count ,0, sa.uri);
        setItem(count, 1, sa.id);
        setItem(count ,2, sa.name);
        if(sa.city_idx < d.pt_data.cities.size())
            setItem(count ,3, d.pt_data.cities[sa.city_idx].name);
        setItem(count ,4, sa.id);
        setItem(count ,5, sa.coord.x);
        setItem(count ,6, sa.coord.y);
        count++;
    }
}

void navisu::show_validity_pattern(){
    resetTable(3, d.pt_data.validity_patterns.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External code" << "Id" << "Pattern");

    for(size_t i=0; i < d.pt_data.validity_patterns.size(); ++i){
        ValidityPattern & vp = d.pt_data.validity_patterns[i];
        setItem(i, 0, vp.uri);
        setItem(i, 1, vp.id);
        setItem(i, 2, vp.str());
    }
}

void navisu::show_line(){
    resetTable(13, d.pt_data.lines.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External code"<< "Id" << "Name" << "code" << "Forward Name" << "Backward Name"
                                               << "Additional data" << "Color" << "Sort" << "Mode Type"
                                               << "Network" << "Foward Direction" << "Backward Direction");
    for(size_t i=0; i < d.pt_data.lines.size(); ++i){
        Line & l = d.pt_data.lines[i];
        setItem(i, 0, l.uri);
        setItem(i, 1, l.id);
        setItem(i, 2, l.name);
        setItem(i, 3, l.code);
        setItem(i, 4, l.forward_name);
        setItem(i, 5, l.backward_name);
        setItem(i, 6, l.additional_data);
        setItem(i, 7, l.color);
        setItem(i, 8, l.sort);
        if(l.commercial_mode_idx < d.pt_data.commercial_modes.size())
            setItem(i, 9, formatHeader(d.pt_data.commercial_modes[l.commercial_mode_idx]));
        if(l.network_idx < d.pt_data.networks.size())
            setItem(i, 10, formatHeader(d.pt_data.networks[l.network_idx]));
        if(l.forward_direction_idx < d.pt_data.stop_areas.size())
            setItem(i, 11, formatHeader(d.pt_data.stop_areas[l.forward_direction_idx]));
        if(l.backward_direction_idx < d.pt_data.stop_areas.size())
            setItem(i, 12, formatHeader(d.pt_data.stop_points[l.backward_direction_idx]));
    }
}

void navisu::show_journey_pattern(){
    resetTable(8, d.pt_data.journey_patterns.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Name" << "Is Frequence" << "Is Foward" << "Is Adapted" << "Line" << "Mode Type" );

    for(size_t i=0; i < d.pt_data.journey_patterns.size(); ++i){
        JourneyPattern & r = d.pt_data.journey_patterns[i];
        setItem(i, 0, r.uri);
        setItem(i, 1, r.id);
        setItem(i, 2, r.name);
        setItem(i, 3, r.is_frequence);
        setItem(i, 4, r.is_forward);
        setItem(i, 5, r.is_adapted);
        if(r.line_idx < d.pt_data.lines.size())
            setItem(i, 6, formatHeader(d.pt_data.lines[r.line_idx]));
        if(r.commercial_mode_idx < d.pt_data.commercial_modes.size())
            setItem(i, 7, formatHeader(d.pt_data.commercial_modes[r.commercial_mode_idx]));
    }
}

void navisu::show_vehicle_journey(){
    resetTable(8, d.pt_data.vehicle_journeys.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() <<"External Code" << "Id" << "Name" << "JourneyPattern" << "Company" << "Mode" << "Vehicle" << "Is adapted");

    for(size_t i=0; i < d.pt_data.vehicle_journeys.size(); ++i){
        VehicleJourney & vj = d.pt_data.vehicle_journeys[i];
        setItem(i, 0, vj.uri);
        setItem(i, 1, vj.id);
        setItem(i, 2, vj.name);
        if(vj.journey_pattern_idx < d.pt_data.journey_patterns.size())
            setItem(i, 3, formatHeader(d.pt_data.journey_patterns[vj.journey_pattern_idx]));
        if(vj.company_idx < d.pt_data.companies.size())
            setItem(i, 4, formatHeader(d.pt_data.companies[vj.company_idx]));
        if(vj.mode_idx < d.pt_data.modes.size())
            setItem(i, 5, formatHeader(d.pt_data.modes[vj.mode_idx]));
        if(vj.vehicle_idx < d.pt_data.vehicles.size())
            setItem(i, 6, formatHeader(d.pt_data.vehicles[vj.vehicle_idx]));
        setItem(i, 7, vj.is_adapted);
    }
}

void navisu::show_stop_point(){
    resetTable(7, d.pt_data.stop_points.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Name" << "StopArea" << "City" << "Mode"
                                               << "Network");
    for(size_t i=0; i < d.pt_data.stop_points.size(); ++i){
        StopPoint & sp = d.pt_data.stop_points[i];
        setItem(i, 0, sp.uri);
        setItem(i, 1, sp.id);
        setItem(i, 2, sp.name);
        if(sp.stop_area_idx < d.pt_data.stop_areas.size())
            setItem(i, 3, formatHeader(d.pt_data.stop_areas[sp.stop_area_idx]));
        if(sp.city_idx < d.pt_data.cities.size())
            setItem(i, 4, formatHeader(d.pt_data.cities[sp.city_idx]));
        if(sp.mode_idx < d.pt_data.modes.size())
            setItem(i, 5, formatHeader(d.pt_data.modes[sp.mode_idx]));
        if(sp.network_idx < d.pt_data.networks.size())
            setItem(i, 6, formatHeader(d.pt_data.networks[sp.network_idx]));
    }
}

void navisu::show_stop_time(){
    resetTable(8, d.pt_data.stop_times.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Arrival Time" << "Departure Time"
                                               << "Vehicle Journey" << "Order" << "ODT" << "Zone");

    for(size_t i=0; i < d.pt_data.stop_times.size(); ++i){
        StopTime & st = d.pt_data.stop_times[i];
        setItem(i, 0, st.uri);
        setItem(i, 1, st.id);
        setItem(i, 2, st.arrival_time);
        setItem(i, 3, st.departure_time);
        if(st.vehicle_journey_idx < d.pt_data.vehicle_journeys.size())
            setItem(i, 4, formatHeader(d.pt_data.vehicle_journeys[st.vehicle_journey_idx]));
        setItem(i, 5, st.order);
        setItem(i, 6, st.ODT);
        setItem(i, 7, st.zone);
    }

}

void navisu::show_network(){
    resetTable(4, d.pt_data.networks.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Name" << "Nb lines");
    for(size_t i=0; i < d.pt_data.networks.size(); ++i){
        Network & n = d.pt_data.networks[i];
        setItem(i, 0, n.uri);
        setItem(i, 1, n.id);
        setItem(i, 2, n.name);
        setItem(i, 3, n.line_list.size());
    }
}

void navisu::show_mode() {
    resetTable(4, d.pt_data.modes.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Name" << "Mode Type");
    for(size_t i=0; i < d.pt_data.modes.size(); ++i){
        Mode & m = d.pt_data.modes[i];
        setItem(i, 0, m.uri);
        setItem(i, 1, m.id);
        setItem(i, 2, m.name);
        if(m.commercial_mode_idx < d.pt_data.commercial_modes.size())
            setItem(i, 3, formatHeader(d.pt_data.commercial_modes[m.commercial_mode_idx]));
    }
}

void navisu::show_commercial_mode(){
    resetTable(5, d.pt_data.commercial_modes.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Name" << "Nb Modes" << "Nb lines");
    for(size_t i=0; i < d.pt_data.commercial_modes.size(); ++i){
        ModeType & mt = d.pt_data.commercial_modes[i];
        setItem(i, 0, mt.uri);
        setItem(i, 1, mt.id);
        setItem(i, 2, mt.name);
        setItem(i, 3, mt.mode_list.size());
        setItem(i, 4, mt.line_list.size());
    }
}

void navisu::show_city() {
    resetTable(6, d.pt_data.cities.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Name" << "Postal Code" << "Department" << "X" << "Y");
    for(size_t i=0; i < d.pt_data.cities.size(); ++i){
        City & c = d.pt_data.cities[i];
        setItem(i, 0, c.uri);
        setItem(i, 1, c.id);
        setItem(i, 2, c.name);
        setItem(i, 3, c.main_postal_code);
        if(c.department_idx < d.pt_data.departments.size())
            setItem(i, 4, formatHeader(d.pt_data.departments[c.department_idx]));
        setItem(i, 5, c.coord.x);
        setItem(i, 6, c.coord.y);
    }
}

void navisu::show_connection(){
    resetTable(6, d.pt_data.connections.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Departure" << "Destination" << "Duration" << "Max Duration");
    for(size_t i=0; i < d.pt_data.connections.size(); ++i){
        Connection & c = d.pt_data.connections[i];
        setItem(i, 0, c.uri);
        setItem(i, 1, c.id);
        if(c.departure_stop_point_idx < d.pt_data.stop_points.size())
            setItem(i, 2, formatHeader(d.pt_data.stop_points[c.departure_stop_point_idx]));
        if(c.destination_stop_point_idx < d.pt_data.stop_points.size())
            setItem(i, 3, formatHeader(d.pt_data.stop_points[c.destination_stop_point_idx]));
        setItem(i, 4, c.duration);
        setItem(i, 5, c.max_duration);
    }
}

void navisu::show_district(){
    resetTable(6, d.pt_data.districts.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Name" << "Main City" << "Country" << "Nb departments");
    for(size_t i=0; i < d.pt_data.districts.size(); ++i){
        District & di = d.pt_data.districts[i];
        setItem(i, 0, di.uri);
        setItem(i, 1, di.id);
        setItem(i, 2, di.name);
        if(di.main_city_idx < d.pt_data.cities.size())
            setItem(i, 3, formatHeader(d.pt_data.cities[di.main_city_idx]));
        if(di.country_idx < d.pt_data.countries.size())
            setItem(i, 4, formatHeader(d.pt_data.countries[di.country_idx]));
        setItem(i, 5, di.department_list.size());
    }

}

void navisu::show_journey_pattern_point(){
    resetTable(5, d.pt_data.journey_pattern_points.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Order" << "Stop Point" << "JourneyPattern");
    for(size_t i=0; i < d.pt_data.journey_pattern_points.size(); ++i){
        JourneyPatternPoint & rp = d.pt_data.journey_pattern_points[i];
        setItem(i, 0, rp.uri);
        setItem(i, 1, rp.id);
        setItem(i, 2, rp.order);
        if(rp.stop_point_idx < d.pt_data.stop_points.size())
            setItem(i, 3, formatHeader(d.pt_data.stop_points[rp.stop_point_idx]));
        if(rp.journey_pattern_idx < d.pt_data.journey_patterns.size())
            setItem(i, 4, formatHeader(d.pt_data.journey_patterns[rp.journey_pattern_idx]));

    }
}

void navisu::show_department(){
    resetTable(6, d.pt_data.departments.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Name" << "Main City" << "District" << "Nb Cities");
    for(size_t i=0; i < d.pt_data.departments.size(); ++i){
        Department & dpt = d.pt_data.departments[i];
        setItem(i, 0, dpt.uri);
        setItem(i, 1, dpt.id);
        setItem(i, 2, dpt.name);
        if(dpt.main_city_idx < d.pt_data.cities.size())
            setItem(i, 3, formatHeader(d.pt_data.cities[dpt.main_city_idx]));
        if(dpt.district_idx < d.pt_data.districts.size())
            setItem(i, 4, formatHeader(d.pt_data.districts[dpt.district_idx]));
        setItem(i, 5, dpt.city_list.size());
    }
}

void navisu::show_company(){
    resetTable(4, d.pt_data.companies.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Name" << "Nb lines");
    for(size_t i = 0; i < d.pt_data.companies.size(); ++i){
        Company & c = d.pt_data.companies[i];
        setItem(i, 0, c.uri);
        setItem(i, 1, c.id);
        setItem(i, 2, c.name);
        setItem(i, 3, c.line_list.size());
    }
}

void navisu::show_vehicle(){
    resetTable(3, d.pt_data.vehicles.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Name");
    for(size_t i = 0; i < d.pt_data.vehicles.size(); ++i){
        Vehicle & v = d.pt_data.vehicles[i];
        setItem(i, 0, v.uri);
        setItem(i, 1, v.id);
        setItem(i, 2, v.name);
    }

}

void navisu::show_country(){
    resetTable(5, d.pt_data.countries.size());
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "External Code" << "Id" << "Name" << "Main City" << "Nb districts");
    for(size_t i = 0; i < d.pt_data.countries.size(); ++i){
        Country & c = d.pt_data.countries[i];
        setItem(i, 0, c.uri);
        setItem(i, 1, c.id);
        setItem(i, 2, c.name);
        if(c.main_city_idx < d.pt_data.cities.size())
            setItem(i, 3, formatHeader(d.pt_data.cities[c.main_city_idx]));
        setItem(i, 4, c.district_list.size());
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    navisu w;
    w.show();

    return a.exec();
}
