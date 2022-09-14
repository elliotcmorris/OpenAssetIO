#
#   Copyright 2022 The Foundry Visionmongers Ltd
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
@namespace openassetio.codegen
A tool for automatic generation of Traits and Specification classes from
simple YAML descriptions.

The codegen tool provides a quick way to generate the boilerplate code
that forms strongly-typed views on TraitsData objects. Specification and
Trait classes can be reliably generated from a simple plain-text
description.

This package provides several ways to invoke code generation:

 - The `openassetio-codegen` CLI.
 - The `generate` entrypoint in this module.
 - Custom generation scripts that make use of the `parser` and
   `generators` modules.

In order to standardize naming conventions and mitigate against Trait ID
collisions across global usage of OpenAssetIO, the codegen tool
establishes some conventions around these descriptions:

 - Each description defines a 'package', with a unique name.
 - A package can contain any number of Traits or Specifications.
 - Traits and Specifications are grouped into simple, flat namespaces.
 - Each Trait or Specification must have a unique name within its
   namespace, but names can be reused across namespaces or packages.
 - Trait IDs are formed by combining the package name, namespace and
   Trait name.
 - Trait accessors are prefixed with namespace and package in the
   case of internal or cross-package collisions within any given
   Specification's TraitSet.
 - Generated code follows the OpenAssetIO style guide for target
   languages.
 - Names are constrained to alpha-numeric characters to prevent
   collisions when names are folded to the restricted character sets of
   languages such as Python, C, etc.

Note: The above conventions are imposed entirely by the
`openassetio-codegen` tool. OpenAssetIO itself has no constraints on
Trait IDs or property names.

The default generation process is split into three phases:

 1. A source description (YAML) is loaded, and validated against the
    openassetio-codegen JSON Schema (see schema.json).
 2. This description is converted to an intermediate PackageDeclaration
    (see datamodel.py).
 3. This declaration is passed to one or more generators to produce
    language-specific implementations.

If you wish to write a custom generation process, from a different
source, for example, the implementation of `generate` below may form a
useful reference.
"""

import logging

from . import parser
from . import generators


#
# Code Generation
#


# pylint: disable=too-many-arguments
def generate(
    description_path: str,
    output_directory: str,
    languages: list[str],
    creation_callback,
    logger: logging.Logger,
    dry_run: bool = False,
    template_globals=None,
):
    """
    A high-level entry point into code generation. This can be used for
    programmatic generation that mirrors the behaviour of the CLI.

    Generates implementations for the Traits and/or Specifications in
    the supplied description.

    @param description_path: The path to a YAML package description
        conforming the schema.json
    @param output_directory: The root output directory for code,
        language generators will place their output into one or more
        subdirectories.
    @param languages: A list of languages to generate, see
        generators.ALL
    @param creation_callback: A callback, called with the path to each
        directory or file generated by each invoked language generator.

    @param logger: All messaging will be submitted to the supplied
        logger.

    @param dry_run: When enabled, description parsing and validation
        will be performed, but no code generated. Increase log-level to
        INFO to inspect package structure.

    @param template_globals: Any additional globals to pass to the
        generation templates. Currently used fields are:
          - copyrightOwner: str [""] The copyright owner, if set a
            copyright notice and SPDX License Identifier will be added
            to the generated code.
          - copyrightDate: str [<current_year>] The copyright year(s) if
            set, otherwise the current year will be used.
          - spdxLicenseIdentifier: str ["Apache-2.0"] The SPDX license
            identifier under which the code is licensed. (see:
            https://spdx.org/licenses)
    """

    # Retrieve the package structure from the YAML file
    package_description = parser.load_yaml(description_path)
    # Validate this against the published schema
    parser.validate_package_description(package_description)
    # Build the intermediate representation for the generators
    package_declaration = parser.build_package_declaration(package_description)

    # As a convenience, log the parsed structure
    _log_package_declaration(package_declaration, logger)

    if dry_run:
        return

    # Generate all requested languages
    for language in languages:

        logger.info("Generating %s...", language)
        child_logger = logger.getChild(language)

        # Derive template globals for things such as copyright, etc...
        globals_ = generators.helpers.default_template_globals()
        if template_globals:
            globals_.update(template_globals)
        globals_["language"] = language

        # Retrieve the generator by looking up an attribute with the
        # requested language name and generate.
        generator = getattr(generators, language)
        generator.generate(
            package_declaration, globals_, output_directory, creation_callback, child_logger
        )


#
# Helpers
#


def _log_package_declaration(package, logger):
    """
    Logs a description of the supplied package declaration.
    A high-level structure outline is output through INFO messages.
    """
    logger.info(f"Package: {package.id}")
    if package.traits:
        logger.info("Traits:")
        for namespace in package.traits:
            logger.info(f"{namespace.id}:")
            for trait in namespace.members:
                logger.info("  - %s", trait.name)
    if package.specifications:
        logger.info("Specifications:")
        for namespace in package.specifications:
            logger.info(f"{namespace.id}:")
            for specification in namespace.members:
                logger.info("  - %s", specification.id)
