########## USE until module is ready ################################
#
# This module finds if wxWindows is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#
#  WXWINDOWS_LINK_DIRECTORIES = link directories, useful for rpath on Unix
#  WXWINDOWS_LIBRARIES       = all to the wxWindows libraries ( and  linker flags on unix )
#  CMAKE_WX_CXX_FLAGS        = compiler flags for building wxWindows
#  WXWINDOWS_INCLUDE_PATH    = include path of wxWindows
#  WXWINDOWS_INCLUDE_DIR    = all include path of wxWindows and wxArt2D
#  WXWINDOWS_DEFINITIONS  = all flags of wxWindows and wxArt2D

IF( WIN32 )

     FIND_PATH( WXWINDOWS_INSTALL_PATH
        wx-config.in
        $ENV{WXWIN}
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\wxWindows_is1;Inno Setup: App Path]"
    )

    IF ( EXISTS ${WXWINDOWS_INSTALL_PATH}/lib/vc_lib )
        SET( WXVERSION "2.51_OR_HIGHER" )
    ELSE( EXISTS ${WXWINDOWS_INSTALL_PATH}/lib/vc_lib )
        SET( WXVERSION "2.42_OR_LOWER" )
    ENDIF ( EXISTS ${WXWINDOWS_INSTALL_PATH}/lib/vc_lib )

    MESSAGE( STATUS "wxWidget install path: ${WXWINDOWS_INSTALL_PATH} version:  ${WXVERSION}" )

    OPTION( WXWINDOWS_USE_DEBUG  "Use Debug versions of wxWindows libraries" ON)

    OPTION( WXWINDOWS_USE_UNICODE "Use Unicode versions of wxWindows libraries" OFF)

    IF ( WXVERSION MATCHES "2.51_OR_HIGHER" )

            OPTION( WXWINDOWS_USE_SHARED "Use shared versions (.so) of wxWindows libraries" OFF)

            OPTION( WXWINDOWS_USE_UNIV "Use Universal versions (.so) of wxWindows libraries" OFF)

            IF( WXWINDOWS_USE_SHARED )
                SET(WXWINDOWS_LIB_PATH "${WXWINDOWS_INSTALL_PATH}/lib/vc_dll" )
            ELSE( WXWINDOWS_USE_SHARED )
                SET(WXWINDOWS_LIB_PATH "${WXWINDOWS_INSTALL_PATH}/lib/vc_lib" )
            ENDIF( WXWINDOWS_USE_SHARED )
            SET( WXWINDOWS_LINK_DIRECTORIES ${WXWINDOWS_LIB_PATH} )

            # find all  flavors of wxWidget libraries based on the name they get by default.
            # If the users decides to use his own naming, he must choose himself
            # the eventual library to use.

            # set flags detected for library types

            # UNIV versions
            IF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswuniv/wx/setup.h )
                SET( RELEASE_AVAILABLE TRUE )
                SET( UNIV_AVAILABLE TRUE )
            ENDIF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswuniv/wx/setup.h )

            IF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswunivd/wx/setup.h )
                SET( DEBUG_AVAILABLE TRUE )
                SET( UNIV_AVAILABLE TRUE )
            ENDIF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswunivd/wx/setup.h )

            IF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswunivu/wx/setup.h )
                SET( RELEASE_AVAILABLE TRUE )
                SET( UNIV_AVAILABLE TRUE )
                SET( UNICODE_AVAILABLE TRUE )
            ENDIF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswunivu/wx/setup.h )

            IF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswunivud/wx/setup.h )
                SET( DEBUG_AVAILABLE TRUE )
                SET( UNIV_AVAILABLE TRUE )
                SET( UNICODE_AVAILABLE TRUE )
            ENDIF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswunivud/wx/setup.h )

            # Non universal versions
            IF ( EXISTS ${WXWINDOWS_LIB_PATH}/msw/wx/setup.h )
                SET( RELEASE_AVAILABLE TRUE )
            ENDIF ( EXISTS ${WXWINDOWS_LIB_PATH}/msw/wx/setup.h )

            IF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswd/wx/setup.h )
                SET( DEBUG_AVAILABLE TRUE )
            ENDIF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswd/wx/setup.h )

            IF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswu/wx/setup.h )
                SET( RELEASE_AVAILABLE TRUE )
                SET( UNICODE_AVAILABLE TRUE )
            ENDIF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswu/wx/setup.h )

            IF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswud/wx/setup.h )
                SET( DEBUG_AVAILABLE TRUE )
                SET( UNICODE_AVAILABLE TRUE )
            ENDIF ( EXISTS ${WXWINDOWS_LIB_PATH}/mswud/wx/setup.h )

            SET( LIBRARYWANT  "${WXWINDOWS_LIB_PATH}/msw" )

            IF ( WXWINDOWS_USE_UNIV )
                IF ( NOT  UNIV_AVAILABLE )
                    MESSAGE( SEND_ERROR "universal library version not available")
                ELSE ( NOT  UNIV_AVAILABLE )
                    SET( LIBRARYWANT "${LIBRARYWANT}univ" CACHE INTERNAL "aap")
                    SET( PFUNIV  "univ" )
                ENDIF ( NOT  UNIV_AVAILABLE )
            ENDIF ( WXWINDOWS_USE_UNIV )

            SET( WXPF "${WXPF}25" )
            SET( PFVERSION "25" )
            SET( PFCOMPILER "" )
            SET( PFLIBEXT "lib" )

            IF ( WXWINDOWS_USE_UNICODE )
                IF ( NOT  UNICODE_AVAILABLE )
                    MESSAGE( SEND_ERROR "unicode library version not available")
                ELSE ( NOT  UNICODE_AVAILABLE )
                    SET( LIBRARYWANT "${LIBRARYWANT}u" )
                    SET( PFUNICODE "u" )
                ENDIF ( NOT  UNICODE_AVAILABLE )
            ENDIF ( WXWINDOWS_USE_UNICODE )

            # check option against found libraries
            IF ( WXWINDOWS_USE_DEBUG )
                IF ( NOT DEBUG_AVAILABLE )
                    MESSAGE( SEND_ERROR "debug library version not available")
                ELSE ( NOT  DEBUG_AVAILABLE )
                    SET( LIBRARYWANT "${LIBRARYWANT}d" )
                    SET( PFDEBUG "d" )
                ENDIF ( NOT  DEBUG_AVAILABLE )
            ENDIF ( WXWINDOWS_USE_DEBUG )

            #MESSAGE( "${LIBRARYWANT}")

            #find include directories
            SET ( WXWINDOWS_INCLUDE_PATH
                ${WXWINDOWS_INSTALL_PATH}/include
            )
            FIND_PATH(WXWINDOWS_INCLUDE_DIR  wx/wx.h
                ${WXWINDOWS_INCLUDE_PATH}
            )

            IF(WXWINDOWS_INCLUDE_DIR)
                SET( WXWINDOWS_INCLUDE_DIR ${WXWINDOWS_INCLUDE_DIR} ${WXWINDOWS_INCLUDE_DIR}/../contrib/include )
            ENDIF(WXWINDOWS_INCLUDE_DIR)

            # append the build specific include dir for wx/setup.h:
            IF ( EXISTS ${LIBRARYWANT}/wx/setup.h )
                SET( WXWINDOWS_INCLUDE_DIR  ${WXWINDOWS_INCLUDE_DIR}  ${LIBRARYWANT} )
            ENDIF ( EXISTS ${LIBRARYWANT}/wx/setup.h )

            SET( WXWINDOWS_LIBRARIES "" )
            SET(CMAKE_WX_LIBRARIES ${CMAKE_WX_LIBRARIES} comctl32 wsock32 rpcrt4)

            #start filling library string with needed libraries for the choosen configuration.
            SET( WXWINDOWS_LIBRARIES ${CMAKE_WX_LIBRARIES} )

            SET( WXPF "${PFVERSION}${PFUNICODE}${PFDEBUG}" )
            SET(WXWINDOWS_LIBRARIES ${WXWINDOWS_LIBRARIES}
                wxbase${WXPF}${PFCOMPILER}.${PFLIBEXT}
                wxbase${WXPF}_net${PFCOMPILER}.${PFLIBEXT}  wxbase${WXPF}_xml${PFCOMPILER}.${PFLIBEXT}
            )

            SET( WXPF "${PFVERSION}${PFUNICODE}${PFDEBUG}" )
            SET(WXWINDOWS_LIBRARIES ${WXWINDOWS_LIBRARIES}
                wxmsw${WXPF}_adv${PFCOMPILER}.${PFLIBEXT}      wxmsw${WXPF}_core${PFCOMPILER}.${PFLIBEXT}
                wxmsw${WXPF}_html${PFCOMPILER}.${PFLIBEXT}
            )

            IF( NOT WXWINDOWS_USE_SHARED )
                SET( WXPF "${PFVERSION}${PFUNICODE}${PFDEBUG}" )
                # ODBC  is not needed in wxArt2D
                #SET(WXWINDOWS_LIBRARIES ${WXWINDOWS_LIBRARIES} wxbase${WXPF}_odbc${PFCOMPILER}.${PFLIBEXT} )

                SET( WXPF "${PFVERSION}${PFUNICODE}${PFDEBUG}" )
                SET(WXWINDOWS_LIBRARIES ${WXWINDOWS_LIBRARIES}
                    wxmsw${WXPF}_dbgrid.${PFLIBEXT}
                    wxmsw${WXPF}_gl${PFCOMPILER}.${PFLIBEXT}
                )
            ENDIF( NOT WXWINDOWS_USE_SHARED )

            IF ( WXWINDOWS_USE_UNIV )
                SET( WXPF "${PFUNIV}${PFVERSION}${PFUNICODE}${PFDEBUG}" )
                SET(WXWINDOWS_LIBRARIES ${WXWINDOWS_LIBRARIES}
                    wxmsw${WXPF}_core${PFCOMPILER}.${PFLIBEXT}
                )
            ENDIF ( WXWINDOWS_USE_UNIV )

            SET( WXPF "${PFUNICODE}${PFDEBUG}" )
            SET(WXWINDOWS_LIBRARIES ${WXWINDOWS_LIBRARIES}
                wxregex${WXPF}.lib
            )

            SET( WXPF "${PFDEBUG}" )
            SET(WXWINDOWS_LIBRARIES ${WXWINDOWS_LIBRARIES}
                #skip the next since it is detected seperately
                #wxexpat${WXPF}.lib
                wxpng${WXPF}.lib
                wxtiff${WXPF}.lib
                wxjpeg${WXPF}.lib
                wxzlib${WXPF}.lib
            )

    # The NEXT part will go away in the future SO do NOT optimize with the above
    ELSE ( WXVERSION MATCHES "2.51_OR_HIGHER" )

            SET ( WXWINDOWS_POSSIBLE_LIB_PATHS
                ${WXWINDOWS_INSTALL_PATH}/lib
            )

            FIND_LIBRARY( WXWINDOWS_STATIC_LIBRARY
                NAMES wxmswd
                PATHS ${WXWINDOWS_POSSIBLE_LIB_PATHS}
            )

            FIND_LIBRARY( WXWINDOWS_SHARED_LIBRARY
                NAMES wx24_0
                PATHS ${WXWINDOWS_POSSIBLE_LIB_PATHS}
            )

            SET ( WXWINDOWS_POSSIBLE_INCLUDE_PATHS
                ${WXWINDOWS_INSTALL_PATH}/include
            )

            FIND_PATH( WXWINDOWS_INCLUDE_PATH
                wx/wx.h
                ${WXWINDOWS_POSSIBLE_INCLUDE_PATHS}
            )

            IF( WXWINDOWS_SHARED_LIBRARY )
                OPTION(WXWINDOWS_USE_SHARED_LIBS
                       "Use shared versions of wxWindows libraries" ON)
                MARK_AS_ADVANCED(WXWINDOWS_USE_SHARED_LIBS)
            ENDIF( WXWINDOWS_SHARED_LIBRARY )

            SET(CMAKE_WX_LIBRARIES ${CMAKE_WX_LIBRARIES} comctl32 wsock32 rpcrt4)

            SET( WXWINDOWS_LINK_DIRECTORIES ${WXWINDOWS_INSTALL_PATH}/lib )

            IF( WXWINDOWS_USE_SHARED_LIBS )
                SET(WXWINDOWS_LIBRARIES ${WXWINDOWS_SHARED_LIBRARY} ${CMAKE_WX_LIBRARIES} )
            ELSE( WXWINDOWS_USE_SHARED_LIBS )
                SET(WXWINDOWS_LIBRARIES ${WXWINDOWS_STATIC_LIBRARY} regexd.lib pngd.lib zlibd.lib jpegd.lib tiffd.lib ${CMAKE_WX_LIBRARIES})
            ENDIF( WXWINDOWS_USE_SHARED_LIBS )

            MARK_AS_ADVANCED(
        #       WXWINDOWS_STATIC_LIBRARY
                WXWINDOWS_SHARED_LIBRARY
                WXWINDOWS_INCLUDE_PATH
            )

            #these or at least many of them or uses by wxWindows.
            IF( WXWINDOWS_USE_DEBUG )
                IF ( WXWINDOWS_USE_UNICODE )
                    #the setup.h is placed under $wxinstall/lib/mswud
                    SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -DWIN32;-D__WIN32__;-D_WINDOWS;-DWINVER=0x0400;-D__WXMSW__;/D__WINDOWS__;-D__WIN95__;-DSTRICT;/D__WXDEBUG__;/DwxUSE_UNICODE)
                ELSE ( WXWINDOWS_USE_UNICODE )
                    #the setup.h is placed under $wxinstall/lib/mswd
                    SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -DWIN32;-D__WIN32__;-D_WINDOWS;-DWINVER=0x0400;-D__WXMSW__;/D__WINDOWS__;-D__WIN95__;-DSTRICT;/D__WXDEBUG__)
                ENDIF ( WXWINDOWS_USE_UNICODE )
            ELSE ( WXWINDOWS_USE_DEBUG )
                IF ( WXWINDOWS_USE_UNICODE )
                    SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -DWIN32;-D__WIN32__;-D_WINDOWS;-DWINVER=0x0400;-D__WXMSW__;/D__WINDOWS__;-D__WIN95__;-DSTRICT;/DwxUSE_UNICODE)
                ELSE ( WXWINDOWS_USE_UNICODE )
                    SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -DWIN32;-D__WIN32__;-D_WINDOWS;-DWINVER=0x0400;-D__WXMSW__;/D__WINDOWS__;-D__WIN95__;-DSTRICT)
                ENDIF ( WXWINDOWS_USE_UNICODE )

            ENDIF( WXWINDOWS_USE_DEBUG )

            IF( WXWINDOWS_USE_SHARED_LIBS )

                IF(WXWINDOWS_INCLUDE_PATH)
                    SET( WXWINDOWS_INCLUDE_DIR  ${WXWINDOWS_INCLUDE_PATH}  ${WXWINDOWS_INCLUDE_PATH}/../contrib/include )
                    IF( WXWINDOWS_USE_DEBUG )
                        IF ( WXWINDOWS_USE_UNICODE )
                            #the setup.h is placed under $wxinstall/lib/mswud
                            SET( WXWINDOWS_INCLUDE_DIR  ${WXWINDOWS_INCLUDE_DIR}  ${WXWINDOWS_INSTALL_PATH}/lib/mswdllud )
                        ELSE ( WXWINDOWS_USE_UNICODE )
                            #the setup.h is placed under $wxinstall/lib/mswd
                            SET( WXWINDOWS_INCLUDE_DIR  ${WXWINDOWS_INCLUDE_DIR}  ${WXWINDOWS_INSTALL_PATH}/lib/mswdlld )
                        ENDIF ( WXWINDOWS_USE_UNICODE )
                    ELSE( WXWINDOWS_USE_DEBUG )
                        IF ( WXWINDOWS_USE_UNICODE )
                            #the setup.h is placed under $wxinstall/lib/msw
                            SET( WXWINDOWS_INCLUDE_DIR ${WXWINDOWS_INCLUDE_DIR}  ${WXWINDOWS_INSTALL_PATH}/lib/mswdllu )
                        ELSE ( WXWINDOWS_USE_UNICODE )
                            #the setup.h is placed under $wxinstall/lib/mswu
                            SET( WXWINDOWS_INCLUDE_DIR ${WXWINDOWS_INCLUDE_DIR}  ${WXWINDOWS_INSTALL_PATH}/lib/mswdll )
                        ENDIF ( WXWINDOWS_USE_UNICODE )
                    ENDIF( WXWINDOWS_USE_DEBUG )
                ENDIF(WXWINDOWS_INCLUDE_PATH)

            ELSE( WXWINDOWS_USE_SHARED_LIBS )

                IF(WXWINDOWS_INCLUDE_PATH)
                    SET( WXWINDOWS_INCLUDE_DIR  ${WXWINDOWS_INCLUDE_PATH}  ${WXWINDOWS_INCLUDE_PATH}/../contrib/include )
                    IF( WXWINDOWS_USE_DEBUG )
                        IF ( WXWINDOWS_USE_UNICODE )
                            #the setup.h is placed under $wxinstall/lib/mswud
                            SET( WXWINDOWS_INCLUDE_DIR  ${WXWINDOWS_INCLUDE_DIR}  ${WXWINDOWS_INSTALL_PATH}/lib/mswud )
                        ELSE ( WXWINDOWS_USE_UNICODE )
                            #the setup.h is placed under $wxinstall/lib/mswd
                            SET( WXWINDOWS_INCLUDE_DIR  ${WXWINDOWS_INCLUDE_DIR}  ${WXWINDOWS_INSTALL_PATH}/lib/mswd )
                        ENDIF ( WXWINDOWS_USE_UNICODE )
                    ELSE( WXWINDOWS_USE_DEBUG )
                        IF ( WXWINDOWS_USE_UNICODE )
                            #the setup.h is placed under $wxinstall/lib/msw
                            SET( WXWINDOWS_INCLUDE_DIR ${WXWINDOWS_INCLUDE_DIR}  ${WXWINDOWS_INSTALL_PATH}/lib/mswu )
                        ELSE ( WXWINDOWS_USE_UNICODE )
                            #the setup.h is placed under $wxinstall/lib/mswu
                            SET( WXWINDOWS_INCLUDE_DIR ${WXWINDOWS_INCLUDE_DIR}  ${WXWINDOWS_INSTALL_PATH}/lib/msw )
                        ENDIF ( WXWINDOWS_USE_UNICODE )
                    ENDIF( WXWINDOWS_USE_DEBUG )
                ENDIF(WXWINDOWS_INCLUDE_PATH)

            ENDIF( WXWINDOWS_USE_SHARED_LIBS )


    ENDIF ( WXVERSION MATCHES "2.51_OR_HIGHER" )


    #MESSAGE( "${WXWINDOWS_LIBRARIES}")

    SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -DWIN32 -D__WXMSW__  )

    IF( WXWINDOWS_USE_SHARED )
        SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -DWXUSINGDLL  )
    ENDIF( WXWINDOWS_USE_SHARED )

    #these or at least many of them or uses by wxWindows.
    IF( WXWINDOWS_USE_DEBUG )
        SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -D_DEBUG_  -D__WXDEBUG__  )
    ENDIF( WXWINDOWS_USE_DEBUG )

    IF ( WXWINDOWS_USE_UNICODE )
        SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -DwxUSE_UNICODE  )
    ENDIF ( WXWINDOWS_USE_UNICODE )

    SET( WXWINDOWS_RC ${WXWINDOWS_INSTALL_PATH}/include/wx/msw/wx.rc )
    # used in ADD_EXECUTABLE is the WIN32GUI to tell it is a typical windows GUI executable.
    SET( WIN32GUI WIN32 )

    # not used here
    SET( CMAKE_WX_CXX_FLAGS "" )

    MARK_AS_ADVANCED(
        WXWINDOWS_INCLUDE_PATH
        WXWINDOWS_INSTALL_PATH
    )

