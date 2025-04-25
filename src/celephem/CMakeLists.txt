set(CELEPHEM_SOURCES
  customorbit.cpp
  customorbit.h
  customrotation.cpp
  customrotation.h
  jpleph.cpp
  jpleph.h
  nutation.cpp
  nutation.h
  orbit.cpp
  orbit.h
  precession.cpp
  precession.h
  rotation.cpp
  rotation.h
  sampfile.cpp
  sampfile.h
  samporbit.cpp
  samporbit.h
  samporient.cpp
  samporient.h
  vsop87.cpp
  vsop87.h
)

if(ENABLE_SPICE)
  list(APPEND CELEPHEM_SOURCES
    spiceinterface.cpp
    spiceinterface.h
    spiceorbit.cpp
    spiceorbit.h
    spicerotation.cpp
    spicerotation.h
  )
endif()


if(ENABLE_CELX)
  list(APPEND CELEPHEM_SOURCES
    scriptobject.cpp
    scriptobject.h
    scriptorbit.cpp
    scriptorbit.h
    scriptrotation.cpp
    scriptrotation.h
  )
endif()

# These object files are merged in the celegine library
add_library(celephem OBJECT ${CELEPHEM_SOURCES})

gperf_add_table(celephem customrotation.gperf customrotation.cpp 4)
gperf_add_table(celephem customorbit.gperf    customorbit.cpp    4)

set(ORBIT_PROFILER_SOURCES
  jpleph.cpp
  jpleph.h
  nutation.cpp
  nutation.h
  orbit_profiler.cpp
  orbit.cpp
  orbit.h
  precession.cpp
  precession.h
  rotation.cpp
  rotation.h
  sampfile.cpp
  sampfile.h
  samporbit.cpp
  samporbit.h
  samporient.cpp
  samporient.h
  vsop87.cpp
  vsop87.h
)

add_executable(orbit_profiler ${ORBIT_PROFILER_SOURCES})
target_link_libraries(orbit_profiler
  celutil 
  celastro
  celcompat
  celengine
  celestia
  "-framework Foundation")