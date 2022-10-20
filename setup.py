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
import inspect
import os
import subprocess
import shlex

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

        @param _ext: Extension to build (we only have one, so ignored).
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


setup(
    name="openassetio",
    version="1.0.0a4",
    description=(
        "An open-source interoperability standard for tools and content management systems used in"
        " media production."
    ),
    long_description=inspect.cleandoc(
        """ In modern creative pipelines, data is often managed by an authoritative system (Asset
            Management System, Digital Asset Manager, MAM, et. al).

            It is common for media creation tools to reference this managed data by its present
            location in a file system.

            OpenAssetIO enables tools to reference managed data by identity (using an "Entity
            Reference") instead of a file system path.

            This is achieved through the definition of a common set of interactions between a host
            of the API (eg: a Digital Content Creation tool or pipeline script) and an Asset
            Management System.

            This common API surface area removes the need for common pipeline business logic to be
            re-implemented against the native API of each tool, and allows the tools themselves to
            design new workflows that streamline the creation of complex assets."""
    ),
    url="https://github.com/OpenAssetIO/OpenAssetIO",
    author="The Foundry Visionmongers Ltd",
    packages=find_packages(where="python"),
    classifiers=[  # See https://pypi.org/classifiers
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Topic :: Software Development :: Build Tools",
        "License :: OSI Approved :: Apache Software License",
        "Natural Language :: English",
        "Operating System :: MacOS",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: Software Development :: Object Brokering",
        "Topic :: System :: Distributed Computing",
        "Topic :: System :: Filesystems",
    ],
    keywords="asset ams dam mam pipeline dcc media resolver",
    project_urls={
        "Source": "https://github.com/OpenAssetIO/OpenAssetIO",
        "Documentation": "https://openassetio.github.io/OpenAssetIO",
        "Issues": "https://github.com/OpenAssetIO/OpenAssetIO/issues",
    },
    package_dir={"": "python"},
    python_requires=">=3.9",
    ext_modules=[Extension("openassetio._openassetio", sources=[])],
    cmdclass={"build_ext": build_ext},
)
