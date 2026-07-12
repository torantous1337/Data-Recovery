cmake_minimum_required(VERSION 3.24)

include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(spdlog nlohmann_json)

find_package(SQLite3 REQUIRED)

set(RECOVERY_ENGINE_HEADERS
    include/RecoveryEngine/Core/Types.h
    include/RecoveryEngine/IO/IDiskAccess.h
    include/RecoveryEngine/Parsers/IFileSystemParser.h
    include/RecoveryEngine/Parsers/ParserFactory.h
    include/RecoveryEngine/Carving/SignatureDatabase.h
    include/RecoveryEngine/Carving/AhoCorasick.h
    include/RecoveryEngine/Carving/CarvingEngine.h
    include/RecoveryEngine/Recovery/RecoveryManager.h
    include/RecoveryEngine/Persistence/SessionStore.h
    include/RecoveryEngine/Imaging/ImagingEngine.h
    include/RecoveryEngine/Raid/RaidReconstructor.h
    include/RecoveryEngine/Integrations/VssProvider.h
    include/RecoveryEngine/Integrations/BitLockerAdapter.h
    include/RecoveryEngine/Interop/RecoveryEngineC.h
)

set(RECOVERY_ENGINE_SOURCES
    src/IO/PhysicalDriveAccess.cpp
    src/IO/RawImageAccess.cpp
    src/Parsers/NtfsParser.cpp
    src/Parsers/Fat32Parser.cpp
    src/Parsers/ExFatParser.cpp
    src/Parsers/RefsParser.cpp
    src/Parsers/ParserFactory.cpp
    src/Carving/AhoCorasick.cpp
    src/Carving/SignatureDatabase.cpp
    src/Carving/CarvingEngine.cpp
    src/Recovery/RecoveryManager.cpp
    src/Persistence/SessionStore.cpp
    src/Imaging/ImagingEngine.cpp
    src/Raid/RaidReconstructor.cpp
    src/Integrations/VssProvider.cpp
    src/Integrations/BitLockerAdapter.cpp
    src/Interop/RecoveryEngineC.cpp
)

add_library(RecoveryEngine SHARED ${RECOVERY_ENGINE_HEADERS} ${RECOVERY_ENGINE_SOURCES})

target_include_directories(RecoveryEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(RecoveryEngine
    PUBLIC
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        SQLite::SQLite3
)

if (WIN32)
    target_link_libraries(RecoveryEngine PUBLIC Ole32 VssApi)
endif()
