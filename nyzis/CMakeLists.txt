
project(nyzis)

option(ENABLE_X_IN_NYZIS "with this option nyzis links to X and can access the X copy/paste buffer" OFF)

include_directories(${YZIS_INCLUDES} ${NCURSES_INCLUDE_DIR})

########### next target ###############
set(nyzis_SRCS
   main.cpp
   nysession.cpp
   nyview.cpp
   nystatusbar.cpp
)

add_executable(nyzis ${nyzis_SRCS})
target_link_libraries(nyzis ${NCURSES_LIBRARIES} Qt5::Core yzis )

if (ENABLE_X_IN_NYZIS)
    target_link_libraries(nyzis ${NCURSES_LIBRARIES} ${X11_LIBRARIES} Qt5::Core yzis )
else (ENABLE_X_IN_NYZIS)
    target_link_libraries(nyzis ${NCURSES_LIBRARIES} Qt5::Core yzis )
endif (ENABLE_X_IN_NYZIS)

install(TARGETS nyzis DESTINATION bin)

add_doc(nyzis libyzis)
