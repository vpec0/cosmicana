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
		  const char* data_version = "v08_34_00", const char* source = "", const char* topdir = "");
    ~DqDxProcessor(){}

    int Initialize();
    int Process();
    int Finalize();

protected:
    virtual TTree* findInputTree();

private:
    virtual int SelectEvent(anatree* evt);
    virtual int SelectTrack(anatree* evt, int itrack);
    virtual int SelectHit(anatree* evt, int itrack, int iplane, int ihit);

protected:
    int whichTPC(double x);

protected:
    const double APA_X_POSITIONS[3] = {-726.7681, 0., 726.7681};
    const double CPA_X_POSITIONS[2] = {-363.38405, 363.38405};
    const double TPC_Z_SIZE = 232.39;
    const double kXtoT = 1./160.563; // converts cm to ms, calculated for field 0.5kV/cm and temperature 87K

    std::vector<TString> fAllowed;

protected:
    TChain* fTree;
    anatree* fEvent;

    size_t fSize;

    TString fIFName;
    TString fOutPref;
    TString fSource;
    TString fTopdir;

    TFile* fOutFile;

    size_t fBatchNo;
    size_t fNruns;
    size_t fStartRun;

    TString fDataVersion;

    map<TString, vector<TH1*> > fMVHists;
    map<TString, map<int, vector<TH1*> > > fMVHistsByTPC;

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
			     const char* data_version = "v08_34_00",
			     const char* source = "", const char* topdir = ""):
    fTree(0),
    fEvent(0),
    fSize(0),
    fIFName(fname),
    fOutPref(outpref),
    fSource(source),
    fTopdir(topdir),
    fOutFile(0),
    fBatchNo(batchNo),
    fNruns(Nruns),
    fStartRun(startRun),
    fDataVersion(data_version)
{
    // allow only selected branches!
    //
    // This improves the speed of the tree processing. Any branch to
    // be used needs to be added here, otherwise we don't get any data
    // from it.
    std::vector<TString> allowed = {
	"run",
	"event",
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
	"trklen_pandoraTrack",
	//	"trktpc_pandoraTrack"
    };
    for (auto allow: allowed)
	fAllowed.push_back(allow);
}


int DqDxProcessor::Initialize()
{
    /// Histograms

#define HIST_LIST				\
    H1(Dqdx_vs_x, "");				\
    H1(Dqdx_vs_x_plane0, " plane 0");		\
    H1(Dqdx_vs_x_plane1, " plane 1");		\
    H1(Dqdx_vs_x_plane2, " plane 2");		\
    H2(Dqdx_vs_t, "");				\
    H2(Dqdx_vs_t_plane0, " plane 0");		\
    H2(Dqdx_vs_t_plane1, " plane 1");		\
    H2(Dqdx_vs_t_plane2, " plane 2")		\


    TH1* hists[NHists] = {};
#define H1(name, title) hists[name] =			    \
       new TH2D(#name, Form("dQ/dx vs x%s;x [cm]",title), \
		400, -800, 800, 400, 0, 800 )
#define H2(name, title) hists[name] =			    \
       new TH2D(#name, Form("dQ/dx vs x%s;t [ms]",title), \
		400, -5, 5, 400, 0, 800 )

    HIST_LIST;

    for (auto h: hists)
	fMVHists["dqdx"].push_back(h);
#undef H1
#undef H2


    // do the same for each TPC -> 300 histograms x 8
// #define H1(name, title) tmphists[name] =				\
// 	new TH2F(Form("%s_tpc%d", #name, itpc), Form("TPC %d: dQ/dx vs x%s;x [cm]", itpc, title), \
// 		 150, -100, 500, 250, 0, 500 )
// #define H2(name, title) // don't create any vs t hists
//     // tmphists[name] =							\
//     // new TH2F(Form("%s_tpc%d", #name, itpc), Form("TPC %d: dQ/dx vs x%s;t [ms]", itpc, title), \
//     // 	 400, -5, 5, 250, 0, 500 )

//     for (int itpc = 0; itpc < 300; ++itpc) {
// 	TH1* tmphists[NHists] = {};

// 	HIST_LIST;

// 	for (auto h: tmphists)
// 	    fMVHistsByTPC["dqdx"][itpc].push_back(h);
//     }
// #undef H1
// #undef H2


    TH1* hists_corrected[NHists_dedx] = {};
#define H1(name, title) hists_corrected[name] =				\
	new TH2D(#name"_corr", Form("dQ/dx vs x%s Corrected;x [cm]",title), \
		 400, -800, 800, 400, 0, 800 )
#define H2(name, title) hists_corrected[name] =				\
	new TH2D(#name"_corr", Form("dQ/dx vs x%s Corrected;t [ms]",title), \
		 400, -5, 5, 400, 0, 800 )

    HIST_LIST;

    for (auto h: hists_corrected)
	fMVHists["dqdx_corr"].push_back(h);
#undef H1
#undef H2


    TH1* hists_dedx[NHists_dedx] = {};
#define H1(name, title) hists_dedx[name] = new TH2D(#name, Form("dE/dx vs x%s;x [cm]",title), 400, -800, 800, 400, 0, 10 )
#define H2(name, title) hists_dedx[name] = new TH2D(#name, Form("dE/dx vs x%s;t [ms]",title), 400, -5, 5, 400, 0, 10 )
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
#undef H1
#undef H2


    //**** Input tree ****
    findInputTree();

    //***** Output file *****
    fOutFile = TFile::Open(Form("%sanahists.root", fOutPref.Data()), "recreate");

    return fOutFile->IsOpen();
}

