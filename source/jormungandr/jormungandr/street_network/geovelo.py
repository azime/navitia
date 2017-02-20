#  Copyright (c) 2001-2017, Canal TP and/or its affiliates. All rights reserved.
#
# This file is part of Navitia,
#     the software to build cool stuff with public transport.
#
# Hope you'll enjoy and contribute to this project,
#     powered by Canal TP (www.canaltp.fr).
# Help us simplify mobility and open public transport:
#     a non ending quest to the responsive locomotion way of traveling!
#
# LICENCE: This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Stay tuned using
# twitter @navitia
# IRC #navitia on freenode
# https://groups.google.com/d/forum/navitia
# www.navitia.io

from __future__ import absolute_import, print_function, unicode_literals, division
import logging
import requests as requests
import pybreaker
import json
from navitiacommon import response_pb2
from jormungandr import app
from jormungandr.exceptions import TechnicalError, InvalidArguments, UnableToParse
from jormungandr.street_network.street_network import AbstractStreetNetworkService
from jormungandr.utils import get_pt_object_coord, is_url


class Geovelo(AbstractStreetNetworkService):

    def __init__(self, instance, service_url, id='geovelo', timeout=10, api_key=None, **kwargs):
        self.instance = instance
        self.sn_system_id = id
        if not is_url(service_url):
            raise ValueError('service_url {} is not a valid url'.format(service_url))
        self.service_url = service_url
        self.api_key = api_key
        self.timeout = timeout
        self.breaker = pybreaker.CircuitBreaker(fail_max=app.config['CIRCUIT_BREAKER_MAX_GEOVELO_FAIL'],
                                                reset_timeout=app.config['CIRCUIT_BREAKER_GEOVELO_TIMEOUT_S'])

    @staticmethod
    def _pt_object_summary(pt_object):
        coord = get_pt_object_coord(pt_object)
        return [coord.lat, coord.lon, getattr(pt_object, 'uri', None)]

    @staticmethod
    def _make_request_arguments(origins, destinations):
        return {
            'starts': [Geovelo._pt_object_summary(o) for o in origins],
            'ends': [Geovelo._pt_object_summary(o) for o in destinations],
        }

    def _call_geovelo(self, url, method=requests.post, data=None):
        logging.getLogger(__name__).debug('Geovelo routing service , call url : {}'.format(url))
        try:
            return self.breaker.call(method, url, timeout=self.timeout, data=data,
                                     headers={'content-type': 'application/json',
                                              'Api-Key': self.api_key})
        except pybreaker.CircuitBreakerError as e:
            logging.getLogger(__name__).error('Geovelo routing service dead (error: {})'.format(e))
            self.record_external_failure('circuit breaker open')
        except requests.Timeout as t:
            logging.getLogger(__name__).error('Geovelo routing service dead (error: {})'.format(t))
            self.record_external_failure('timeout')
        except Exception as e:
            logging.getLogger(__name__).exception('Geovelo routing error')
            self.record_external_failure(str(e))
        return None

    def _get_matrix(self, json_response):
        '''
        build the 1-n response matrix from geovelo table
        each element is ["start", "end", duration] (first being the header)
        as it is ordered, we only consider the third
        '''
        sn_routing_matrix = response_pb2.StreetNetworkRoutingMatrix()
        row = sn_routing_matrix.rows.add()
        if json_response[0] != ["start_reference", "end_reference", "duration"]:
            logging.getLogger(__name__).error('Geovelo parsing error. Response: {}'.format(json_response))
            raise UnableToParse('Geovelo parsing error. Response: {}'.format(json_response))
        for e in json_response[1:]:
            if e[2] == 'duration':
                continue
            routing = row.routing_response.add()
            if e[2]:
                routing.duration = e[2]
                routing.routing_status = response_pb2.reached
            else:
                routing.duration = -1
                routing.routing_status = response_pb2.unknown
        return sn_routing_matrix

    @staticmethod
    def _check_response(response):
        if response is None:
            raise TechnicalError('impossible to access geovelo service')
        if response.status_code != 200:
            logging.getLogger(__name__).error('Geovelo service unavailable, impossible to query : {}'
                                              ' with response : {}'
                                              .format(response.url, response.text))
            raise TechnicalError('Geovelo service unavailable, impossible to query : {}'.
                                 format(response.url))

    def get_street_network_routing_matrix(self, origins, destinations, street_network_mode, max_duration, request, **kwargs):
        if street_network_mode != "bike":
            logging.getLogger(__name__).error('Geovelo, mode {} not implemented'.format(street_network_mode))
            raise InvalidArguments('Geovelo, mode {} not implemented'.format(street_network_mode))
        if len(origins) != 1 and len(destinations) != 1:
            logging.getLogger(__name__).error('Geovelo, managing only 1-n in connector, requested {}-{}'
                                              .format(len(origins), len(destinations)))
            raise InvalidArguments('Geovelo, managing only 1-n in connector, requested {}-{}'
                                   .format(len(origins), len(destinations)))

        data = self._make_request_arguments(origins, destinations)
        r = self._call_geovelo('{}/{}'.format(self.service_url, 'api/v2/routes_m2m'),
                               requests.post, json.dumps(data))
        self._check_response(r)
        resp_json = r.json()

        if len(resp_json) - 1 != len(origins) * len(destinations):
            logging.getLogger(__name__).error('Geovelo nb response != nb requested')
            raise UnableToParse('Geovelo nb response != nb requested')

        return self._get_matrix(resp_json)

    def _get_response(self, json_response, mode, pt_object_origin, pt_object_destination, datetime, clockwise):
        resp = response_pb2.Response()
        resp.status_code = 200
        resp.response_type = response_pb2.ITINERARY_FOUND

        if json_response[0] != ["start_reference", "end_reference", "duration"] or len(json_response) != 2:
            logging.getLogger(__name__).error('Geovelo parsing error. Response: {}'.format(json_response))
            raise UnableToParse('Geovelo parsing error. Response: {}'.format(json_response))
        duration = json_response[1][2]

        journey = resp.journeys.add()
        journey.duration = duration
        if clockwise:
            journey.departure_date_time = datetime
            journey.arrival_date_time = datetime + duration
        else:
            journey.departure_date_time = datetime - duration
            journey.arrival_date_time = datetime

        journey.durations.total = duration

        section = journey.sections.add()
        section.type = response_pb2.STREET_NETWORK

        section.begin_date_time = journey.departure_date_time
        section.end_date_time = journey.arrival_date_time

        section.id = 'section_0'
        section.duration = journey.duration
        section.length = journey.duration*3 #assuming speed is 3m/s

        section.origin.CopyFrom(pt_object_origin)
        section.destination.CopyFrom(pt_object_destination)

        section.street_network.duration = section.duration
        section.street_network.length = section.length
        section.street_network.mode = response_pb2.Bike

        path_item = section.street_network.path_items.add()
        path_item.name = "unknown"
        path_item.duration = section.duration
        path_item.length = section.length
        # TODO: calculate direction
        path_item.direction = 0

        pt_coord = section.street_network.coordinates.add()
        coord = get_pt_object_coord(pt_object_origin)
        pt_coord.lat, pt_coord.lon = coord.lat, coord.lon
        pt_coord = section.street_network.coordinates.add()
        coord = get_pt_object_coord(pt_object_destination)
        pt_coord.lat, pt_coord.lon = coord.lat, coord.lon

        return resp

    def direct_path(self, mode, pt_object_origin, pt_object_destination, datetime, clockwise, request):
        if mode != "bike":
            logging.getLogger(__name__).error('Geovelo, mode {} not implemented'.format(mode))
            raise InvalidArguments('Geovelo, mode {} not implemented'.format(mode))

        data = Geovelo._make_request_arguments([pt_object_origin], [pt_object_destination])
        r = self._call_geovelo('{}/{}'.format(self.service_url, 'api/v2/routes_m2m'),
                               requests.post, json.dumps(data))
        self._check_response(r)
        resp_json = r.json()

        if len(resp_json) != 2:
            logging.getLogger(__name__).error('Geovelo nb response != nb requested')
            raise UnableToParse('Geovelo nb response != nb requested')

        return self._get_response(resp_json, mode, pt_object_origin, pt_object_destination, datetime, clockwise)