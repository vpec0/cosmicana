#define DQDXPROCESSOR_CXX
#include "DqDxProcessor.h"

class ProcessorAPACPA : public DqDxProcessor
{
public:
    ProcessorAPACPA(const char* fname = "", const char* outpref = "",
		    int batchNo = 20002100, size_t Nruns = 10, size_t startRun = 0,
		    const char* data_version = "v08_34_00") :
	DqDxProcessor(fname, outpref, batchNo, Nruns, startRun, data_version)
    {
	fAllowed.push_back("trkstartx_pandoraTrack");
	fAllowed.push_back("trkendx_pandoraTrack");
    }

private:
    virtual int SelectEvent(anatree* evt);
    virtual int SelectTrack(anatree* evt, int itrack);
    virtual int SelectHit(anatree* evt, int itrack, int iplane, int ihit);


    TH1F* mHist;
};


void
process_dqdx_vs_x_new_APACPA(const char* fname = "",
			     const char* outpref = "",
			     int batchNo = 20002100,
			     size_t Nruns = 10, size_t startRun = 0,
			     const char* data_version = "v08_34_00")
{
    auto processor = new ProcessorAPACPA(fname, outpref, batchNo, Nruns, startRun, data_version);

    auto status = processor->Initialize();
    if (!status) return;

    processor->Process();

    processor->Finalize();

}


int ProcessorAPACPA::SelectEvent(anatree* evt)
{
    return 1;
}

int ProcessorAPACPA::SelectTrack(anatree* evt, int itrack)
{
    if (evt->trklen_pandoraTrack[itrack] < 200.)
	// track needs to have >= 5 m length
	return false;


    return true;
}

int ProcessorAPACPA::SelectHit(anatree* evt, int itrack, int iplane, int ihit)
{
#ifdef DEBUG
    static int event = 0;
    static int printed_track = -1;
    if (event != evt->event) {
    	event = evt->event;
    	printed_track = -1;
    }
#endif

    float x = evt->trkxyz_pandoraTrack[itrack][iplane][ihit][0];
    float start_x = evt->trkstartx_pandoraTrack[itrack];
    float end_x = evt->trkendx_pandoraTrack[itrack];

    int tpc = whichTPC(x)+1; // 0 = lowest X, 3 = highest X

    if (tpc < 1 || tpc > 4)
	return 0; // outside the active volume

    // test if track within 10 cm from the TPC boundary
    // get distances of track start and end from APA and CPA
    float dx_apa_start = ( -1 + 2*(tpc%2) )*(start_x - APA_X_POSITIONS[tpc/2]);
    float dx_apa_end = ( -1 + 2*(tpc%2) )*(end_x - APA_X_POSITIONS[tpc/2]);
    float dx_cpa_start = ( -1 + 2*((tpc-1)%2) )*(start_x - CPA_X_POSITIONS[(tpc-1)/2]);
    float dx_cpa_end = ( -1 + 2*((tpc-1)%2) )*(end_x - CPA_X_POSITIONS[(tpc-1)/2]);

    if ( (dx_apa_start > 10. || dx_cpa_end > 10.) &&
	 (dx_cpa_start > 10. || dx_apa_end > 10.) )
	return 0;

#ifdef DEBUG
    if (itrack != printed_track) {
    	cout<<Form("T:%dP:%dH:%dX:%.1fTPC:%d, ", itrack, iplane, ihit, x, tpc)<<endl;
    	printed_track = itrack;
    }
#endif // DEBUG

    return 1;
}
