// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 The Foundry Visionmongers Ltd
#pragma once

#include <Python.h>

#include <openassetio/export.h>
#include <openassetio/python/export.h>
#include <openassetio/typedefs.hpp>

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
/**
 * Converter functionality for going between C++ and CPython objects.
 */
namespace python::converter {
/**
 * Casts a C++ API object to the equivalent Python object.
 *
 * This template is explicitly instantiated to only the OpenAssetIO
 * types, and is not intended to be a generic converter.
 *
 * The purpose of this function is allow Python/C++ conversion whilst
 * hiding the pybind11 dependency, allowing consumers to retrieve a
 * Python object without having to have pybind in their build stack. A
 * pybind cast is done behind the scenes, returning the released ptr
 * from that.
 *
 * @note This function does not acquire the GIL.
 *
 * @warning A Python environment, with openassetio imported, must be
 *          available in order to use this function.
 *
 * @throws std::invalid_argument if the input is null.
 * @throws std::runtime_error if the cast fails. The .what() of this
 *         exception is the Python exception string.
 *
 * @param objectPtr An OpenAssetIO pointer type, (eg, @needsref
 * ManagerPtr). The returned pointer takes shared ownership of this
 * input \p objectPtr, and will keep the c++ instance alive until the
 * `PyObject` is destroyed.
 *
 * @return A `PyObject` pointer to an object of the Python api type
 * associated with the c++ api object provided.
 * The pointer is not an RAII type, and will be returned with an
 * incremented reference count for the caller to manage,  and must
 * must be released via `Py_DECREF` or similar mechanisms.
 */
template <typename T>
OPENASSETIO_PYTHON_BRIDGE_EXPORT PyObject* castToPyObject(const T& objectPtr);

/**
 * Casts a Python object to the equivalent C++ Api object.
 *
 * This template is explicitly instantiated to only the OpenAssetIO
 * types, and is not intended to be a generic converter.
 *
 * The purpose of this function is to Python/C++ conversion whilst
 * hiding the pybind11 dependency, allowing consumers to retrieve a
 * Python object without having to have pybind in their build stack. A
 * pybind cast is done behind the scenes, returning the C++ object
 * pointer from that.
 *
 * This function will increment the reference count of the provided
 * Python object for the lifetime of the returned C++ object.
 * When the returned C++ object falls out of scope or is otherwise
 * cleaned up, the Python object will have its reference count reduced
 * by one, potentially invoking cleanup.
 *
 * Using this function requires specifying the template argument of
 * the C++ API type equivalent to the type of the object referred to by
 * the \p pyObject pointer.
 *
 * @code{.cpp}
 * ManagerPtr manager = castFromPyObject<Manager>(pyManager);
 * @endcode
 *
 *
 * If the types of the template argument and the \p pyObject are not
 * equivalent, an exception will be thrown due to inability to perform
 * the cast.
 *
 * @note This function does not acquire the GIL.
 *
 * @warning A Python environment, with openassetio imported, must be
 *          available in order to use this function.
 *
 * @throws std::invalid_argument if the function fails due to inability
 * to cast between types, or if the input is null.
 *
 * @param pyObject A `PyObject` pointer to a Python object that must be
 * of equivalent type to the template argument.
 *
 * @return An OpenAssetIO pointer to a C++ API object cast from the
 * provided \p pyObject. The lifetime of the PyObject will be extended
 * to at least the lifetime of the returned C++ object.
 */
template <typename T>
OPENASSETIO_PYTHON_BRIDGE_EXPORT typename T::Ptr castFromPyObject(PyObject* pyObject);

}  // namespace python::converter
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
