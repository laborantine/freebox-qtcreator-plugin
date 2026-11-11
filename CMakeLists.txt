# Windows / Qt Creator 20.0.2 port, 2026-11-11.
cmake_minimum_required(VERSION 3.24)
project(Freebox LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.11 COMPONENTS Core Gui Network Svg Widgets REQUIRED)
find_package(QtCreator REQUIRED COMPONENTS Core ProjectExplorer QtSupport Debugger QmlProfiler)
if(NOT IDE_VERSION VERSION_EQUAL "20.0.2")
    message(FATAL_ERROR "This port targets Qt Creator 20.0.2.")
endif()

add_qtc_plugin(Freebox
    PLUGIN_DEPENDS
        QtCreator::Core QtCreator::ProjectExplorer QtCreator::QtSupport
        QtCreator::Debugger QtCreator::QmlProfiler
    DEPENDS
        Qt::Network Qt::Widgets QtCreator::ExtensionSystem QtCreator::QmlJS QtCreator::Utils
    INCLUDES . src
    SOURCES
        freebox.qrc
        fileformat/filefilteritems.cc fileformat/filefilteritems.hh
        fileformat/manifest.cc fileformat/manifest.hh
        fileformat/qmlprojectfileformat.cpp fileformat/qmlprojectfileformat.hh
        fileformat/qmlprojectitem.cpp fileformat/qmlprojectitem.hh
        http/server.cc http/server.hh
        remote/remoteqml.cc remote/remoteqml.hh
        ssdp/ssdpclient.cc ssdp/ssdpclient.hh
        ssdp/ssdpmessage.cc ssdp/ssdpmessage.hh
        ssdp/ssdpsearch.cc ssdp/ssdpsearch.hh
        ssdp/udplistener.cc ssdp/udplistener.hh
        ssdp/udpsocket.cc ssdp/udpsocket.hh
        src/configuration.cc src/configuration.hh
        src/device.cc src/device.hh
        src/freeboxplugin.cc src/freeboxplugin.hh
        src/freestorepackager.cc src/freestorepackager.hh
        src/project.cc src/project.hh
        src/runconfigurationfactory.cc src/runconfigurationfactory.hh
        src/runcontrol.cc src/runcontrol.hh
        src/tar.cc src/tar.hh
        util/crc32.c util/crc32.h
        util/gzipper.cc util/gzipper.hh
        util/jsonrpc.cc util/jsonrpc.hh
)
set_target_properties(Freebox PROPERTIES OUTPUT_NAME Freebox4)
