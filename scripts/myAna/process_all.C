#define CosmicAna_cxx
#include "CosmicAna.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

typedef vector<TH1*> hist_vec_t;

TCanvas* processHists(hist_vec_t& hists, const char* pref);
TH2* getMean(TH3* h, int pax = 0, int xax = 2, int yax = 1);

enum {
    kX = 0,
    kY,
    kZ
};

const unsigned int NVOX_X = 10;
const unsigned int NVOX_Y = 10;
const unsigned int NVOX_Z = 10;

void CosmicAna::Loop()
{
//   In a ROOT session, you can do:
//      root> .L CosmicAna.C
//      root> CosmicAna t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;

   // create de/dx histograms for voxels in the detector
   hist_vec_t hists;
   for (int ix = 0; ix < NVOX_X; ix++) {
       for (int iy = 0; iy < NVOX_Y; iy++) {
	   for (int iz = 0; iz < NVOX_Z; iz++) {
	       auto h = new TH1I(Form("h%02d%02d%02d",ix,iy,iz),
				 "", 50, 0, 5);
	       hists.push_back(h);
	   }
       }
   }



   Long64_t nentries = fChain->GetEntriesFast();
   cout<<"Will process "<<nentries<<" entries"<<endl;

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;

      // decide on voxzel
      static const double xmin = -300;
      static const double xmax = 300;
      static const double ymin = 0;
      static const double ymax = 600;
      static const double zmin = 0;
      static const double zmax = 600;

      static const double dx = (xmax - xmin)/NVOX_X;
      static const double dy = (ymax - ymin)/NVOX_Y;
      static const double dz = (zmax - zmin)/NVOX_Z;

      for (int ip = 0; ip < n_points; ip++) {
	  int ix = (dedx_pos[ip][0] - xmin)/dx;
	  int iy = (dedx_pos[ip][1] - ymin)/dy;
	  int iz = (dedx_pos[ip][2] - zmin)/dz;

	  if ( !(ix >= 0 && ix < NVOX_X &&
		 iy >= 0 && iy < NVOX_Y &&
		 iz >= 0 && iz < NVOX_Z) ) continue;

	  hists[(ix*NVOX_Y+iy)*NVOX_Z + iz] -> Fill(dedx[ip]);

      }
   }

   // dump info on one stored histogram
   size_t size = sizeof(*hists[0]);
   cout<<"h1 size: "<<(size/1e3)<<"kB"<<endl;

   size = 0;
   for (auto h: hists)
       size += sizeof(*h);
   cout<<"All hists size: "<<(size/1e3)<<"kB"<<endl;


   cout<<"About to process hists of all tracks"<<endl;
   auto c1 = processHists(hists, "");
   cout<<"Will clear all the used hists"<<endl;
   for(auto h: hists)
       delete h;
   hists.clear();

   c1->SetTitle("All");
}


int process_all(const char* fname)
{
    gStyle -> SetOptStat(0);
    gStyle -> SetNumberContours(255);

    TChain* tree = new TChain("CosmicAna/muon_tree");
    tree->Add(fname);
    CosmicAna* ana = new CosmicAna(tree);
    ana->Loop();

    return 0;
}


TCanvas* processHists(hist_vec_t& hists, const char* pref)
{
    auto h3 = new TH3D(Form("%smvp", pref),";X [cm];Y [cm];Z [cm]",
		       NVOX_X, -300, 300,
		       NVOX_Y, 0, 600,
		       NVOX_Z, 0, 600);

    // auto h3err = new TH3D(Form("%smvp_err", pref),";X [cm];Y [cm];Z [cm]",
    // 			  NVOX_X, -300, 300,
    // 			  NVOX_Y, 0, 600,
    // 			  NVOX_Z, 0, 600);

    size_t size = sizeof(*h3);
    cout<<"h3 size: "<<(size/1e3)<<"kB"<<endl;

    for (int ix = 0; ix < NVOX_X; ix++) {
	for (int iy = 0; iy < NVOX_Y; iy++) {
	    for (int iz = 0; iz < NVOX_Z; iz++) {
		auto h = hists[(ix*NVOX_Y+iy)*NVOX_Z + iz];
		if (h->GetEntries() < 20) continue;

		double maxbin = h -> GetMaximumBin();
		double maxdedx = h -> GetBinCenter(maxbin);
		// cout <<"About to do fit "<<ix <<", "<<iy<<", "<<iz<<endl;
		h -> Fit("landau", "LQ0", "", maxdedx - 0.5, maxdedx + 1);
		auto fit = h -> GetFunction("landau");

		double mpv = fit->GetParameter(1);
		double mpv_err = fit->GetParError(1);

		if ( mpv_err < 0.05 ) {
		    h3 -> SetBinContent (ix+1, iy+1, iz+1, mpv );
		    // h3err -> SetBinContent (ix+1, iy+1, iz+1, mpv_err );
		}

		// cout<<"Voxel "<<ix<<", "<<iy<<", "<<iz<<": ";
		// cout<<h->GetEntries()
		//     <<" entries; MPV: "
		//     <<mpv
		//     <<", MPV Error: "<<mpv_err<<endl;
	    }
	}
    }

    auto c = new TCanvas(Form("%sc", pref),"dE/dX MPV", 900, 300);
    c->Divide(3,1);
    c->cd(1); getMean(h3, kZ, kX, kY) -> Draw("colz");
    c->cd(2); getMean(h3, kX, kZ, kY) -> Draw("colz");
    c->cd(3); getMean(h3, kY, kZ, kX) -> Draw("colz");


    return c;
}


TH2* getMean(TH3* h, int pax, int xax, int yax)
{
    static const char* labels[3] = {"x", "y", "z"};
    TString outlabel = labels[yax];
    outlabel += labels[xax];

    TAxis* axes[3];
    axes[kX] = h->GetXaxis();
    axes[kY] = h->GetYaxis();
    axes[kZ] = h->GetZaxis();

    int nbins[3] = { axes[kX]->GetNbins(),
		     axes[kY]->GetNbins(),
		     axes[kZ]->GetNbins() };

    auto hout = new TH2D(Form("%sp%s", h->GetName(), (const char*)outlabel),
			 Form(";%s;%s", axes[xax]->GetTitle(), axes[yax]->GetTitle()),
			 nbins[xax], axes[xax]->GetXmin(), axes[xax]->GetXmax(),
			 nbins[yax], axes[yax]->GetXmin(), axes[yax]->GetXmax()
			 );

    // sum contents of input hist along projection axis into the output hist
    // ix, iy, iz run over bins of input hist
    for (int ix = 0; ix < nbins[kX]; ix++) {
	for (int iy = 0; iy < nbins[kY]; iy++) {
	    for (int iz = 0; iz < nbins[kZ]; iz++) {
		int bins[3] = {ix+1, iy+1, iz+1};
		double cont = hout -> GetBinContent(bins[xax], bins[yax]);
		cont += h -> GetBinContent(ix+1, iy+1, iz+1);
		hout -> SetBinContent(bins[xax], bins[yax], cont);
	    }
	}
    }

    // normalise the output 2D hist -- divide by number of input bins in the projection axis
    // ix, iy run over bins of output hist
    for (int ix = 0; ix < nbins[xax]; ix++) {
	for (int iy = 0; iy < nbins[yax]; iy++) {
	    double cont = hout->GetBinContent(ix+1, iy+1);
	    hout->SetBinContent(ix+1, iy+1, cont / nbins[pax]);
	}
    }

    hout->SetAxisRange(1.8, 2.05, "z");

    return hout;
}
