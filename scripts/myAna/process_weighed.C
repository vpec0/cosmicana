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

const double xmin = -300;
const double xmax = 300;
const double ymin = 0;
const double ymax = 600;
const double zmin = 0;
const double zmax = 600;

const double dx = (xmax - xmin)/NVOX_X;
const double dy = (ymax - ymin)/NVOX_Y;
const double dz = (zmax - zmin)/NVOX_Z;

TString gOutpref("");

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
   hist_vec_t hists_pierce;
   for (int ix = 0; ix < NVOX_X; ix++) {
       for (int iy = 0; iy < NVOX_Y; iy++) {
	   for (int iz = 0; iz < NVOX_Z; iz++) {
	       TString title = Form("X:[%.1f,%.1f],Y:[%.1f,%.1f],"
				    "Z:[%.1f,%.1f];dE/dx [MeV/cm]",
				    xmin+ix*dx, xmin+(ix+1)*dx,
				    ymin+iy*dy, ymin+(iy+1)*dy,
				    zmin+iz*dz, zmin+(iz+1)*dz);
	       auto h = new TH1I(Form("h%02d%02d%02d",ix,iy,iz),
				 "All tracks "+title, 100, 0, 5);
	       hists.push_back(h);
	       auto hp = new TH1D(Form("hP%02d%02d%02d",ix,iy,iz),
				 "Piercing tracks "+title, 100, 0, 5);
	       hists_pierce.push_back(hp);
	   }
       }
   }
   vector<double> total_weight, total_weight_pierce;
   total_weight.resize(hists.size(), 0);
   total_weight_pierce.resize(hists_pierce.size(), 0);


   Long64_t nentries = fChain->GetEntriesFast();
   cout<<"Will process "<<nentries<<" entries"<<endl;

   int npiercers = 0;
   int nall = 0;

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;

      nall++;

      bool cathode_crosser = (startPos[0] * endPos[0]) < 0.;
      bool anode_crosser = (// starts at an anode face
			    (startPos[1] < 606. && startPos[2] < 694 && startPos[2] > 0.5) ||
			    // exits at an anode face (within 1 cm of the plane)
			    (endPos[1] < 606. && startPos[2] < 694 && startPos[2] > 0.5
			     && (endPos[0] < -357. || endPos[0] > 357.))
			    );
      bool piercer = cathode_crosser && anode_crosser;
      if(piercer)
	  npiercers++;

      // loop over all trajectory points and store their de/dx in the appropriate voxel hist
      // FIXME: I beleive each contribution should be weighted by the path length.
      // Longer segments should have larger weight. Add the weight during filling and store
      // cumulative weight, then rescale by that factor.
      TVector3 prev_pos(startPos);
      for (int ip = 0; ip < n_points; ip++) {
	  // decide on voxel
	  int ix = (dedx_pos[ip][0] - xmin)/dx;
	  int iy = (dedx_pos[ip][1] - ymin)/dy;
	  int iz = (dedx_pos[ip][2] - zmin)/dz;

	  TVector3 current_pos(sp_pos[ip]);
	  double dd = (current_pos - prev_pos).Mag(); // segment distance; use it as a weight
	  prev_pos = current_pos;

	  if ( !(ix >= 0 && ix < NVOX_X &&
		 iy >= 0 && iy < NVOX_Y &&
		 iz >= 0 && iz < NVOX_Z) ) continue;

	  int idx = (ix*NVOX_Y+iy)*NVOX_Z + iz;
	  hists[idx] -> Fill(dedx[ip], dd); // weighing by the segment distance
	  total_weight[idx] += dd;
	  if (piercer) {
	      hists_pierce[idx] -> Fill(dedx[ip], dd);
	      total_weight_pierce[idx] += dd;
	  }
      }
   }

   // renormalize the histograms
   auto it = total_weight.begin();
   for (auto h: hists){
       h->Scale(h->GetEntries()/(*it));
       it++;
   }
   it = total_weight_pierce.begin();
   for (auto h: hists_pierce){
       h->Scale(h->GetEntries()/(*it));
       it++;
   }


   cout<< "Number of all muon tracks: "<<nall<<endl;
   cout<< "Number of piercing muon tracks: "<<npiercers<<endl;


   // dump info on one stored histogram
   size_t size = sizeof(*hists[0]);
   cout<<"h1 size: "<<(size/1e3)<<"kB"<<endl;

   size = 0;
   for (auto h: hists)
       size += sizeof(*h);
   cout<<"All hists size: "<<(size/1e3)<<"kB"<<endl;

   cout<<"About to process hists of all tracks"<<endl;
   auto c1 = processHists(hists, "");
   // cout<<"Will clear all the used hists"<<endl;
   // for(auto h: hists)
   //     delete h;
   // hists.clear();


   cout<<"About to process hists of piercing tracks"<<endl;
   auto c2 = processHists(hists_pierce, "p_");

   c1->SetTitle("All");
   c2->SetTitle("Cathode-Anode Piercers");

   // save all the histograms
   // open file to store the histograms
   auto outf = TFile::Open(Form("%shists_dedx_by_voxel_segment_weight.root", gOutpref.Data()),"recreate");
   outf->mkdir("all_tracks")->cd();
   for (auto h: hists)
       h->Write();
   outf->mkdir("piercing_tracks")->cd();
   for (auto h: hists_pierce)
       h->Write();

   outf->cd();
   c1 -> Write();
   c2 -> Write();

   // Close the output file
   outf->Close();
}


int process_weighed(const char* fname, const char* outpref = "")
{
    gOutpref = outpref;
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
		if (h->GetEntries() < 10) continue;

		double maxbin = h -> GetMaximumBin();
		double maxdedx = h -> GetBinCenter(maxbin);
		// cout <<"About to do fit "<<ix <<", "<<iy<<", "<<iz<<endl;
		int status = h -> Fit("landau", "LQ0", "", maxdedx - 0.5, maxdedx + 1);
		if (status) // fit not succesful, try again
		    h -> Fit("landau", "LQ0M", "", maxdedx - 0.3, maxdedx + 0.8);
		auto fit = h -> GetFunction("landau");
		// reset the drawing bit
		fit->SetBit(TF1::kNotDraw, false);

		double mpv = fit->GetParameter(1);
		double mpv_err = fit->GetParError(1);

		if ( mpv_err < 0.1 ) {
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

    hout->SetAxisRange(2., 2.06, "z");

    return hout;
}
