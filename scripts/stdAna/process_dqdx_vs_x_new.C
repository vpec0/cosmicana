#define DQDXPROCESSOR_CXX
#include "DqDxProcessor.h"

void process_dqdx_vs_x_new(const char* fname = "", const char* outpref = "",
			   int batchNo = 20002100, size_t Nruns = 10, size_t startRun = 0,
			   const char* data_version = "v08_34_00",
			   const char* source = "", const char* topdir = "")
{
    auto processor = new DqDxProcessor(fname, outpref, batchNo, Nruns, startRun, data_version, source, topdir);

    auto status = processor->Initialize();
    if (!status) return;

    processor->Process();

    processor->Finalize();

}
