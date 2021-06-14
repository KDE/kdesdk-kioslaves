add_definitions(-DTRANSLATION_DOMAIN=\"kio5_perldoc\")

add_library(kio_perldoc MODULE)
set_target_properties(kio_perldoc PROPERTIES
    OUTPUT_NAME "perldoc"
)

target_sources(kio_perldoc PRIVATE
    perldoc.cpp
)

target_link_libraries(kio_perldoc
    KF5::I18n
    KF5::KIOCore
)

install(TARGETS kio_perldoc DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kio)
install(PROGRAMS pod2html.pl DESTINATION ${DATA_INSTALL_DIR}/kio_perldoc)
