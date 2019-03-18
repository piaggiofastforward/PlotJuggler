
include_directories( ./ ../  ../../include  ../../common ${Qt5WebSockets_INCLUDE_DIRS})

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_WEBSOCKETS)

#find_package(Qt5 REQUIRED COMPONENTS WebSockets )

set(PFFPMC_DIR $ENV{HOME}/motion2/pmc-software)
set(ZMQ_DIR ${PFFPMC_DIR}/third_party/libzmq)

set(CMAKE_CXX_STANDARD 17)

#QT5_WRAP_UI ( UI_SRC  ../common/selectlistdialog.ui  )

include_directories(
        .
        ${PFFPMC_DIR}/include
        ${PFFPMC_DIR}/third_party/cereal/include
        ${ZMQ_DIR}/include
        ${EIGEN3_INCLUDE_DIR}
        json/include
)

SET( SRC
    datastreamserver.cpp datastreamserver.h
    ../../include/PlotJuggler/datastreamer_base.h
    )

link_directories(${ZMQ_DIR}/install/x86_64/lib)

add_library(DataStreamZMQ SHARED ${SRC} ${UI_SRC}  )
#target_link_libraries(DataStreamZMQ  ${Qt5Widgets_LIBRARIES} Qt5::WebSockets Qt5::Xml)
target_link_libraries(DataStreamZMQ  ${Qt5Widgets_LIBRARIES} Qt5::Xml zmq pthread)

