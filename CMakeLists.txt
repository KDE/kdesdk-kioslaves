cmake_minimum_required(VERSION 3.0)
project(kioslave)

set(QT_MIN_VERSION 5.5.0)
set(KF5_MIN_VERSION 5.27.0)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE
    COMPONENTS
    Core
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED
    COMPONENTS
    I18n
    KIO
)

find_package(Perl 5.10)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050e00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054700)

if(PERL_FOUND)
    add_subdirectory(perldoc)
endif(PERL_FOUND)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
