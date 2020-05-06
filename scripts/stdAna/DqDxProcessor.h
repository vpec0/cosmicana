#ifndef DQDXPROCESSOR_H
#define DQDXPROCESSOR_H

#include "FileHandler.h"
#include "HistHandler.h"

#include "anatree.h"

class DqDxProcessor
{
public:
    DqDxProcessor(const char* fname = "", const char* outpref = "",
		  int batchNo = 20002100, size_t Nruns = 10, size_t startRun = 0,
		  const char* data_version = "v08_34_00");
    ~DqDxProcessor();

    int Initialize();
    int Process();
    int Finalize();

private:
    virtual int SelectEvent(anatree* evt);
    virtual int SelectTrack(anatree* evt, int itrack);

private:
    int whichTPC(double x);

private:
    const double APA_X_POSITIONS[3] = {-726.7681, 0., 726.7681};
    const double CPA_X_POSITIONS[2] = {-363.38405, 363.38405};
    const double TPC_Z_SIZE = 232.39;
    const double kXtoT = 1./160.563; // converts cm to ms, calculated for field 0.5kV/cm and temperature 87K

private:
    TChain* fTree;
    anatree* fEvent;

    size_t fSize;

    TString fIFName;
    TString fOutPref;

    TFile* fOutFile;

    size_t fBatchNo;
    size_t fNruns;
    size_t fStartRun;

    TString fDataVersion;

    map<TString, vector<TH1*>> fMVHists;

    FileHandler fFHandler;
    HistHandler fHHandler;

    enum {
	Dqdx_vs_x = 0,
	Dqdx_vs_x_plane0,
	Dqdx_vs_x_plane1,
	Dqdx_vs_x_plane2,
	Dqdx_vs_t,
	Dqdx_vs_t_plane0,
	Dqdx_vs_t_plane1,
	Dqdx_vs_t_plane2,
	NHists
    };

    enum {
	Dedx_vs_x = 0,
	Dedx_vs_x_plane0,
	Dedx_vs_x_plane1,
	Dedx_vs_x_plane2,
	Dedx_vs_t,
	Dedx_vs_t_plane0,
	Dedx_vs_t_plane1,
	Dedx_vs_t_plane2,
	NHists_dedx
    };


};

#endif

#ifdef DQDXPROCESSOR_CXX

DqDxProcessor::DqDxProcessor(const char* fname = "", const char* outpref = "",
			     int batchNo = 20002100, size_t Nruns = 10, size_t startRun = 0,
			     const char* data_version = "v08_34_00"):
    fTree(0),
    fEvent(0),
    fSize(0),
    fIFName(fname),
    fOutPref(outpref),
    fOutFile(0),
    fBatchNo(batchNo),
    fNruns(Nruns),
    fStartRun(startRun),
    fDataVersion(data_version)
{
}


