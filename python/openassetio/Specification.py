#
#   Copyright 2013-2021 The Foundry Visionmongers Ltd
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

from .SpecificationFactory import SpecificationFactory

## @todo [TC] Though the behavior of Specification is largely correct, the
## implementation needs re-thinking. The relationship between
## SpecificationBase and Specification doesn't really make sense.
## Especially the declaration of `_type` and `_prefix` which then end
## up duplicated in `__schema`. We still need those components, but
## there should be a cleaner way to implement this. It's going to need
## to be re-thought to allow runtime extensions in python or C++ anyway
## so we can hopefully pick it up then.

# We want these properties to be available here, so people just deriving a
# 'Specification' don't need to worry about where these properties really come
# from - we don't want most people to have to care about the 'core' module.
from ._core.objects import UntypedProperty, TypedProperty, FixedInterfaceObject


__all__ = ['SpecificationBase', 'Specification', 'UntypedProperty', 'TypedProperty']


class SpecificationBase(FixedInterfaceObject):
    """
    The simplest of Specifications that just holds the data store, and
    schema type. This can be used in cases that have no need to work
    with the data in any type-specific way.
    """

    _data = {}

    def __init__(self, schema, data=None):

        self.__schema = schema
        # The default for data is None, not {} to avoid mutable defaults issues
        # This data is written to by the SpecificationProperty class
        self._data = data if data else {}

    def schema(self):
        """
        @return str, The schema identifier for the data held in the
        specification.
        """
        return self.__schema

    def data(self, copy=True):
        """
        @param copy bool, When True (default) then a copy of the data
        will be returned, rather than a reference, to help avoid
        mutating the specifications data by accident.

        @return dict, The data of the specification.
        """
        if copy:
            return dict(self._data)

        return self._data

    def _setSchema(self, schema):
        self.__schema = schema

    def __str__(self):
        data = []
        for key, value in self._data.items():
            if value is not None:
                data.append("'%s':%s" % (key, repr(value)))
        return "SpecificationBase('%s', {%s})" % (self.__schema, ", ".join(data))

    def __repr__(self):
        return str(self)


class Specification(SpecificationBase):
    """
    The simplest form of Specification in common use. It extends the
    base specification to better define the schema.

    If introduces the notion that the @ref Specification schema consists
    of two parts, separated by a token - the 'prefix' and 'type'. For
    example:

      @li `core.locale:image.catalog`
      @li **prefix**: `core.locale`
      @li **type**: `image.catalog`

    This is to allow common prefixes to be best represented by a single
    derived class. For example, several locales would have different
    types, but the same prefix. This allows the generic @ref
    openassetio.specifications.LocaleSpecification to be used to
    manipulate these objects.

    Types should also be hierarchical, in a way that indicates
    compatibility with the class hierarchy.  It should be that a
    file.image.texture can degenerate into a file.image if necessary. As
    such, a properties of a more derived specification should always be
    a superset of any other specification indicated by its 'type'.

    The @ref SpecificationFactory understands the concept of prefixes,
    etc... when wrapping on instantiating a Specification from data.
    """
    __metaclass__ = SpecificationFactory

    _prefix = "core"
    _type = ""
    __kPrefixSeparator = ':'

    def __init__(self, data=None):
        schema = self.generateSchema(self._prefix, self._type)
        super(Specification, self).__init__(schema, data)

    def __str__(self):
        data = []
        for key, value in self._data.items():
            if value is not None:
                data.append("'%s':%s" % (key, repr(value)))
        return "%s({%s})" % (self.__class__.__name__, ", ".join(data))

    def __repr__(self):
        return str(self)

    def isOfType(self, typeOrTypes, includeDerived=True, prefix=None):
        """
        Returns whether the specification is of a requested type, by
        comparison of the type string.

        @param typeOrTypes [str or Specification, or list of], The types
        to compare against. eg:
         @code
            spec.isOfType(openassetio.specifications.FileSpecification)
            spec.isOfType(('file', 'group.shot'), includeDerived=True)
         @endcode

        @param includeDerived bool, If True, then the match will include
        any specializations of the supplied type. For example if you
        used a typeString of "file", a specification of type
        "file.image" would still match. If this is false, it must be the
        exact type match.

        @param prefix str, An optional prefix string, to allow complete
        comparison of the schema, not just the type.

        @note This call doesn't not consider the 'prefix' of the
        Specification, unless the additional 'prefix' argument is
        supplied.
        """
        if self._type and self._prefix:
            ourPrefix = self._prefix
            ourType = self._type
        else:
            ourPrefix, ourType = self.schemaComponents(self.schema())

        if prefix and not prefix == ourPrefix:
            return False

        if not isinstance(typeOrTypes, (list, tuple)):
            typeOrTypes = (typeOrTypes,)

        for typ in typeOrTypes:
            if inspect.isclass(typ) and issubclass(typ, Specification):
                typ = typ._type # pylint: disable=protected-access
            if includeDerived:
                if ourType.startswith(typ):
                    return True
            elif ourType == typ:
                return True

        return False

    def field(self, name, defaultValue=None):
        """
        Fetches the property from the specification, if present,
        otherwise returns the default value.

        This is short hand for the following code, that avoids either
        copying the data, or exposing the mutable data dictionary.
        Consequently, it should be used by preference.

        @code
        data = specification.data(copy=False).get(name, defaultValue)
        @endcode
        """
        return self._data.get(name, defaultValue)

    @classmethod
    def generateSchema(cls, prefix, typ):
        """
        To be used over naive string concatenation to build a schema
        string.

        @return str, The schema string for the given prefix and type.
        """
        return "%s%s%s" % (prefix, cls.__kPrefixSeparator, typ)

    @classmethod
    def schemaComponents(cls, schema):
        """
        Splits a schema string into a prefix, type tuple. Should be used
        over manual attempts at tokenization.

        @return tuple, (prefix, type), The prefix will be an empty
        string if there is none.
        """
        if cls.__kPrefixSeparator in schema:
            return schema.rsplit(cls.__kPrefixSeparator, 1)

        return "", schema

    def prefix(self):
        """
        @return str, the prefix of this specifications schema, or an
        empty string.
        """
        return self.schemaComponents(self.__schema)[0]

    def type(self):
        """
        @return str, the schemas type, without prefix or separator
        token.
        """
        return self.schemaComponents(self.__schema)[1]
