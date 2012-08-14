#include "raptor.h"
#include "type/data.h"
#include "utils/timer.h"
#include "boost/date_time.hpp"
#include "naviMake/build_helper.h"

#include <valgrind/callgrind.h>

using namespace navitia;

int main(int, char **) {
    type::Data data;
    {
        Timer t("Chargement des données");
        data.load_lz4("/home/vlara/navitia/jeu/IdF/IdF.nav");

        data.build_proximity_list();
    }



    //    int depart = 0;
    //    int arrivee = 2;
    //    std::cout << "Recherche de chemin entre " << data.pt_data.stop_areas.at(depart).name << " et " << data.pt_data.stop_areas.at(arrivee).name << std::endl;



    ////    BOOST_FOREACH(navitia::type::VehicleJourney vj, data.pt_data.vehicle_journeys) {
    ////        unsigned int precstid = data.pt_data.stop_times.size() + 1;
    ////        BOOST_FOREACH(unsigned int stid, vj.stop_time_list){
    ////            if(precstid != (data.pt_data.stop_times.size() + 1)) {
    ////                if(data.pt_data.stop_times.at(precstid).arrival_time > data.pt_data.stop_times.at(stid).arrival_time ||
    ////                   data.pt_data.stop_times.at(precstid).departure_time > data.pt_data.stop_times.at(stid).departure_time )
    ////                    std::cout <<"Bug de données" << std::endl;
    ////            }
    ////            precstid =  stid;
    ////        }
    ////    }plannerreverse?format=json&departure_lat=48.83192652572024&departure_lon=2.355914323083246&destination_lat=48.873944716692286&destination_lon=2.34836122413323&time=1000&date=20120511
    //    std::cout << "size : " << data.pt_data.route_points.at(10).vehicle_journey_list_arrival.size() << " "
    //              << data.pt_data.route_points.at(10).vehicle_journey_list.size()   << std::endl;
    routing::raptor::RAPTOR raptor(data);
    {
        Timer t("Calcul raptor");
        auto result = raptor.compute(12344, 51, 28800, 0);
        std::cout << result << std::endl;
        std::cout << makeItineraire(result);

        //        BOOST_FOREACH(auto pouet, result) {
        //        std::cout << pouet << std::endl << std::endl;

        //        std::cout << makeItineraire(pouet);
        //        }
        //        routing::Path result = raptor.compute(11484, 5596, 28800, 7);
    }

//    BOOST_FOREACH(unsigned int spidx, data.pt_data.stop_areas.at(3849).stop_point_list) {
//        BOOST_FOREACH(unsigned int rpidx, data.pt_data.stop_points.at(spidx).route_point_list) {
//            BOOST_FOREACH(unsigned int vjidx, data.pt_data.route_points.at(rpidx).vehicle_journey_list) {
//                if(data.pt_data.routes.at(data.pt_data.vehicle_journeys.at(vjidx).route_idx).line_idx == 1620) {
//                    navitia::type::RoutePoint rp = data.pt_data.route_points.at(rpidx);
//                    navitia::type::Route route = data.pt_data.routes.at(rp.route_idx);

//                    std::cout << " departure time " << data.pt_data.stop_times.at(data.pt_data.vehicle_journeys.at(vjidx).stop_time_list.at(data.pt_data.route_points.at(rpidx).order)).departure_time
//                              << " " << data.pt_data.stop_times.at(data.pt_data.vehicle_journeys.at(vjidx).stop_time_list.at(data.pt_data.route_points.at(rpidx).order)).arrival_time  << " " << vjidx << " " << route.idx
//                              << " " << data.pt_data.stop_points.at(data.pt_data.route_points.at(route.route_point_list.at(rp.order-1)).stop_point_idx).stop_area_idx << " " <<
//                                 data.pt_data.stop_times.at(data.pt_data.vehicle_journeys.at(vjidx).stop_time_list.at(rp.order-1)).departure_time << " " <<data.pt_data.vehicle_journeys.at(vjidx).stop_time_list.at(rp.order-1) << std::endl;
//                }
//            }
//        }
//    }
//    BOOST_FOREACH(unsigned int rpidx, data.pt_data.routes.at(1905).route_point_list) {
//        std::cout <<data.pt_data.stop_points.at(data.pt_data.route_points.at(rpidx).stop_point_idx).stop_area_idx << " " <<  << std::endl;
//    }


//    std::cout << data.pt_data.vehicle_journeys.at(25653).route_idx << std::endl;


    //    std::cout << "Je suis ici ! " << std::endl;

    //    {
    //        Timer t("Calcul raptor");/*
    //        routing::Path result = raptor.makeItineraire(raptor.compute(depart, arrivee, 72000, 7));
    //        std::cout << result;*/

    //        routing::raptor::map_int_pint_t bests;
    //        bests[depart] = navitia::routing::raptor::type_retour(-1, navitia::routing::DateTime(7, 72000));
    //        std::cout << raptor.compute(bests, arrivee);
    //    }

    //    {
    //        Timer t("RAPTOR");
    //        std::cout << data.pt_data.stop_areas.at(13587).coord << data.pt_data.stop_areas.at(2460).coord << std::endl;

    //        std::cout << raptor.compute(navitia::type::GeographicalCoord(2.3305474316803103, 48.867483087514856), 500, navitia::type::GeographicalCoord(2.349430179055217, 48.84850904718449), 500, 28800, 7);
    //    }





}
