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

#include "type/message.h"

#include "utils/logger.h"

#include <boost/format.hpp>

namespace pt = boost::posix_time;
namespace bg = boost::gregorian;


namespace navitia { namespace type {

MessageHolder& MessageHolder::operator=(const navitia::type::MessageHolder&& other){
    this->messages = std::move(other.messages);
    return *this;
}


bool Message::is_valid(const boost::posix_time::ptime& now, const boost::posix_time::time_period& action_period) const{
    if(now.is_not_a_date_time() && action_period.is_null()){
        return false;
    }

    bool to_return = is_publishable(now);

    if(!action_period.is_null()){
        to_return = to_return && is_applicable(action_period);
    }
    return to_return;
}

bool Message::is_publishable(const boost::posix_time::ptime& time) const{
    return publication_period.contains(time);
}

bool AtPerturbation::is_applicable(const boost::posix_time::time_period& period) const{
    bool days_intersects = false;

    //intersection de la period ou se déroule l'action et de la periode de validité de la perturbation
    pt::time_period common_period = period.intersection(application_period);

    //si la periode commune est null, l'impact n'est pas valide
    if(common_period.is_null()){
        return false;
    }

    //on doit travailler jour par jour
    //
    bg::date current_date = common_period.begin().date();

    while(!days_intersects && current_date <= common_period.end().date()){
        //on test uniquement le debut de la period, si la fin est valide, elle sera testé à la prochaine itération
        days_intersects = valid_day_of_week(current_date);

        //vérification des plages horaires journaliéres
        pt::time_period current_period = common_period.intersection(pt::time_period(pt::ptime(current_date, pt::seconds(0)), pt::hours(24)));
        days_intersects = days_intersects && valid_hour_perturbation(current_period);

        current_date += bg::days(1);
    }


    return days_intersects;

}

bool AtPerturbation::valid_hour_perturbation(const pt::time_period& period) const{
    pt::time_period daily_period(pt::ptime(period.begin().date(), application_daily_start_hour),
            pt::ptime(period.begin().date(), application_daily_end_hour));

    return period.intersects(daily_period);

}

bool AtPerturbation::valid_day_of_week(const bg::date& date) const{

    switch(date.day_of_week()){
        case bg::Monday: return this->active_days[0];
        case bg::Tuesday: return this->active_days[1];
        case bg::Wednesday: return this->active_days[2];
        case bg::Thursday: return this->active_days[3];
        case bg::Friday: return this->active_days[4];
        case bg::Saturday: return this->active_days[5];
        case bg::Sunday: return this->active_days[6];
    }
    return false; // Ne devrait pas arriver
}


}}//namespace
