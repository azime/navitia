# coding=utf-8

#  Copyright (c) 2001-2014, Canal TP and/or its affiliates. All rights reserved.
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
from flask.ext.restful import fields, marshal_with, reqparse
from flask.ext.restful.inputs import boolean
from jormungandr import i_manager
from jormungandr.interfaces.v1.StatedResource import StatedResource
from jormungandr.interfaces.v1.make_links import add_coverage_link, add_collection_links, clean_links
from jormungandr.interfaces.v1.converters_collection_type import collections_to_resource_type
from collections import OrderedDict
from jormungandr.interfaces.v1.fields import NonNullNested, FieldDateTime


region_fields = {
    "id": fields.String(attribute="region_id"),
    "start_production_date": fields.String,
    "end_production_date": fields.String,
    "last_load_at": FieldDateTime(),
    "name": fields.String,
    "status": fields.String,
    "shape": fields.String,
    "error": NonNullNested({
        "code": fields.String,
        "value": fields.String
    }),
    "dataset_created_at": fields.String(),

}
regions_fields = OrderedDict([
    ("regions", fields.List(fields.Nested(region_fields)))
])

collections = collections_to_resource_type.keys()


class Coverage(StatedResource):

    @clean_links()
    @add_coverage_link()
    @add_collection_links(collections)
    @marshal_with(regions_fields)
    def get(self, region=None, lon=None, lat=None):

        parser = reqparse.RequestParser()
        parser.add_argument("disable_geojson", type=boolean, default=False)

        args = parser.parse_args()

        resp = i_manager.regions(region, lon, lat)
        if 'regions' in resp:
            resp['regions'] = sorted(resp['regions'], cmp=lambda reg1, reg2: cmp(reg1.get('name'), reg2.get('name')))
        if args['disable_geojson']:
            for r in resp['regions']:
                if 'shape' in r:
                    del r['shape']
        return resp, 200
