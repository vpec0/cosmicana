/**
 * Processes the CosmicMuonEvent files.
 *
 **/


#include "CosmicMuonEvent.h"

void doXlog(TH1* h);

void hists_reco_eff(const char* fname = "", const char* outpref = "")
{
    enum {
	HNRecoVsEloss = 0,
	NHists,
	GEffVsEloss = 0,
	NGraphs
    };

    TH1* hists[NHists] = {};
#define H1(name, title, nbins, low, high) hists[name] = new TH1F(#name, title, nbins, low, high)
#define H2(name, title, nbinsx, lowx, highx, nbinsy, lowy, highy) \
    hists[name] = new TH2F(#name, title, nbinsx, lowx, highx, nbinsy, lowy, highy)

    H2(HNRecoVsEloss, ";Energy Loss [MeV];Number of Reconstructed Tracks", 200, 0., 20., 50, 0., 50.);

    TGraph* graphs[NGraphs] = {};
#define GR(name, title, npoints) {auto gr = new TGraph(npoints); gr->SetNameTitle(#name, title); \
	graphs[name] = gr; }

    GR(GEffVsEloss, "", 20);

    //***** Input tree *****
    TChain* tree = new TChain("cosmicMuons");
    tree->Add(fname);
    auto evt = new CosmicMuonEvent::Event_t();
    CosmicMuonEvent::registerTree(tree, *evt);

    // allow only selected branches!
    //
    // This improves the speed of the tree processing. Any branch to
    // be used needs to be added here, otherwise we don't get any data
    // from it.
    std::vector<TString> allowed = {
	"run",
	"event",
	"StartE_tpcAV",
	"StartPointx_tpcAV",
	"StartPointy_tpcAV",
	"StartPointz_tpcAV",
	"StartPx_tpcAV",
	"StartPy_tpcAV",
	"StartPz_tpcAV",
	"StartP_tpcAV",
	"EndPointx_tpcAV",
	"EndPointy_tpcAV",
	"EndPointz_tpcAV",
	"Eloss",
	"pathlen",
	"nRecoPandora",
    };
    tree->SetBranchStatus("*", 0);
    for (auto br : allowed)
	tree->SetBranchStatus(br, 1);


    //***** Process *****
    cout<<"Starting a loop over the tree"<<endl;
    int size = tree->GetEntries();
    int fiftieth = size / 50;
    cout<<"Will loop over "<<size<<" entries."<<endl;
    int entries_processed = 0;

    //size = 50000;
    cout<<"|                                                  |\r|";
    for (int ientry = 0; ientry < size; ++ientry) {
	// print progress
	if ( (ientry+1)%fiftieth == 0) {
	    cout<<"-";
	    cout.flush();
	}

	// get an entry
	int status = tree->GetEntry(ientry);
	if (!status) break;
	entries_processed++;

    }// tree entry loop
    cout<<"|"<<endl;


    //***** Output file *****
    auto outf = TFile::Open(Form("%sanahists.root", outpref), "UPDATE");

    //***** Save hists *****
    for (auto h : hists) {
	h->Write(0, TObject::kOverwrite);
    }

    //***** Save graphs *****
    for (auto gr : graphs) {
	gr->Write(0, TObject::kOverwrite);
    }

    outf->Close();
    cout<<"Saved and closed output file "<<outf->GetName()<<endl;
}


void doXlog(TH1* h)
// redo scales for x-log hists
{
    TAxis* axis = h->GetXaxis();

    double start = TMath::Log10(axis->GetXmin());
    double stop = TMath::Log10(axis->GetXmax());
    double range = stop - start;
    int nbins = axis->GetNbins();
    double binwidth = range / nbins;

    double *bins = new double[nbins+1];
    for (int i = 0; i < (nbins+1); ++i) {
        bins[i] = TMath::Power(10, start + i*binwidth);
    }

    axis->Set(nbins, bins);

    delete[] bins;
}
