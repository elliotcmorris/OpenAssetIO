// SPDX-License-Identifier: Apache-2.0
// Copyright 2013-2023 The Foundry Visionmongers Ltd
#include <string_view>

#include <pybind11/eval.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include <openassetio/errors/exceptions.hpp>

#include "../_openassetio.hpp"

namespace {
void registerNonBatchExceptions(const py::module &mod) {
  using openassetio::errors::ConfigurationException;
  using openassetio::errors::InputValidationException;
  using openassetio::errors::NotImplementedException;
  using openassetio::errors::OpenAssetIOException;
  using openassetio::errors::UnhandledException;

  // Register a new exception type. Note that this is not sufficient to
  // cause C++ exceptions to be translated. See
  // `register_exception_translator` below.
  const auto registerPyException = [&mod](const char *pyTypeName, const py::handle &base) {
    return py::exception<void /* unused */>{mod, pyTypeName, base};
  };

  // Register our OpenAssetIO exception types. Since these exceptions
  // simply take a string message and no other data, we can make use
  // of pybind11's limited custom exception support.
  const py::object pyOpenAssetIOException =
      registerPyException("OpenAssetIOException", PyExc_RuntimeError);
  // NOLINTNEXTLINE(bugprone-throw-keyword-missing)
  registerPyException("UnhandledException", pyOpenAssetIOException);
  // NOLINTNEXTLINE(bugprone-throw-keyword-missing)
  registerPyException("NotImplementedException", pyOpenAssetIOException);
  const py::object pyInputValidationException =
      registerPyException("InputValidationException", pyOpenAssetIOException);
  // NOLINTNEXTLINE(bugprone-throw-keyword-missing)
  registerPyException("ConfigurationException", pyInputValidationException);

  // Register a function that will translate our C++ exceptions to the
  // appropriate Python exception type.
  //
  // Note that capturing lambdas are not allowed here, so we must
  // `import` the exception type in the body of the function.
  py::register_exception_translator([](std::exception_ptr pexc) {
    if (!pexc) {
      return;
    }
    const py::module_ pyModule = py::module_::import("openassetio._openassetio.errors");

    // Use CPython's PyErr_SetObject to set a custom exception type as
    // the currently active Python exception in this thread.
    const auto setPyException = [&pyModule](const char *pyTypeName, const auto &exc) {
      const py::object pyClass = pyModule.attr(pyTypeName);
      const py::object pyInstance = pyClass(exc.what());
      PyErr_SetObject(pyClass.ptr(), pyInstance.ptr());
    };

    // Handle the different possible C++ exceptions, creating the
    // corresponding Python exception and setting it as the active
    // exception in this thread.
    try {
      std::rethrow_exception(std::move(pexc));
    } catch (const ConfigurationException &exc) {
      setPyException("ConfigurationException", exc);
    } catch (const InputValidationException &exc) {
      setPyException("InputValidationException", exc);
    } catch (const NotImplementedException &exc) {
      setPyException("NotImplementedException", exc);
    } catch (const UnhandledException &exc) {
      setPyException("UnhandledException", exc);
    } catch (const OpenAssetIOException &exc) {
      setPyException("OpenAssetIOException", exc);
    }
  });
}

void registerBatchElementExceptions(const py::module &mod) {
  // Pybind has very limited support for custom exception types. This is
  // a well-known tricky issue and is apparently not on the roadmap to
  // fix. The only direct support is for an exception type that takes a
  // single string parameter (message). However, we need `index` and
  // `error` parameters to mirror C++.
  //
  // A way forward is provided by the sketch in:
  // https://github.com/pybind/pybind11/issues/1281#issuecomment-1375950333
  //
  // We execute a Python string literal to create our base exception
  // type. The `globals` and `locals` dict parameters dictate the scope
  // of execution, so we use this to ensure the definition is scoped to
  // our `_openassetio` module.
  //
  // We can then retrieve this base exception type as a pybind object
  // and use it as the base class for pybind's limited exception
  // registration API.

  // Make sure OpenAssetIOException has already been registered!
  py::exec(R"pybind(
class BatchElementException(OpenAssetIOException):
    def __init__(self, index: int, error):
        self.index = index
        self.error = error
        super().__init__(error.message))pybind",
           mod.attr("__dict__"), mod.attr("__dict__"));

  // Retrieve a handle the the exception type just created by executing
  // the string literal above.
  const py::object pyBatchElementException = mod.attr("BatchElementException");

  using openassetio::errors::BatchElementEntityReferenceException;
  using openassetio::errors::BatchElementException;
  using openassetio::errors::EntityAccessErrorBatchElementException;
  using openassetio::errors::EntityResolutionErrorBatchElementException;
  using openassetio::errors::InvalidEntityReferenceBatchElementException;
  using openassetio::errors::InvalidPreflightHintBatchElementException;
  using openassetio::errors::InvalidTraitsDataBatchElementException;
  using openassetio::errors::InvalidTraitSetBatchElementException;
  using openassetio::errors::MalformedEntityReferenceBatchElementException;
  using openassetio::errors::UnknownBatchElementException;

  // Register a new exception type using `BatchElementException` base
  // class created above. Note that this is not sufficient to cause C++
  // exceptions to be translated. See `register_exception_translator`
  // below.
  const auto registerPyException = [&mod](const char *pyTypeName, const py::object &baseType) {
    return py::exception<void /* unused */>{mod, pyTypeName, baseType};
  };

  // Register each of our BatchElement exception types.
  // NOLINTNEXTLINE(bugprone-throw-keyword-missing)
  registerPyException("UnknownBatchElementException", pyBatchElementException);
  auto pyBatchElementEntityReferenceException =
      registerPyException("BatchElementEntityReferenceException", pyBatchElementException);
  // NOLINTNEXTLINE(bugprone-throw-keyword-missing)
  registerPyException("InvalidEntityReferenceBatchElementException",
                      pyBatchElementEntityReferenceException);
  // NOLINTNEXTLINE(bugprone-throw-keyword-missing)
  registerPyException("MalformedEntityReferenceBatchElementException",
                      pyBatchElementEntityReferenceException);
  // NOLINTNEXTLINE(bugprone-throw-keyword-missing)
  registerPyException("EntityAccessErrorBatchElementException",
                      pyBatchElementEntityReferenceException);
  // NOLINTNEXTLINE(bugprone-throw-keyword-missing)
  registerPyException("EntityResolutionErrorBatchElementException",
                      pyBatchElementEntityReferenceException);
  auto pyInvalidTraitsDataBatchElementException =
      registerPyException("InvalidTraitsDataBatchElementException", pyBatchElementException);
  // NOLINTNEXTLINE(bugprone-throw-keyword-missing)
  registerPyException("InvalidPreflightHintBatchElementException",
                      pyInvalidTraitsDataBatchElementException);
  // NOLINTNEXTLINE(bugprone-throw-keyword-missing)
  registerPyException("InvalidTraitSetBatchElementException", pyBatchElementException);

  // Register a function that will translate our C++ exceptions to the
  // appropriate Python exception type.
  //
  // Note that capturing lambdas are not allowed here, so we must
  // `import` the exception type in the body of the function.
  py::register_exception_translator([](std::exception_ptr pexc) {
    if (!pexc) {
      return;
    }
    const py::module_ pyModule = py::module_::import("openassetio._openassetio.errors");

    // Use CPython's PyErr_SetObject to set a custom exception type as
    // the currently active Python exception in this thread.
    const auto setPyException = [&pyModule](const char *pyTypeName, const auto &exc) {
      const py::object pyClass = pyModule.attr(pyTypeName);
      const py::object pyInstance = pyClass(exc.index, exc.error);
      PyErr_SetObject(pyClass.ptr(), pyInstance.ptr());
    };

    // Handle the different possible C++ exceptions, creating the
    // corresponding Python exception and setting it as the active
    // exception in this thread.
    try {
      std::rethrow_exception(std::move(pexc));
    } catch (const UnknownBatchElementException &exc) {
      setPyException("UnknownBatchElementException", exc);
    } catch (const InvalidEntityReferenceBatchElementException &exc) {
      setPyException("InvalidEntityReferenceBatchElementException", exc);
    } catch (const MalformedEntityReferenceBatchElementException &exc) {
      setPyException("MalformedEntityReferenceBatchElementException", exc);
    } catch (const EntityAccessErrorBatchElementException &exc) {
      setPyException("EntityAccessErrorBatchElementException", exc);
    } catch (const EntityResolutionErrorBatchElementException &exc) {
      setPyException("EntityResolutionErrorBatchElementException", exc);
    } catch (const InvalidPreflightHintBatchElementException &exc) {
      setPyException("InvalidPreflightHintBatchElementException", exc);
    } catch (const InvalidTraitSetBatchElementException &exc) {
      setPyException("InvalidTraitSetBatchElementException", exc);
    } catch (const InvalidTraitsDataBatchElementException &exc) {
      setPyException("InvalidTraitsDataBatchElementException", exc);
    } catch (const BatchElementEntityReferenceException &exc) {
      setPyException("BatchElementEntityReferenceException", exc);
    } catch (const BatchElementException &exc) {
      setPyException("BatchElementException", exc);
    }
  });
}
}  // namespace

void registerExceptions(const py::module &mod) {
  registerNonBatchExceptions(mod);
  // Make SURE the non batch exception is registered first, as that
  // includes the shared base exception.
  registerBatchElementExceptions(mod);
}
