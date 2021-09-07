"""
This file is a part of the Kithare programming language source code.
The source code for Kithare programming language is distributed under the MIT
license.
Copyright (C) 2021 Kithare Organization

builder/cflags.py
Defines a CompilerFlags class for handling cflags
"""


import json
import os
from pathlib import Path
from typing import Union


class CompilerFlags:
    """
    A CompilerFlags class
    """

    def __init__(self, basepath: Path):
        """
        Initialise CompilerFlags instance. This has attributes of individual
        compiler flags
        """
        self.basepath = basepath

        # C Compiler name, gcc by default
        self.cc: Union[str, Path] = "gcc"

        # C++ Compiler name, g++ by default
        self.cxx: Union[str, Path] = "g++"

        # Windres command
        self.windres: Union[str, Path] = "windres"

        # flags for the C compiler, any Path object here is interpeted as -I
        self.cflags: list[Union[str, Path]] = []

        # flags for the C++ compiler, any Path object here is interpeted as -I
        self.cxxflags: list[Union[str, Path]] = []

        # flags for the C AND C++ compiler, any Path object here is interpeted as -I
        self.ccflags: list[Union[str, Path]] = []

        # flags for the C PreProcessor
        self.cppflags: list[str] = []

        # flags for the linker, any Path object here is interpeted as -L
        self.ldflags: list[Union[str, Path]] = []

    def get_compiler(self, ext: str = "cpp"):
        """
        Get compiler by file extension. Returns self.cc for C files and
        self.cxx for cpp files
        """
        if ext.endswith("c"):
            return str(self.cc)

        if ext.endswith("cpp"):
            return str(self.cxx)

        raise RuntimeError(f"InternalError: Function 'get_compiler' got arg {ext}")

    @classmethod
    def from_json(cls, basepath: Path, jsondata: Union[Path, str]):
        """
        Get CompilerFlags object from json dict representation
        """
        if isinstance(jsondata, Path):
            try:
                jsondata = jsondata.read_text()
            except FileNotFoundError:
                jsondata = "null"

        try:
            obj = json.loads(jsondata)
        except json.JSONDecodeError:
            obj = None

        ret = cls(basepath)
        if isinstance(obj, list):
            # a list was the old way of storing CompilerFlags in json, now this
            # is merely kept for compatability
            ret.ccflags.extend(obj)

        elif isinstance(obj, dict):
            ret.from_dict(**obj)

        return ret

    def from_dict(self, **dictobj: Union[str, list]):
        """
        Fill in attributes from a dict to the CompilerFlags object
        """
        cc = dictobj.get("CC")
        if isinstance(cc, str):
            self.cc = cc

        cxx = dictobj.get("CXX")
        if isinstance(cxx, str):
            self.cxx = cxx

        for attr, key in (
            (self.ccflags, "CCFLAGS"),
            (self.ldflags, "LDFLAGS"),
            (self.ldflags, "LDLIBS"),
            (self.cflags, "CFLAGS"),
            (self.cxxflags, "CXXFLAGS"),
            (self.cppflags, "CPPFLAGS"),
        ):
            try:
                obj = dictobj[key]
            except KeyError:
                pass
            else:
                if isinstance(obj, str):
                    obj = obj.split()

                attr.extend(obj)

    def load_from_env(self):
        """
        Load any CompilerFlags from environment variables
        """
        self.from_dict(**os.environ)

    def resolve_paths(
        self, *flags: Union[str, Path], hflag: str = "-I", rel_base: bool = False
    ):
        """
        Resolve path objects in a list, and return an iterator
        """
        for flag in flags:
            if isinstance(flag, str):
                yield flag
                continue

            if rel_base:
                flag = flag.relative_to(self.basepath)

            yield hflag + str(flag)

    def __eq__(self, other: object):
        """
        Compare two CompilerFlags objects
        """
        if not isinstance(other, CompilerFlags):
            return NotImplemented

        cc = self.cc if isinstance(self.cc, str) else self.cc.relative_to(self.basepath)
        cxx = (
            self.cxx
            if isinstance(self.cxx, str)
            else self.cxx.relative_to(self.basepath)
        )

        occ = (
            other.cc
            if isinstance(other.cc, str)
            else other.cc.relative_to(other.basepath)
        )
        ocxx = (
            other.cxx
            if isinstance(other.cxx, str)
            else other.cxx.relative_to(other.basepath)
        )

        if str(cc) != str(occ) or str(cxx) != str(ocxx):
            return False

        for attr, other_attr in (
            (self.cflags, other.cflags),
            (self.cxxflags, other.cxxflags),
        ):
            if sorted(
                self.resolve_paths(*attr, *self.ccflags, rel_base=True)
            ) != sorted(self.resolve_paths(*other_attr, *other.ccflags, rel_base=True)):
                return False

        if sorted(
            self.resolve_paths(*self.ldflags, hflag="-L", rel_base=True)
        ) != sorted(self.resolve_paths(*other.ldflags, hflag="-L", rel_base=True)):
            return False

        return sorted(self.cppflags) == sorted(other.cppflags)

    def to_json(self, file: Path):
        """
        Write a file with a JSON dict representation of the CompilerFlags
        object
        """
        cc = self.cc if isinstance(self.cc, str) else self.cc.relative_to(self.basepath)
        cxx = (
            self.cxx
            if isinstance(self.cxx, str)
            else self.cxx.relative_to(self.basepath)
        )

        file.write_text(
            json.dumps(
                {
                    "CC": str(cc),
                    "CXX": str(cxx),
                    "CPPFLAGS": self.cppflags,
                    "CCFLAGS": list(self.resolve_paths(*self.ccflags, rel_base=True)),
                    "CFLAGS": list(self.resolve_paths(*self.cflags, rel_base=True)),
                    "CXXFLAGS": list(self.resolve_paths(*self.cxxflags, rel_base=True)),
                    "LDFLAGS": list(
                        self.resolve_paths(*self.ldflags, hflag="-L", rel_base=True)
                    ),
                },
                indent=4,
            )
        )

    def flags_by_ext(self, ext: str):
        """
        Get the appropriate compiler flags for compilation based on file
        extension. rel_base arg specifies whether the paths should be relative
        to base dir, or current working dir
        """
        if ext.endswith("c"):
            yield from self.cppflags
            yield from self.resolve_paths(*self.ccflags)
            yield from self.resolve_paths(*self.cflags)

        if ext.endswith("cpp"):
            yield from self.cppflags
            yield from self.resolve_paths(*self.ccflags)
            yield from self.resolve_paths(*self.cxxflags)

        if ext.endswith("o"):
            yield from self.resolve_paths(*self.ldflags, hflag="-L")