int DqDxProcessor::Initialize()
{
    /// Histograms

    TH1* hists[NHists] = {};
#define H1(name, title, nbins, low, high) hists[name] = new TH1F(#name, title, nbins, low, high)
#define H2(name, title, nbinsx, lowx, highx, nbinsy, lowy, highy) hists[name] = new TH2F(#name, title, nbinsx, lowx, highx, nbinsy, lowy, highy)

    H2(Dqdx_vs_x, "dQ/dx vs x;x [cm]", 400, -800, 800, 400, 0, 800);
    H2(Dqdx_vs_x_plane0, "dQ/dx vs x plane 0;x [cm]", 400, -800, 800, 400, 0, 800);
    H2(Dqdx_vs_x_plane1, "dQ/dx vs x plane 1;x [cm]", 400, -800, 800, 400, 0, 800);
    H2(Dqdx_vs_x_plane2, "dQ/dx vs x plane 2;x [cm]", 400, -800, 800, 400, 0, 800);

    H2(Dqdx_vs_t, "dQ/dx vs t;t [ms]", 400, -5, 5, 400, 0, 800);
    H2(Dqdx_vs_t_plane0, "dQ/dx vs t plane 0;t [ms]", 400, -5, 5, 400, 0, 800);
    H2(Dqdx_vs_t_plane1, "dQ/dx vs t plane 1;t [ms]", 400, -5, 5, 400, 0, 800);
    H2(Dqdx_vs_t_plane2, "dQ/dx vs t plane 2;t [ms]", 400, -5, 5, 400, 0, 800);

    for (auto h: hists)
	fMVHists["dqdx"].push_back(h);


#undef H1
#undef H2
   TH1* hists_corrected[NHists_dedx] = {};
#define H1(name, title) hists_corrected[name] =			    \
       new TH2F(#name"_corr", Form("dQ/dx vs x%s Corrected;x [cm]",title), \
		400, -800, 800, 400, 0, 800 )
#define H2(name, title) hists_corrected[name] =			    \
       new TH2F(#name"_corr", Form("dQ/dx vs x%s Corrected;t [ms]",title), \
		400, -5, 5, 400, 0, 800 )

    H1(Dqdx_vs_x, "");
    H1(Dqdx_vs_x_plane0, " plane 0");
    H1(Dqdx_vs_x_plane1, " plane 1");
    H1(Dqdx_vs_x_plane2, " plane 2");

    H2(Dqdx_vs_t, "");
    H2(Dqdx_vs_t_plane0, " plane 0");
    H2(Dqdx_vs_t_plane1, " plane 1");
    H2(Dqdx_vs_t_plane2, " plane 2");

    for (auto h: hists_corrected)
	fMVHists["dqdx_corr"].push_back(h);

#undef H1
#undef H2
   TH1* hists_dedx[NHists_dedx] = {};
#define H1(name, title) hists_dedx[name] = new TH2F(#name, Form("dE/dx vs x%s;x [cm]",title), 400, -800, 800, 400, 0, 10 )
#define H2(name, title) hists_dedx[name] = new TH2F(#name, Form("dE/dx vs x%s;t [ms]",title), 400, -5, 5, 400, 0, 10 )

    H1(Dedx_vs_x, "");
    H1(Dedx_vs_x_plane0, " plane 0");
    H1(Dedx_vs_x_plane1, " plane 1");
    H1(Dedx_vs_x_plane2, " plane 2");

    H2(Dedx_vs_t, "");
    H2(Dedx_vs_t_plane0, " plane 0");
    H2(Dedx_vs_t_plane1, " plane 1");
    H2(Dedx_vs_t_plane2, " plane 2");

    for (auto h: hists_dedx)
	fMVHists["dedx"].push_back(h);


    //***** Input tree *****
    auto tree = new TChain("analysistree/anatree");
    size_t size = fFHandler.attachFiles(tree, fIFName, fBatchNo, fNruns, fStartRun, fDataVersion);
    anatree* evt = new anatree(tree);

    fTree = tree;
    fEvent = evt;
    fSize = size;

    // allow only selected branches!
    //
    // This improves the speed of the tree processing. Any branch to
    // be used needs to be added here, otherwise we don't get any data
    // from it.
    std::vector<TString> allowed = {
	// "run",
	// "event",
    	// "geant_list_size",
	// "inTPCActive",
	// "Eng",
	// "Mother",
	// "TrackId",
	// "pdg",
	// "processname",
	// "StartE_tpcAV",
	// "StartPx_tpcAV",
	// "StartPy_tpcAV",
	// "StartPz_tpcAV",
	// "StartP_tpcAV",
	// "EndE_tpcAV",
	// "EndPointx",
	// "EndPointy",
	// "EndPointz",
	// "EndPointx_tpcAV",
	// "EndPointy_tpcAV",
	// "EndPointz_tpcAV",
	// "pathlen",
	"ntracks_pandoraTrack",
	"ntrkhits_pandoraTrack",
	"trkdqdx_pandoraTrack",
	"trkdedx_pandoraTrack",
	"trkxyz_pandoraTrack",
	"trklen_pandoraTrack"
    };
    tree->SetBranchStatus("*", 0);
    AnaTree::AllowBranches(tree, allowed);
    tree->SetMakeClass(1);

    //***** Output file *****
    fOutFile = TFile::Open(Form("%sanahists.root", fOutPref.Data()), "UPDATE");

    return fOutFile->IsOpen();
}

