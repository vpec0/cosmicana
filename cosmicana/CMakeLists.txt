# basic source code CMakeLists.txt
art_make(LIB_LIBRARIES nusimdata_SimulationBase
  MODULE_LIBRARIES larsim_Simulation
  lardataobj_Simulation
  larcoreobj_SummaryData
  larcorealg_Geometry
  larcore_Geometry_Geometry_service
  #nutools_RandomUtils_NuRandomService_service
  larsim_EventGenerator
  nusimdata_SimulationBase
  ${ART_FRAMEWORK_SERVICES_REGISTRY}
  ${ART_ROOT_IO_TFILE_SUPPORT} ${ROOT_CORE}
  ${ART_ROOT_IO_TFILESERVICE_SERVICE}
  art_Persistency_Provenance
  ${MF_MESSAGELOGGER}
  ${CLHEP}
  ${ROOT_HIST}
  ${ROOT_EG}
  ${ROOT_GEOM}
  ${ROOT_PHYSICS}
  ${ROOT_BASIC_LIB_LIST}
  )

install_headers()
install_source()
install_fhicl()
