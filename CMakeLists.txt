cmake_minimum_required(VERSION 3.17)
project(ZOSRework)

set(CMAKE_CXX_STANDARD 20)

add_executable(ZOSRework main.cpp
        filesystem/global/Globals.hpp
        filesystem/io/AddressType.h
        filesystem/io/PathContext.cpp
        filesystem/io/PathContext.hpp
        filesystem/io/model/INode.cpp
        filesystem/io/model/INode.hpp
        filesystem/io/model/FolderItem.cpp
        filesystem/io/model/FolderItem.hpp
        filesystem/util/FSException.cpp
        filesystem/util/FSException.hpp
        filesystem/util/ConversionUtils.hpp
        filesystem/io/model/SuperBlock.cpp
        filesystem/io/model/SuperBlock.hpp
        filesystem/io/model/Bitmap.cpp
        filesystem/io/model/Bitmap.hpp
        filesystem/components/PathInfo.cpp
        filesystem/components/PathInfo.hpp
        filesystem/components/FileSystemController.cpp
        filesystem/components/FileSystemController.hpp
        filesystem/cli/CLI.cpp
        filesystem/cli/CLI.hpp
        filesystem/components/FileSystem.cpp
        filesystem/components/FileSystem.hpp
        filesystem/components/MemoryAllocator.cpp
        filesystem/components/MemoryAllocator.hpp
        filesystem/components/INodeIO.cpp
        filesystem/components/INodeIO.hpp filesystem/util/StringParsing.hpp filesystem/util/FileStream.cpp filesystem/util/FileStream.hpp)