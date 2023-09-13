#
#   Copyright 2023 The Foundry Visionmongers Ltd
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
Tests of C++ binding utilities for exception types.
"""
import re
# pylint: disable=no-self-use
# pylint: disable=invalid-name,redefined-outer-name
# pylint: disable=missing-class-docstring,missing-function-docstring
from unittest import mock

import pytest

from openassetio import _openassetio_test  # pylint: disable=no-name-in-module
from openassetio import errors, EntityReference, TraitsData


expected_error_message = "Explosion 💥!"
expected_entity_reference = EntityReference("bogus:///entity_reference")
expected_entity_error_message = f"{expected_error_message} [{expected_entity_reference}]"

all_exception_types = (
    errors.OpenAssetIOException,
    errors.InputValidationException,
    errors.ConfigurationException,
    errors.NotImplementedException,
    errors.UnhandledException,
    errors.BatchElementException,
    errors.BatchElementEntityReferenceException,
    errors.UnknownBatchElementException,
    errors.InvalidTraitSetBatchElementException,
    errors.InvalidTraitsDataBatchElementException,
    errors.EntityAccessErrorBatchElementException,
    errors.InvalidEntityReferenceBatchElementException,
    errors.MalformedEntityReferenceBatchElementException,
    errors.EntityResolutionErrorBatchElementException,
    errors.InvalidPreflightHintBatchElementException,
)

batch_element_exceptions = (
    errors.BatchElementException,
    errors.BatchElementEntityReferenceException,
    errors.UnknownBatchElementException,
    errors.InvalidTraitSetBatchElementException,
    errors.InvalidTraitsDataBatchElementException,
    errors.EntityAccessErrorBatchElementException,
    errors.InvalidEntityReferenceBatchElementException,
    errors.MalformedEntityReferenceBatchElementException,
    errors.EntityResolutionErrorBatchElementException,
    errors.InvalidPreflightHintBatchElementException,
)

batch_element_error_codes = (
    errors.BatchElementError.ErrorCode.kUnknown,
    errors.BatchElementError.ErrorCode.kInvalidEntityReference,
    errors.BatchElementError.ErrorCode.kUnknown,
    errors.BatchElementError.ErrorCode.kInvalidTraitSet,
    errors.BatchElementError.ErrorCode.kInvalidTraitsData,
    errors.BatchElementError.ErrorCode.kEntityAccessError,
    errors.BatchElementError.ErrorCode.kInvalidEntityReference,
    errors.BatchElementError.ErrorCode.kMalformedEntityReference,
    errors.BatchElementError.ErrorCode.kEntityResolutionError,
    errors.BatchElementError.ErrorCode.kInvalidPreflightHint,
)

entity_reference_exceptions = (
    errors.BatchElementEntityReferenceException,
    errors.EntityAccessErrorBatchElementException,
    errors.InvalidEntityReferenceBatchElementException,
    errors.MalformedEntityReferenceBatchElementException,
    errors.EntityResolutionErrorBatchElementException,
    errors.InvalidTraitSetBatchElementException,
    errors.InvalidTraitsDataBatchElementException,
    errors.InvalidPreflightHintBatchElementException,
)


class Test_exception_hierarchy:
    @pytest.mark.parametrize("exceptionType", all_exception_types)
    def test_when_exception_thrown_then_can_catch_as_OpenAssetIOException(self, exceptionType):
        with pytest.raises(errors.OpenAssetIOException):
            _openassetio_test.throwExceptionWithPopulatedArgs(exceptionType.__name__)

    @pytest.mark.parametrize("exceptionType", batch_element_exceptions)
    def test_when_batch_exception_thrown_then_can_catch_as_BatchElementException(
        self, exceptionType
    ):
        with pytest.raises(errors.BatchElementException):
            _openassetio_test.throwExceptionWithPopulatedArgs(exceptionType.__name__)

    @pytest.mark.parametrize("exceptionType", entity_reference_exceptions)
    def test_when_entity_exception_thrown_then_can_catch_as_BatchElementEntityReferenceException(
        self, exceptionType
    ):
        with pytest.raises(errors.BatchElementEntityReferenceException):
            _openassetio_test.throwExceptionWithPopulatedArgs(exceptionType.__name__)

    def test_when_ConfigurationException_thrown_then_can_catch_as_InputValidationException(
        self,
    ):
        with pytest.raises(errors.InputValidationException):
            _openassetio_test.throwExceptionWithPopulatedArgs(
                errors.ConfigurationException.__name__
            )

    def test_when_InvalidPreflightHint_thrown_then_can_catch_as_InvalidTraitsData(
        self,
    ):
        with pytest.raises(errors.InvalidTraitsDataBatchElementException):
            _openassetio_test.throwExceptionWithPopulatedArgs(
                errors.InvalidPreflightHintBatchElementException.__name__
            )


class Test_exception_data:
    @pytest.mark.parametrize("exception_type", all_exception_types)
    def test_when_nonentity_cpp_exception_thrown_then_message_is_retained(self, exception_type):
        with pytest.raises(exception_type, match=f"^{expected_error_message}$"):
            _openassetio_test.throwExceptionWithUnpopulatedArgs(exception_type.__name__)

    @pytest.mark.parametrize("exception_type", entity_reference_exceptions)
    def test_when_batch_entity_cpp_exception_thrown_then_message_is_retained(self, exception_type):
        with pytest.raises(exception_type, match=f"^{re.escape(expected_entity_error_message)}$"):
            _openassetio_test.throwExceptionWithPopulatedArgs(exception_type.__name__)

    @pytest.mark.parametrize(
        "exception_type,expected_code",
        zip(batch_element_exceptions, batch_element_error_codes),
    )
    def test_when_batch_cpp_exception_thrown_then_BatchElementError_is_retained(
        self, exception_type, expected_code
    ):
        with pytest.raises(exception_type) as exc:
            _openassetio_test.throwExceptionWithPopulatedArgs(exception_type.__name__)

        assert exc.value.error == errors.BatchElementError(expected_code, expected_error_message)

    @pytest.mark.parametrize("exception_type", entity_reference_exceptions)
    def test_when_entity_batch_cpp_exception_thrown_then_entity_ref_is_retained(
        self, exception_type
    ):
        with pytest.raises(exception_type) as exc:
            _openassetio_test.throwExceptionWithUnpopulatedArgs(exception_type.__name__)

        assert exc.value.entityReference is None

        with pytest.raises(exception_type) as exc:
            _openassetio_test.throwExceptionWithPopulatedArgs(exception_type.__name__)

        assert exc.value.entityReference == expected_entity_reference

    def test_when_trait_set_batch_cpp_exception_thrown_then_trait_set_is_retained(self):
        exception_type = errors.InvalidTraitSetBatchElementException

        with pytest.raises(exception_type) as exc:
            _openassetio_test.throwExceptionWithUnpopulatedArgs(exception_type.__name__)

        assert exc.value.traitSet is None

        with pytest.raises(exception_type) as exc:
            _openassetio_test.throwExceptionWithPopulatedArgs(exception_type.__name__)

        assert exc.value.traitSet == {"trait1", "trait2"}

    @pytest.mark.parametrize(
        "exception_type",
        (
            errors.InvalidTraitsDataBatchElementException,
            errors.InvalidPreflightHintBatchElementException,
        ),
    )
    def test_when_traits_data_batch_cpp_exception_thrown_then_traits_data_is_retained(
        self, exception_type
    ):
        with pytest.raises(exception_type) as exc:
            _openassetio_test.throwExceptionWithUnpopulatedArgs(exception_type.__name__)

        assert exc.value.traitsData is None

        with pytest.raises(exception_type) as exc:
            _openassetio_test.throwExceptionWithPopulatedArgs(exception_type.__name__)

        assert exc.value.traitsData == TraitsData({"trait1", "trait2"})
