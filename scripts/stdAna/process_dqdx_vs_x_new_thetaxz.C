#define DQDXPROCESSOR_CXX
#include "DqDxProcessor.h"

class ProcessorThetaXZ : public DqDxProcessor
{
public:
    ProcessorThetaXZ(const char* fname = "", const char* outpref = "",
		    int batchNo = 20002100, size_t Nruns = 10, size_t startRun = 0,
		    const char* data_version = "v08_34_00") :
	DqDxProcessor(fname, outpref, batchNo, Nruns, startRun, data_version)
    {
	fAllowed.push_back("trkthetaxz_pandoraTrack");
	mHist = 0;//new TH1F("h",";#theta_{XZ}/#pi", 100, -1, 1);
    }

private:
    virtual int SelectEvent(anatree* evt);
    virtual int SelectTrack(anatree* evt, int itrack);
    virtual int SelectHit(anatree* evt, int itrack, int iplane, int ihit);


public:
    TH1F* mHist;
};


void
process_dqdx_vs_x_new_thetaxz(const char* fname = "",
			      const char* outpref = "",
			      int batchNo = 20002100,
			      size_t Nruns = 10, size_t startRun = 0,
			      const char* data_version = "v08_34_00")
{
    auto processor = new ProcessorThetaXZ(fname, outpref, batchNo, Nruns, startRun, data_version);

    auto status = processor->Initialize();
    if (!status) return;

    processor->Process();

    //processor->mHist->Draw();

    processor->Finalize();

}


int ProcessorThetaXZ::SelectEvent(anatree* evt)
{
    return 1;
}

int ProcessorThetaXZ::SelectTrack(anatree* evt, int itrack)
{
    if (evt->trklen_pandoraTrack[itrack] < 200.)
	// track needs to have >= 5 m length
	return false;


    // make theta_xz cut
    float theta_xz = evt->trkthetaxz_pandoraTrack[itrack];
    static const float pi_half = TMath::Pi()*0.5;
    static const float cut = pi_half*0.5;
    if ( abs(theta_xz - pi_half) < cut ||
    	 abs(theta_xz + pi_half) < cut )
    	return false;

    //mHist->Fill(theta_xz/pi_half*0.5);

    return true;
}

int ProcessorThetaXZ::SelectHit(anatree* evt, int itrack, int iplane, int ihit)
{
    return 1;
}
