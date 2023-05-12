// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 The Foundry Visionmongers Ltd

#include <pybind11/embed.h>

#include <openassetio/export.h>

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include <openassetio/python/converter/converters.hpp>

#include <openassetio/Context.hpp>
#include <openassetio/TraitsData.hpp>
#include <openassetio/hostApi/HostInterface.hpp>
#include <openassetio/hostApi/Manager.hpp>
#include <openassetio/hostApi/ManagerFactory.hpp>
#include <openassetio/hostApi/ManagerImplementationFactoryInterface.hpp>
#include <openassetio/log/ConsoleLogger.hpp>
#include <openassetio/log/LoggerInterface.hpp>
#include <openassetio/log/SeverityFilter.hpp>
#include <openassetio/managerApi/Host.hpp>
#include <openassetio/managerApi/HostSession.hpp>
#include <openassetio/managerApi/ManagerInterface.hpp>
#include <openassetio/managerApi/ManagerStateBase.hpp>
#include <openassetio/python/hostApi.hpp>

namespace py = pybind11;

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
namespace {
constexpr const char* kTestTraitId = "TestTrait";
}  // namespace

SCENARIO("Mutations in one language are reflected in the other") {
  // Cause the openassetio-python lib to be loaded so pybind can cast.
  py::module_::import("openassetio");

  GIVEN("A C++ object casted to a python object") {
    TraitsDataPtr traitsData = TraitsData::make();
    PyObject* pyTraitsData = openassetio::python::converter::castToPyObject(traitsData);
    REQUIRE(pyTraitsData != nullptr);

    WHEN("Data is set via the c++ object") {
      traitsData->addTrait(kTestTraitId);

      THEN("The python object reflects that data set.") {
        PyObject* resultBool = PyObject_CallMethod(pyTraitsData, "hasTrait", "s", kTestTraitId);

        // Use CHECK rather than REQUIRE in order to ensure the Decrefs
        // below actually execute.
        CHECK(PyBool_Check(resultBool));
        CHECK(PyObject_IsTrue(resultBool));
        Py_DECREF(resultBool);
      }
    }
    WHEN("Data is set via the python object") {
      PyObject_CallMethod(pyTraitsData, "addTrait", "s", kTestTraitId);
      CHECK(traitsData->hasTrait("") == false);
    }
    Py_DECREF(pyTraitsData);
  }
  GIVEN("A python object casted to a C++ object") {
    const py::object pyClass = py::module_::import("openassetio").attr("TraitsData");
    py::object pyInstance = pyClass();
    PyObject* pyTraitsData = pyInstance.release().ptr();
    TraitsDataPtr traitsData =
        openassetio::python::converter::castFromPyObject<TraitsData>(pyTraitsData);

    WHEN("Data is set via the c++ object") {
      traitsData->addTrait(kTestTraitId);

      THEN("The python object reflects that data set.") {
        PyObject* resultBool = PyObject_CallMethod(pyTraitsData, "hasTrait", "s", kTestTraitId);

        // Use CHECK rather than REQUIRE in order to ensure the Decrefs
        // below actually execute.
        CHECK(PyBool_Check(resultBool));
        CHECK(PyObject_IsTrue(resultBool));
        Py_DECREF(resultBool);
      }
    }
    WHEN("Data is set via the python object") {
      PyObject_CallMethod(pyTraitsData, "addTrait", "s", kTestTraitId);
      CHECK(traitsData->hasTrait("") == false);
    }
    Py_DECREF(pyTraitsData);
  }
}

SCENARIO("Casting to PyObject extends object lifetime") {
  GIVEN("A python object casted from a C++ object") {
    WHEN("The C++ object falls out of scope") {
      PyObject* pyTraitsData = nullptr;
      {
        TraitsDataPtr traitsData = TraitsData::make();
        traitsData->addTrait(kTestTraitId);
        pyTraitsData = openassetio::python::converter::castToPyObject(traitsData);
        CHECK(Py_REFCNT(pyTraitsData) == 1);
      }
      THEN("The python object remains alive and can be operated on via the python interpreter") {
        CHECK(Py_REFCNT(pyTraitsData) == 1);
        PyObject* resultBool = PyObject_CallMethod(pyTraitsData, "hasTrait", "s", kTestTraitId);

        // Use CHECK rather than REQUIRE in order to ensure the Decrefs
        // below actually execute.
        CHECK(PyBool_Check(resultBool));
        CHECK(PyObject_IsTrue(resultBool));
        Py_DECREF(resultBool);
      }
      Py_DECREF(pyTraitsData);
    }
  }
}

SCENARIO("Casting to a C++ object binds object lifetime") {
  GIVEN("A python object") {
    py::module_::import("openassetio");

    // Create a python object and release it, to simulate an unmanaged
    // PyObject being provided to us.
    const py::object pyClass = py::module_::import("openassetio").attr("TraitsData");
    py::object pyInstance = pyClass();
    PyObject* pyTraitsData = pyInstance.release().ptr();
    CHECK(Py_REFCNT(pyTraitsData) == 1);

    WHEN("The python object is converted to a C++ object") {
      TraitsDataPtr traitsData =
          openassetio::python::converter::castFromPyObject<TraitsData>(pyTraitsData);

      THEN("The python reference count has been increased") {
        CHECK(Py_REFCNT(pyTraitsData) == 2);
      }
    }
    AND_WHEN("The c++ object falls out of scope") {
      THEN("The python reference count is reduced") { CHECK(Py_REFCNT(pyTraitsData) == 1); }
    }
    Py_DECREF(pyTraitsData);
  }
}