ELSE( WIN32 )

    FIND_PROGRAM( CMAKE_WX_CONFIG wx-config ../wx/bin ../../wx/bin )
    SET( CMAKE_WX_CXX_FLAGS "`${CMAKE_WX_CONFIG} --cxxflags`" )
    SET( WXWINDOWS_LIBRARIES "`${CMAKE_WX_CONFIG} --libs`" )

    ## extract linkdirs (-L) for rpath
    ## use regular expression to match wildcard equivalent "-L*<endchar>"
    ## with <endchar> is a space or a semicolon
    STRING(REGEX MATCHALL "[-][L]([^ ;])+" WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX "${WX_CONFIG_LIBS}" )
    #MESSAGE("DBG  WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX=${WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX}")

    ## remove prefix -L because we need the pure directory for LINK_DIRECTORIES
    ## replace -L by ; because the separator seems to be lost otherwise (bug or
    ## feature?)
    IF(WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX)
      STRING(REGEX REPLACE "[-][L]" ";" WXWINDOWS_LINK_DIRECTORIES ${WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX} )
      #MESSAGE("DBG  WXWINDOWS_LINK_DIRECTORIES=${WXWINDOWS_LINK_DIRECTORIES}")
    ENDIF(WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX)

    ## replace space separated string by semicolon separated vector to make it
    ## work with LINK_DIRECTORIES
    SEPARATE_ARGUMENTS(WXWINDOWS_LINK_DIRECTORIES)

    IF( CYGWIN OR MINGW )
        GET_FILENAME_COMPONENT(WXWINDOWS_INSTALL_PATH ${CMAKE_WX_CONFIG} PATH)
        SET( WXWINDOWS_INSTALL_PATH ${WXWINDOWS_INSTALL_PATH}/.. )
        SET( WXWINDOWS_RC ${WXWINDOWS_INSTALL_PATH}/include/wx/msw/wx.rc )
    ELSE( CYGWIN OR MINGW )
        SET( WXWINDOWS_RC "" )
    ENDIF( CYGWIN OR MINGW )
    SET( WIN32GUI "" ) 
    
    #keep debug settings as indicated by wx-config
    #SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} "${CMAKE_WX_CXX_FLAGS}" )
    #IF( WXWINDOWS_USE_DEBUG )
    #    SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -ggdb )
    #ENDIF( WXWINDOWS_USE_DEBUG )

    IF( CYGWIN OR MINGW )
        IF( CYGWIN )
            SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -D__GNUWIN32__  -O2 -D_WIN32_IE=0x400 -MMD -Wall  )
        ENDIF( CYGWIN )
    ELSE( CYGWIN OR MINGW )
        # just in case `gtk-config --cflags`does not work
        # SET( WXWINDOWS_INCLUDE_DIR ${WXWINDOWS_INCLUDE_DIR}  /usr/include/gtk-2.0 /usr/include/gtk-2.0/include /usr/lib/gtk-2.0/include /usr/include/glib-2.0 /usr/lib/glib-2.0/include /usr/include/pango-1.0 /usr/include/atk-1.0 )
        # What about FindGTK.cmake? and what if somebody uses wxMotif?
        SET( CMAKE_WX_CXX_FLAGS "${CMAKE_WX_CXX_FLAGS} `gtk-config --cflags`" )
        #find Xwindows
        INCLUDE( ${CMAKE_ROOT}/Modules/FindX11.cmake )
        SET( WXWINDOWS_INCLUDE_DIR ${WXWINDOWS_INCLUDE_DIR}  ${X11_INCLUDE_DIR} )
    ENDIF( CYGWIN OR MINGW )

ENDIF( WIN32 )

MARK_AS_ADVANCED(
    CMAKE_WX_CXX_FLAGS
)

#MESSAGE( "${WXWINDOWS_LINK_DIRECTORIES}" )
#MESSAGE( "${WXWINDOWS_LIBRARIES}" )
#MESSAGE( "CMAKE_WX_CXX_FLAGS}" )
#MESSAGE( "WXWINDOWS_INCLUDE_PATH}" )
#MESSAGE( "WXWINDOWS_INCLUDE_DIR}" )
#MESSAGE( "WXWINDOWS_DEFINITIONS}" )