int DqDxProcessor::Process()
{
    //***** Process *****
    size_t size = fSize;
    auto tree = fTree;
    auto evt = fEvent;

    // get hist pointers
    auto hists = fMVHists["dqdx"];
    auto hists_corrected = fMVHists["dqdx_corr"];
    auto hists_dedx = fMVHists["dedx"];


    cout<<"Starting a loop over the tree"<<endl;
    int entries_processed = 0;
    int fiftieth = size / 50;
    cout<<"Will loop over "<<size<<" entries."<<endl;

    size_t events_passing_selection = 0;
    size_t tracks_passing_selection = 0;

    //size = 50000;
    cout<<"|                                                  |\r|";
    size_t ientry = 0;
    while ( tree->GetEntry(ientry++) ) {
	// print progress
	if ( (ientry)%fiftieth == 0) {
	    cout<<"-";
	    cout.flush();
	}

	entries_processed++;

	if (!SelectEvent(evt)) continue;

	int ntracks = evt->ntracks_pandoraTrack;

	// loop over stored tracks
	bool passed = false ;
	for (int itrack = 0; itrack < ntracks; ++itrack) {
	    // make a threshold on reco track length
	    if (!SelectTrack(evt, itrack)) continue;

	    if (!passed) passed = true;

	    ++tracks_passing_selection;

	    // choose the best plane
	    int best_plane = 0;
	    int tmp_hits = 0;
	    for (int i = 0; i < 3; ++i) {
		if ( evt->ntrkhits_pandoraTrack[itrack][i] > tmp_hits ) {
		    tmp_hits = evt->ntrkhits_pandoraTrack[itrack][i];
		    best_plane = i;
		}
	    }

	    // loop over all planes
	    // loop over hits in the best plane
	    for (int iplane = 0; iplane < 3; ++iplane) {
		for (int i = 0; i < tmp_hits; ++i) {
		    double dqdx = evt->trkdqdx_pandoraTrack[itrack][iplane][i];
		    double dedx = evt->trkdedx_pandoraTrack[itrack][iplane][i];
		    if ( dqdx != 0.) {
			double x = evt->trkxyz_pandoraTrack[itrack][iplane][i][0];
			double t = x * kXtoT;

			hists[Dqdx_vs_x_plane0 + iplane]->Fill(x, dqdx);
			hists[Dqdx_vs_t_plane0 + iplane]->Fill(t, dqdx);

			if (iplane == best_plane) {
			    hists[Dqdx_vs_x]->Fill(x, dqdx);
			    hists[Dqdx_vs_t]->Fill(t, dqdx);
			}

			// do corrections, only within main TPCs
			if ( x > APA_X_POSITIONS[0]
			     && x < APA_X_POSITIONS[2] ) {
			    // get the hits drift time from the point's x coordinate
			    int tpc = whichTPC(x);
			    double dt = ( -1 + 2*((tpc+1)%2) )*(x - APA_X_POSITIONS[(tpc+1)/2]);
			    dt *= kXtoT;
			    double correction = TMath::Exp(-dt/2.88);
			    double dqdx_corrected = dqdx/correction;

			    hists_corrected[Dqdx_vs_x_plane0 + iplane]->Fill(x, dqdx_corrected);
			    hists_corrected[Dqdx_vs_t_plane0 + iplane]->Fill(t, dqdx_corrected);
			    if (iplane == best_plane) {
				hists_corrected[Dqdx_vs_x]->Fill(x, dqdx_corrected);
				hists_corrected[Dqdx_vs_t]->Fill(t, dqdx_corrected);
			    }
			}
		    }
		    if ( dedx != 0.) {
			double x = evt->trkxyz_pandoraTrack[itrack][iplane][i][0];
			double t = x * kXtoT;
			hists_dedx[Dedx_vs_x_plane0 + iplane]->Fill(x, dedx);
			hists_dedx[Dedx_vs_t_plane0 + iplane]->Fill(t, dedx);
			if (iplane == best_plane) {
			    hists_dedx[Dedx_vs_x]->Fill(x, dedx);
			    hists_dedx[Dedx_vs_t]->Fill(t, dedx);
			}
		    }
		}
	    }

	}// track loop

	if (passed)
	    ++events_passing_selection;

    }// tree entry loop
    cout<<"|"<<endl;

    cout<<"Done."<<endl;
    cout<<"Processed "<<entries_processed<<" entries."<<endl;
    cout<<"Events passing selection: "<<events_passing_selection<<endl
	<<"Tracks passing selection: "<<tracks_passing_selection<<endl;

    return 1;
}


int DqDxProcessor::Finalize()
{
    fOutFile->cd();
    //***** Save hists *****
    for (auto histspair: fMVHists) {
	for (auto h : histspair.second) {
	    h->Write(0, TObject::kOverwrite);
	}
    }

    fOutFile->Close();
    cout<<"Saved and closed output file "<<fOutFile->GetName()<<endl;
    return 1;
}

int DqDxProcessor::SelectEvent(anatree* evt)
{
    return 1;
}

int DqDxProcessor::SelectTrack(anatree* evt, int itrack)
{
    return (evt->trklen_pandoraTrack[itrack] >= 300.); // at least 2-m long track
}


int DqDxProcessor::whichTPC(double x) {
    int i = 0;
    for (; i < 4; ++i) {
	int iapa = (i+1)/2;
	int icpa = i/2;
	double xapa = APA_X_POSITIONS[iapa];
	double xcpa = CPA_X_POSITIONS[icpa];
	if ( (x > xapa && x < xcpa) || (x < xapa && x > xcpa) )
	    break;
    }

    return i;
}


#endif