SCENARIO("Attempting to cast to an unregistered type") {
  GIVEN("an invalid for casting python object") {
    // Create a python object and release it, to simulate an unmanaged
    // PyObject being provided to us.
    py::object pyClass = py::module_::import("decimal").attr("Decimal");
    py::object pyInstance = pyClass(1.0);
    // PyDecimal is not a type pybind has been registered to convert.
    PyObject* pyDecimal = pyInstance.release().ptr();

    /*
     * Pybind error messages vary between release and debug mode:
     * "Unable to cast Python instance of type <class 'decimal.Decimal'> to C++
     * type 'openassetio::v1::hostApi::Manager'"
     * vs.
     * "Unable to cast Python instance to C++ type (compile in debug
     * mode for details)"
     */
    WHEN("The object is converted to a C++ object") {
      REQUIRE_THROWS_WITH(
          openassetio::python::converter::castFromPyObject<openassetio::hostApi::Manager>(
              pyDecimal),
          Catch::Matchers::StartsWith("Unable to cast Python instance"));
    }

    Py_DECREF(pyDecimal);
  }
}

SCENARIO("Converting from C++ API Objects to Python API Objects without openassetio module loaded",
         "[.][no_openassetio_module]") {
  GIVEN("A python environment without openassetio loaded") {
    const TraitsDataPtr traitsData = TraitsData::make();
    WHEN("An OpenAssetIO type is casted to a python object") {
      THEN("The cast throws an exception") {
        REQUIRE_THROWS_WITH(openassetio::python::converter::castToPyObject(traitsData),
                            std::string("Unregistered type : openassetio::v1::TraitsData"));
      }
    }
  }

  GIVEN("A CPython error state is already set") {
    // If the process terminates with a CPython error, pybind will emit
    // an exception on destruction, causing a terminate call.
    // As we explicitly set an error in this test, use a scope to make
    // sure we clean up after ourselves.
    py::error_scope errorCleanup;

    const std::string errorString = "Test Error";
    // Set the CPython exception.
    PyErr_SetString(PyExc_RuntimeError, errorString.c_str());
    WHEN("The cpp object is casted to a python object") {
      THEN("The cast throws the expected exception") {
        const TraitsDataPtr traitsData = TraitsData::make();
        REQUIRE_THROWS_WITH(openassetio::python::converter::castToPyObject(traitsData),
                            std::string("Unregistered type : openassetio::v1::TraitsData"));
      }
      AND_THEN("The CPython error state is maintained") {
        // castToPyObject, despite messing with the PyErr state itself,
        // should have correctly reset the exception back.
        REQUIRE(PyErr_ExceptionMatches(PyExc_RuntimeError));
        py::error_scope errors;
        REQUIRE(std::string(py::str(errors.value)) == errorString);
      }
    }
  }
}

namespace {

#define CLASS_AND_PTRS(Class) std::tuple<Class, Class##Ptr, Class##ConstPtr>

namespace hostApi = openassetio::hostApi;
namespace log = openassetio::log;
namespace managerApi = openassetio::managerApi;

// clang-format off

using ClassesWithPtrAlias = std::tuple<
    CLASS_AND_PTRS(openassetio::Context),
    CLASS_AND_PTRS(openassetio::TraitsData),
    CLASS_AND_PTRS(hostApi::HostInterface),
    CLASS_AND_PTRS(hostApi::Manager),
    CLASS_AND_PTRS(hostApi::ManagerFactory),
    CLASS_AND_PTRS(hostApi::ManagerImplementationFactoryInterface),
    CLASS_AND_PTRS(log::ConsoleLogger),
    CLASS_AND_PTRS(log::LoggerInterface),
    CLASS_AND_PTRS(log::SeverityFilter),
    CLASS_AND_PTRS(managerApi::Host),
    CLASS_AND_PTRS(managerApi::HostSession),
    CLASS_AND_PTRS(managerApi::ManagerInterface),
    CLASS_AND_PTRS(managerApi::ManagerStateBase)
>;
}  // namespace

// clang-format on
TEMPLATE_LIST_TEST_CASE("Appropriate classes have castFromPyObject functions", "",
                        ClassesWithPtrAlias) {
  using Class = std::tuple_element_t<0, TestType>;

  // These tests check that the nullptr exception works, but also
  // serve to verify that the functions exist for all expected types.
  PyObject* empty = nullptr;
  REQUIRE_THROWS_WITH(openassetio::python::converter::castFromPyObject<Class>(empty),
                      std::string("pyObject cannot be null"));
}

TEMPLATE_LIST_TEST_CASE("Appropriate classes have castToPyObject functions", "",
                        ClassesWithPtrAlias) {
  using ClassPtr = std::tuple_element_t<1, TestType>;

  // These tests check that the nullptr exception works, but also
  // serve to verify that the functions exist for all expected types.
  ClassPtr empty = nullptr;
  REQUIRE_THROWS_WITH(openassetio::python::converter::castToPyObject(empty),
                      std::string("objectPtr cannot be null"));
}
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
