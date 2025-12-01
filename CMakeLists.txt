cmake_minimum_required(VERSION 3.10)
project(RideSharingBackend VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)
include_directories(third_party/Crow/include)
include_directories(third_party/asio/asio/include)

# Find required packages
find_package(SQLite3 REQUIRED)

# Source files
set(SOURCES
    src/service/RiderService.cpp
    src/service/DriverService.cpp
    src/service/RideRequestService.cpp
    src/service/TripService.cpp
    src/service/MatchingService.cpp
    src/repository/RiderRepository.cpp
    src/repository/DriverRepository.cpp
    src/repository/RideRequestRepository.cpp
    src/repository/TripRepository.cpp
    src/model/Rider.cpp
    src/model/Driver.cpp
    src/model/RideRequest.cpp
    src/model/Trip.cpp
    src/utils/Database.cpp
    src/utils/Logger.cpp
    src/utils/Config.cpp
    src/main.cpp
)

# Executable
add_executable(ride_sharing_backend ${SOURCES})

# Link libraries
target_link_libraries(ride_sharing_backend
    sqlite3
    pthread
)



# Install rules
install(TARGETS ride_sharing_backend DESTINATION bin)
install(FILES config/config.json DESTINATION config)
