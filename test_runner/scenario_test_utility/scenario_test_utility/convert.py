#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2020 Tier IV, Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from ament_index_python.packages import get_package_share_directory
from argparse import ArgumentParser
from pathlib import Path
from xml.etree import ElementTree

import sys
import xmlschema
import xmltodict
import yaml


def load(path):
    if path.exists():
        with open(path, 'r') as f:
            return yaml.safe_load(f)
    else:
        print('No such file or directory: ' + input)
        sys.exit()


def func(keyword, node):

    result = {}

    if isinstance(node, dict):
        #
        # ???: { ... }
        #
        for tag, value in node.items():

            if isinstance(value, list) and len(value) == 0:
                #
                # Tag: []
                #
                # => REMOVE
                #
                continue

            if str.islower(tag[0]):
                #
                # tag: { ... }
                #
                # => @tag: { ... }
                #
                result["@" + tag] = value
            else:
                #
                # Tag: { ... }
                #
                # => NO CHANGES
                #
                result[tag] = func(tag, value)

        return result

    elif isinstance(node, list):
        #
        # ???: [ ... ]
        #
        result = []

        for index, item in enumerate(node):
            result.append(func(keyword, item))

        return result

    elif isinstance(node, str):

        if str.islower(keyword[0]):
            result["@" + keyword] = node
        else:
            result[keyword] = node

        return result

    else:
        return result


# def convert(input, output):
#     """
#
#     **Args**
#
#     * input  (`Path`)
#     * output (`Path`)
#     """
#
#     a = func('OpenSCENARIO', load(input))
#     # print(a)
#     # print()
#
#     a.pop('ScenarioModifiers', None)
#     # print(a)
#     b = xmltodict.unparse(a, pretty=False, short_empty_elements=True, indent='  ')
#     print(b)
#     # print()
#     b = b.replace('True', 'true').replace('False', 'false')
#
#     share = get_package_share_directory('scenario_test_utility')
#
#     schema = xmlschema.XMLSchema(
#         str(Path(share).joinpath('../ament_index/resource_index/packages/OpenSCENARIO.xsd')))
#
#     x = None
#
#     try:
#         x = schema.decode(
#             b, validation='skip', converter=xmlschema.converters.UnorderedConverter())
#     except Exception as e:
#         print(e)
#
#     # x = schema.encode(x, unordered=True)
#     print(x)
#
#     # print(
#     #     xmlschema.from_json(json.dumps(a), schema)
#     #     )
#
#     print(output)


def convert(input, output):

    A = ElementTree.fromstring(
        xmltodict.unparse(
            func('OpenSCENARIO', load(input))
            ).replace('True', 'true').replace('False', 'false')
        )

    share = get_package_share_directory('scenario_test_utility')

    schema = xmlschema.XMLSchema(
        str(Path(share).joinpath('../ament_index/resource_index/packages/OpenSCENARIO.xsd')),
        # converter=xmlschema.converters.UnorderedConverter()
        )

    # print(schema.validate(A))

    B, _ = schema.decode(
        A,
        preserve_root=True,
        validation='lax',
        unordered=True
        )

    print(B)
    print()

    C = schema.encode(
        B,
        preserve_root=True,
        unordered=True
        )

    print(
        xmlschema.etree_tostring(
            C,
            spaces_for_tab=2,
            xml_declaration=True
            ))


def main():
    parser = ArgumentParser(description='convert T4v2 scenario into OpenSCENARIO')

    parser.add_argument('--input', type=str, required=True)
    parser.add_argument('--output', type=str, default=Path.cwd())

    args = parser.parse_args()

    convert(Path(args.input).resolve(), Path(args.output).resolve())


if __name__ == "__main__":
    main()
