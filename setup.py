#
#   Copyright 2013-2022 The Foundry Visionmongers Ltd
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
import os
import subprocess
import shlex
import sys

from setuptools import (
    setup,
    Extension,
    find_packages,
)

import setuptools.command.build_ext


class build_ext(setuptools.command.build_ext.build_ext):
    """
    Custom setuptools command class to puppet CMake.
    """

    def build_extension(self, _ext: Extension):
        """
        Hook called by setuptools to build the given Extension.

        Note: For develop installs to work with binary artifacts we must
        use "strict" editable installs, i.e. use
        `pip install --editable --config-settings editable_mode=strict`
        https://setuptools.pypa.io/en/latest/userguide/development_mode.html

        @param _ext: Extension to build (we only have one, so ignored).
        @return:
        """
        self.__cmake(
            [
                "-S",
                ".",
                "-B",
                self.build_temp,
                "-G",
                "Ninja",
                # Place output artifacts where setuptools expects.
                "--install-prefix",
                os.path.abspath(self.build_lib),
                "--preset",
                "setuptools",
                # Tell Cmake which python exectuable to use, as it's neccesary to use the same
                # python installation that setup.py was invoked with.
                "-DPython_EXECUTABLE=" + sys.executable,
                "-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15"
            ]
        )

        self.__cmake(["--build", self.build_temp, "--target", "openassetio-python-module"])

        self.__cmake(
            [
                "--install",
                self.build_temp,
                "--component",
                "openassetio-python-module",
            ]
        )

    def __cmake(self, args: list[str]):
        """
        Execute `cmake` as a subprocess with given arguments.

        @param args: Command-line arguments to pass to `cmake`.
        """
        args = ["cmake"] + args
        self.announce(" ".join(map(shlex.quote, args)), level=2)
        subprocess.check_call(args, env=os.environ.copy(), stderr=subprocess.STDOUT)

    def get_outputs(self):
        """
        Get list of output artifacts created by this command.

        For develop (`--editable`) installs, this is used to get the
        list of files to link to or copy from.

        We must override the base class because the default
        implementation uses `get_output_mapping`, which we override
        to return an empty dict (see below).

        @see get_output_mapping
        @return: List of output files.
        """
        return dict(self._get_output_mapping()).keys()

    def get_output_mapping(self):
        """
        Get the mapping from build artifacts back to original source
        files.

        Since build artifacts are generated (i.e. compiled from C++),
        there are no source files to link to, so the mapping is empty.

        For develop (`--editable`) installs, this is used to determine
        whether a file can be linked back to the source directory, or
        must be copied. Ensuring we return a blank mapping means the
        output artifacts are always copied.

        We must override this from the base class since the default
        implementation generates a mapping expecting the source tree to
        contain the binary artifacts pre-built at the root of the
        package.

        @return: Mapping of build artifacts to corresponding source
        artifact (i.e. blank in this case).
        """
        return {}


setup(
    name="openassetio",
    version="1.0.0-alpha.4",
    packages=find_packages(where="python"),
    package_dir={"": "python"},
    python_requires=">=3.9",
    ext_modules=[Extension("openassetio._openassetio", sources=[])],
    cmdclass={"build_ext": build_ext},
)
