/**
 * Prototype script to process standard analysis tree.
 **/


#include "anatree.h"


#define FOR(i, size) for (int i = 0; i < size; ++i)
typedef const char* STR;

typedef std::map<TString,TH1*> HistList_t;
typedef std::map<TString,TGraph*> GraphList_t;


void redoAspect (TH1*, TCanvas*);
void loop(anatree*, HistList_t&, GraphList_t&);

void processor(STR fname, STR outpref)
{
    cout<<"Starting basicPlots2"<<endl;
    //***** Output Histograms *****

    STR views[3] = {
	"top", "side", "back"
    };

    const int ngraphs_3views = 6;
    STR graphnames_3views[ngraphs_3views] = {
    	"startAll",
	"endAll",
	"startPrimary",
	"endPrimary",
	"startPrimaryInActive",
	"endPrimaryInActive",

    };
    const int nhists = 0;
    STR histnames[nhists] = {
    };

    // cout<<"Will create following histograms:"<<endl
    // 	<<"    ";
    HistList_t hists;
    // FOR(i, nhists_3views) {
    // 	FOR(j, 3) {
    // 	    TString hname = Form("%s_%s", histnames_3views[i], views[j]);
    // 	    auto h = new TH2I(hname, hname, 2, 0., -1, 2, 0., -1.);
    // 	    hists[hname] = h;
    // 	    cout<<h->GetName()<<", ";
    // 	}
    // }
    // cout<<endl;

    cout<<"Will create following graphs:"<<endl
    	<<"    ";
    GraphList_t graphs;
    FOR(i, ngraphs_3views) {
	FOR(j, 3) {
	    TString hname = Form("%s_%s", graphnames_3views[i], views[j]);
	    auto gr = new TGraph();
	    gr->SetName(hname);
	    graphs[hname] = gr;
	    cout<<gr->GetName()<<", ";
	}
    }
    cout<<endl;

    //***** Input tree *****
    auto tree = new TChain("analysistree/anatree");
    int status = tree->Add(fname);
    if (! status) {
	cout<<"Can't find tree "<< tree->GetName() << " in file "<<fname<<endl;
	return;
    } else {
	cout<<"Will process tree "<< tree->GetName() << " in file "<<fname<<endl;
    }
    cout<<(tree->GetEntries())<<endl;

    anatree* evt = new anatree(tree); // need to create on heap, root alocate only limited memory for stack

    // allow only selected branches!
    std::vector<TString> allowed = {
	"Mother",
    	"geant_list_size",
	"StartPointx", "StartPointy", "StartPointz",
	"EndPointx", "EndPointy", "EndPointz",
    };
    tree->SetBranchStatus("*", 0);
    AnaTree::AllowBranches(tree, allowed);


    //***** Process *****
    cout<<"Starting a loop over the tree"<<endl;
    loop(evt, hists, graphs);
    cout<<"Done."<<endl;

    //***** Output file *****
    auto outf = TFile::Open(Form("%sanahists.root", outpref), "UPDATE");

    //***** Save hists *****
    for (auto h: hists) {
	h.second->Write(0, TObject::kOverwrite);
    }
    outf->Close();
    cout<<"Saved and closed output file"<<endl;
}


void loop(anatree* evt, HistList_t& hists, GraphList_t& graphs)
{
    TTree* tree = evt->fChain;
    int size = tree->GetEntries();

    FOR(i, size) {
	tree->GetEntry(i);

	FOR(j, evt->geant_list_size) {
	    graphs["startAll_top"] -> Fill (evt->StartPointz[j], evt->StartPointx[j]);
	    graphs["startAll_side"] -> Fill (evt->StartPointz[j], evt->StartPointy[j]);
	    graphs["startAll_back"] -> Fill (evt->StartPointx[j], evt->StartPointy[j]);

	    graphs["endAll_top"] -> Fill (evt->EndPointz[j], evt->EndPointx[j]);
	    graphs["endAll_side"] -> Fill (evt->EndPointz[j], evt->EndPointy[j]);
	    graphs["endAll_back"] -> Fill (evt->EndPointx[j], evt->EndPointy[j]);

	    if (evt->Mother[j] == 0) {
		graphs["startPrimary_top"] -> Fill (evt->StartPointz[j], evt->StartPointx[j]);
		graphs["startPrimary_side"] -> Fill (evt->StartPointz[j], evt->StartPointy[j]);
		graphs["startPrimary_back"] -> Fill (evt->StartPointx[j], evt->StartPointy[j]);

		graphs["endPrimary_top"] -> Fill (evt->EndPointz[j], evt->EndPointx[j]);
		graphs["endPrimary_side"] -> Fill (evt->EndPointz[j], evt->EndPointy[j]);
		graphs["endPrimary_back"] -> Fill (evt->EndPointx[j], evt->EndPointy[j]);
	    }
	}
    }
}



void redoAspect (TH1* h, TCanvas* c1)
{
	double xmin = h->GetXaxis()->GetXmin();
	double xmax = h->GetXaxis()->GetXmax();
	double ymin = h->GetYaxis()->GetXmin();
	double ymax = h->GetYaxis()->GetXmax();

	double aspect = (xmax - xmin)/(ymax - ymin);

	cout<<"Histogram size (w x h): " << (xmax - xmin) << " x " << (ymax - ymin);
	cout<<"Aspect ratio: " << aspect << endl;

	// correct current canvas size
	double ww = c1->GetWw();
	double wh = c1->GetWh();
	double windoww = c1->GetWindowWidth();
	double windowh = c1->GetWindowHeight();

	cout <<"World width, height: " << ww << " x "<<wh<<endl
	     <<"Window size (w x h): " << windoww << " x " << windowh << endl;

	c1->SetCanvasSize(aspect * wh, wh);
	c1->SetWindowSize(c1->GetWw() + (windoww - ww), windowh);
	gPad->SetCanvasSize(aspect * wh, wh);
}
