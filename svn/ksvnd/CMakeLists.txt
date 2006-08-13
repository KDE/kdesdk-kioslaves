
########### next target ###############

set(kded_ksvnd_PART_SRCS ksvnd.cpp )

kde4_automoc(${kded_ksvnd_PART_SRCS})

kde4_add_ui3_files(kded_ksvnd_PART_SRCS commitdlg.ui )

#kde4_add_dcop_skels(kded_ksvnd_PART_SRCS ksvnd.h )

kde4_add_plugin(kded_ksvnd ${kded_ksvnd_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kded_ksvnd )

target_link_libraries(kded_ksvnd  ${KDE4_KIO_LIBS} )

install(TARGETS kded_ksvnd  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ksvnd.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )


