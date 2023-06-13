#
#   Copyright 2013-2023 The Foundry Visionmongers Ltd
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
"""
Tests that cover the openassetio.hostApi.EntityReferencePager wrapper class.
"""

# pylint: disable=no-self-use
# pylint: disable=invalid-name,redefined-outer-name
# pylint: disable=missing-class-docstring,missing-function-docstring
from unittest import mock

import pytest

from openassetio import EntityReference
from openassetio.hostApi import Manager, EntityReferencePager

class Test_EntityReferencePager_init:
    def test_when_constructed_with_EntityReferencePagerInterface_as_None_then_raises_TypeError(
        self, a_host_session
    ):
        # Check the message is both helpful and that the bindings
        # were loaded in the correct order such that types are
        # described correctly.
        matchExpr = (
            r".+The following argument types are supported:[^(]+"
            r"EntityReferencePager\([^,]+managerApi.EntityReferencePagerInterface,[^,]+managerApi.HostSession.+"
        )

        with pytest.raises(TypeError, match=matchExpr):
            EntityReferencePager(None, a_host_session)


class Test_EntityReferencePager_next:
    def test_method_defined_in_cpp(self, method_introspector):
        assert not method_introspector.is_defined_in_python(EntityReferencePager.next)
        assert method_introspector.is_implemented_once(EntityReferencePager, "next")

    def test_wraps_the_corresponding_method_of_the_held_interface(
        self, an_entity_reference_pager, mock_entity_reference_pager_interface, a_host_session
    ):
        method = mock_entity_reference_pager_interface.mock.next
        an_entity_reference_pager.next()
        method.assert_called_once_with(a_host_session)


class Test_EntityReferencePager_hasNext:
    def test_method_defined_in_cpp(self, method_introspector):
        assert not method_introspector.is_defined_in_python(EntityReferencePager.hasNext)
        assert method_introspector.is_implemented_once(EntityReferencePager, "hasNext")

    @pytest.mark.parametrize("expected", (True, False))
    def test_wraps_the_corresponding_method_of_the_held_interface(
        self, an_entity_reference_pager, mock_entity_reference_pager_interface, a_host_session, expected
    ):
        method = mock_entity_reference_pager_interface.mock.hasNext
        method.return_value = expected

        assert an_entity_reference_pager.hasNext() == expected
        method.assert_called_once_with(a_host_session)


class Test_EntityReferencePager_get:
    def test_method_defined_in_cpp(self, method_introspector):
        assert not method_introspector.is_defined_in_python(EntityReferencePager.get)
        assert method_introspector.is_implemented_once(EntityReferencePager, "get")

    @pytest.mark.parametrize("expected", ([], [EntityReference("first 🌱")], [EntityReference("second 🌿"), EntityReference("third 🌲")]))
    def test_wraps_the_corresponding_method_of_the_held_interface(
        self, an_entity_reference_pager, mock_entity_reference_pager_interface, a_host_session, expected
    ):
        method = mock_entity_reference_pager_interface.mock.get
        method.return_value = expected

        assert an_entity_reference_pager.get() == expected
        method.assert_called_once_with(a_host_session)


@pytest.fixture
def an_entity_reference_pager(mock_entity_reference_pager_interface, a_host_session):
    return EntityReferencePager(mock_entity_reference_pager_interface, a_host_session)