TTree* DqDxProcessor::findInputTree()
{
    if (fIFName.Length() && fNruns && fStartRun) {

	TString baseifname(fIFName);
	baseifname.ReplaceAll( "{batch}", Form("%ld", (fStartRun/100)*100) );

	cout<<baseifname<<endl;

	//***** Input tree *****
	auto tree = new TChain("analysistree/anatree");
	size_t size = 0;
	for (int i = 0; i < fNruns; ++i) {
	    TString ifname(baseifname);
	    ifname.ReplaceAll("{run}", Form("%ld", fStartRun + i) );
	    cout<<"Attaching file "<<ifname<<endl;
	    size += tree->Add(ifname, -1);
	}
	cout<<size<<" number of files attached."<<endl;

	anatree* evt = new anatree(tree);

	fTree = tree;
	fEvent = evt;

	if (size == 0)
	    return 0;
	tree->GetEntry(0);
	fSize = size*tree->GetTree()->GetEntries();

	tree->SetBranchStatus("*", 0);
	AnaTree::AllowBranches(tree, fAllowed);
	tree->SetMakeClass(1);

	return tree;
    }

    return 0;
}
int DqDxProcessor::Finalize()
{
    fOutFile->cd();
    //***** Save hists *****
    for (auto histspair: fMVHists) {
	fOutFile->cd();
	fOutFile->mkdir(histspair.first)->cd();
	cout<<"Making directory "<<histspair.first<<endl;
	for (auto h : histspair.second) {
	    h->Write(0, TObject::kOverwrite);
	}
    }

    fOutFile->cd();
    // for (auto tpcpair: fMVHistsByTPC) {
    // 	auto dir = fOutFile->GetDirectory(tpcpair.first);
    // 	dir->cd();
    // 	cout<<"Descending to directory "<<tpcpair.first<<endl;
    // 	for (auto histspair : tpcpair.second) {
    // 	    dir->mkdir(Form("TPC%d",histspair.first))->cd();
    // 	    for (auto h : histspair.second) {
    // 		// skip not filled hitograms
    // 		if (!h || h->GetEntries() == 0)
    // 		    continue;
    // 		h->Write(0, TObject::kOverwrite);
    // 	    }
    // 	}
    // }


    fOutFile->Close();
    cout<<"Saved and closed output file "<<fOutFile->GetName()<<endl;
    return 1;
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

    //auto hists_bytpc = fMVHistsByTPC["dqdx"];

    cout<<"Starting a loop over the tree"<<endl;
    int entries_processed = 0;
    int fiftieth = size / 50;
    cout<<"Will loop over "<<size<<" entries."<<endl;

    size_t events_passing_selection = 0;
    size_t tracks_passing_selection = 0;

    //size = 50000;
    cout<<"|                                                  |\r|";
    size_t ientry = 0;
    // cout<<">>> Trying to read tree entry: "<<endl;
    tree->GetEntry(0);
    // cout<<">>> before loop"<<endl;
    while ( tree->GetEntry(ientry++) ) {
	// cout<<">>> inside loop"<<endl;

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

	    bool track_passed = false;

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
		int nhits = evt->ntrkhits_pandoraTrack[itrack][iplane];
		if (nhits > MAX_TRACK_HITS)
		    nhits = MAX_TRACK_HITS;
		Float_t* dqdx_arr = evt->trkdqdx_pandoraTrack[itrack][iplane];
		Float_t* dedx_arr = evt->trkdedx_pandoraTrack[itrack][iplane];
		//Int_t* tpc_arr = evt->trktpc_pandoraTrack[itrack][iplane];
		Float_t* xyz_arr = (Float_t*)evt->trkxyz_pandoraTrack[itrack][iplane];
		for (int i = 0; i < nhits; ++i) {
		    if (!SelectHit(evt, itrack, iplane, i)) continue;

		    if (!track_passed) track_passed = true;

		    double dqdx = dqdx_arr[i];
		    double dedx = dedx_arr[i];
		    //int tpc = tpc_arr[i];
		    if ( dqdx != 0.) {
			double x = xyz_arr[i*3];
			double t = x * kXtoT;

			hists[Dqdx_vs_x_plane0 + iplane]->Fill(x, dqdx);
			hists[Dqdx_vs_t_plane0 + iplane]->Fill(t, dqdx);

			if (iplane == best_plane) {
			    hists[Dqdx_vs_x]->Fill(x, dqdx);
			    hists[Dqdx_vs_t]->Fill(t, dqdx);
			}

			// int tpcx = tpc % 6;

			int tpc = whichTPC(x) + 1;
			double dx = ( -1 + 2*(tpc%2) )*(x - APA_X_POSITIONS[tpc/2]);
			double dt = dx*kXtoT;

			// // fill hists by tpc
			// hists_bytpc[tpc][Dqdx_vs_x_plane0 + iplane]->Fill(dx, dqdx);

			// do corrections, only within main TPCs
			if ( x > APA_X_POSITIONS[0]
			     && x < APA_X_POSITIONS[2] ) {
			    // get the hits drift time from the point's x coordinate
			    // TPCs include cryo-side volumes; 6 per
			    // row, 12 per slice, 25 slices

			    // do corrected histograms
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
			double x = xyz_arr[i*3];
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
	    if (track_passed) {
		++tracks_passing_selection;
		if (!passed) passed = true;

#ifdef DEBUG
		double start_x = evt->trkstartx_pandoraTrack[itrack];
		double end_x = evt->trkendx_pandoraTrack[itrack];
		cout<<""
		    <<"Entry: "<<ientry
		    <<", event: "<<evt->event
		    <<", track: "<<itrack
		    <<", track start x: "<<start_x
		    <<", track end x: "<<end_x
		    <<endl;
#endif // DEBUG
	    }

	}// track loop

	if (passed) {
	    ++events_passing_selection;

#ifdef DEBUG
	    fMVHists["dqdx"][3]->Draw("colz");
	    gPad->Modified();
	    gPad->Update();
	    getchar();
	    fMVHists["dqdx"][3]->Reset();
#endif // DEBUG
	}

    }// tree entry loop
    cout<<"|"<<endl;

    cout<<"Done."<<endl;
    cout<<"Processed "<<entries_processed<<" entries."<<endl;
    cout<<"Events passing selection: "<<events_passing_selection<<endl
	<<"Tracks passing selection: "<<tracks_passing_selection<<endl;

    return 1;
}


int DqDxProcessor::SelectEvent(anatree* evt)
{
    return 1;
}

int DqDxProcessor::SelectTrack(anatree* evt, int itrack)
{
    return (evt->trklen_pandoraTrack[itrack] >= 200.); // at least 2-m long track
}

int DqDxProcessor::SelectHit(anatree* evt, int itrack, int iplane, int ihit)
{
    return true;
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
