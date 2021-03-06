
project(libyzis)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/translator.h.in ${CMAKE_CURRENT_BINARY_DIR}/translator.h)
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/translator.h )

include_directories(${YZIS_INCLUDES})

########### next target ###############

set(yzis_LIB_SRCS 
   keys.cpp
   mode_command.cpp 
   action.cpp 
   buffer.cpp 
   color.cpp 
   cursor.cpp 
   debug.cpp 
   drawbuffer.cpp 
   drawbufferiterators.cpp 
   drawcell.cpp 
   drawline.cpp 
   events.cpp 
   folding.cpp 
   font.cpp 
   history.cpp 
   internal_options.cpp 
   line.cpp 
   linesearch.cpp 
   luaengine.cpp 
   luafuncs.cpp 
   luaregexp.cpp 
   mapping.cpp 
   mark.cpp 
   mode_complete.cpp 
   mode.cpp 
   mode_ex.cpp 
   mode_insert.cpp 
   mode_pool.cpp 
   mode_search.cpp 
   mode_visual.cpp 
   option.cpp 
   registers.cpp 
   resourcemgr.cpp 
   search.cpp 
   selection.cpp 
   session.cpp 
   swapfile.cpp 
   tags_interface.cpp 
   tags_stack.cpp 
   undo.cpp 
   view.cpp 
   viewcursor.cpp 
   yzisinfo.cpp 
   yzisinfojumplistrecord.cpp 
   yzisinfostartpositionrecord.cpp 

   readtags/readtags.c 

   kate/attribute.cpp 
   kate/schema.cpp 
   kate/syntaxdocument.cpp 
   kate/syntaxhighlight.cpp 
)

qt5_wrap_cpp(yzis_LIB_MOC_SRCS ${yzis_LIB_MOCS})

add_library(yzis SHARED ${yzis_LIB_SRCS} ${yzis_LIB_MOC_SRCS} )

target_link_libraries(yzis Qt5::Core Qt5::Xml ${LUA_LIBRARIES} ${GETTEXT_LIBRARIES})

set_target_properties(yzis PROPERTIES VERSION 0.0.0 SOVERSION 0 )

if (WIN32)
    install(TARGETS yzis 
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            )
else (WIN32)
    install(TARGETS yzis 
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            )
endif (WIN32)

add_doc(libyzis)
