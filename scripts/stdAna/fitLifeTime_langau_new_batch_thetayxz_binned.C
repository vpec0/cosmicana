#define FOR(i,size) for (int i = 0; i < size; ++i)

//#include "common.icc"

TGraphErrors*	sliceAndFit(TH2* h);
void		Fit(TGraphErrors* gr);
int		getMpv(TH1* h, double& mpv, double& err);
TFitResultPtr	FitLangau(TH1* h, double& mpv);
Double_t	langaufun(Double_t *x, Double_t *par);

const double gXtoT = 1./160.563; // converts cm to ms, calculated for field 0.5kV/cm and temperature 87K

const double boundaries[] = {-726.7681, -363.38405, 0., 363.38405, 726.7681};
const double TPC_Z_SIZE = 232.39;
const double drift_distance = 362;

int gDoMPV = 0;

// spcify tree entries
#define TREE_LIST				\
    INT(batchNo);				\
    INT(batchPart);				\
    DOUBLE_ARRAY(tau_e);			\
    DOUBLE_ARRAY(tau_e_error);			\
    DOUBLE_ARRAY(slope);			\
    DOUBLE_ARRAY(slope_error);			\
    DOUBLE_ARRAY(qcqa);				\
    DOUBLE_ARRAY(qcqa_error);			\
    DOUBLE_ARRAY(chi2);				\
    DOUBLE_ARRAY(ndf);				\

#define INT(name) int name
// binned in plane, angle, segment along x
#define DOUBLE_ARRAY(name) double name[3][4][4]
struct TauEevent_t {
    TREE_LIST;
};
#undef INT
#undef DOUBLE_ARRAY

TTree* createNewTree(const char* treename);
void registerTree(TTree* tree, TauEevent_t* evt);


void
fitLifeTime_langau_new_batch_thetayxz_binned(int batchpart,
					     const char* outpref = "",
					     const char* base_dir = "plots/v08_34_00/elifetime/2m_track_cut",
					     const char* MPV = "",
					     const char* single_file = ""
					     )
{
    if (!strcmp("MPV", MPV)) // strings identical
	gDoMPV = 1;


    TString fname = Form("%s/part%d_dqdx_hists_thetayxz_binned.root",
			 base_dir, batchpart);
    if (strcmp("", single_file)) // given non-empty string
	fname = single_file;

    cout<<"Opening file "<<fname<<" for reading."<<endl;
    auto f = TFile::Open(fname, "read");

    // open an output file; will store the profile graphs into a
    // separate directory (batchno/batchpart/); will update tree with
    // fit results
    TString outfname = outpref;
    if (gDoMPV)
	outfname += "MPV_";
    outfname += "fits.root";
    cout<<"Will write data to "<<outfname<<endl;
    auto outf = TFile::Open(outfname, "update");
    const char* treename = "elifetime";
    auto tree = (TTree*)outf->Get(treename);

    TauEevent_t evt;

    if (!tree)
	tree = createNewTree(treename);

    registerTree(tree, &evt);

    // create directory structure
    TString dirname = Form("part%d", batchpart);
    if (!outf->GetDirectory(dirname))
	outf->mkdir(dirname);
    outf->cd(dirname);

    // loop over all planes and get corresponding histograms
    for (int iplane = 0; iplane < 3; ++iplane) {
	if (iplane != 2) continue; // stick to collection plane
	TString histname = Form("Dqdx_vs_x_plane%d", iplane);
	auto h3d = (TH3F*) f->Get(histname);

	// create plane directory
	auto top_dir = gDirectory;
	TString plane_dir_s = Form("plane%d", iplane);
	if (!top_dir->GetDirectory(plane_dir_s))
	    top_dir->mkdir(plane_dir_s);

	top_dir->cd(plane_dir_s);

	// Loop over direction bins
	FOR(ibin, 4) {
	    h3d->GetZaxis()->SetRange(ibin+1, ibin+1);
	    auto h = (TH2*)h3d->Project3D("yx");
	    h->SetName(Form("%s_%d", h3d->GetName(), ibin));

	    // rebin y axis (dQ/dx)
	    h->Rebin2D(4,2);
	    h->Write();

	    auto plane_dir = gDirectory;
	    TString bin_dir_s = Form("bin%d", ibin);
	    if (!plane_dir->GetDirectory(bin_dir_s))
		plane_dir->mkdir(bin_dir_s);

	    plane_dir->cd(bin_dir_s);
	    auto gr = sliceAndFit(h);
	    plane_dir->cd();

	    gr->Write("", TObject::kOverwrite);

	    // get fit exponential slope, convert to lifetime, store for each fit segment
	    for (int ifit = 0; ifit < 4; ++ifit){
		auto fit = gr->GetFunction(Form("fit%d", ifit));
		double slope = (ifit%2*2 -1)*fit->GetParameter(1);
		double slope_err = fit->GetParError(1);
		double lt = 1./slope * gXtoT;
		double lt_err = slope_err * lt * lt / gXtoT;

		evt.tau_e[iplane][ibin][ifit] = lt;
		evt.tau_e_error[iplane][ibin][ifit] = lt_err;

		evt.slope[iplane][ibin][ifit] = slope/gXtoT;
		evt.slope_error[iplane][ibin][ifit] = slope_err/gXtoT;


		double qcqa = TMath::Exp(-slope*drift_distance);
		double qcqa_err = drift_distance*qcqa * slope_err;

		evt.qcqa[iplane][ibin][ifit] = qcqa;
		evt.qcqa_error[iplane][ibin][ifit] = qcqa_err;

		evt.chi2[iplane][ibin][ifit] = fit->GetChisquare();
		evt.ndf[iplane][ibin][ifit] = fit->GetNDF();
	    }
	    delete h;
	} // angle bin loop
    } // plane loop
    outf->cd();

    evt.batchNo = 0;
    evt.batchPart = batchpart;
    tree->Fill();


    tree->Write(0, TObject::kOverwrite);
    outf->Close();
}

TTree* createNewTree(const char* treename)
{
    auto tree = new TTree(treename, "");

#define INT(name) tree->Branch(#name, 0, #name"/I");
#define DOUBLE_ARRAY(name) tree->Branch(#name, 0, #name"[3][4][4]/D");
    TREE_LIST;
#undef INT
#undef DOUBLE_ARRAY

    return tree;
}


void registerTree(TTree* tree, TauEevent_t* evt)
{
#define INT(name) tree->SetBranchAddress(#name, &evt->name);
#define DOUBLE_ARRAY(name) tree->SetBranchAddress(#name, evt->name);
    TREE_LIST;
#undef INT
#undef DOUBLE_ARRAY
}

TGraphErrors* sliceAndFit(TH2* h)
{
    auto n = h->GetNbinsX();
    //auto gr = new TGraphErrors(n);
    //cout<<"Hist "<<h->GetName()<<" with "<<n<<" bins on X axis"<<endl;
    vector<double> vx, vy, vy_err;


    for (int i = 0; i < n; ++i) {
	// int perquarter = n/4;
	// if (i%perquarter > 30) continue;

	// skip this bin if too close to the boundaries (APA/CPA)
	auto x = h->GetXaxis()->GetBinCenter(i+1);
	double tmpxlow = h->GetXaxis()->GetBinLowEdge(i+1);;
	double tmpxup = h->GetXaxis()->GetBinUpEdge(i+1);;
	if (tmpxlow < boundaries[0] || tmpxup > boundaries[4]) continue;
	int too_close = 0;
	int segment = -1;
	for (auto boundary: boundaries) {
	    if (tmpxlow > boundary) {
		segment++;
		continue;
	    }
	    if ( tmpxup > boundary ) {
		too_close = 1;
		break;
	    }
	}
	if (too_close) continue;

	auto tmph = h->ProjectionY(Form("%s_%d",h->GetName(), i), i+1, i+1);
	double low = h->GetXaxis()->GetBinLowEdge(i+1);
	double hi = h->GetXaxis()->GetBinLowEdge(i+2);
	tmph->SetTitle( Form("x in %.0f cm to %.0f cm;dQ/dx [ADC/cm]", low, hi) );
	// set local range around expected dQ/dx value = ~300
	tmph->SetAxisRange(100,450);

	double mpv, mpverr;
	getMpv(tmph, mpv, mpverr);
	vx.push_back(x);
	vy.push_back(mpv);
	vy_err.push_back(mpverr);


	// store the Landau*Gauss distribution and fit into the output file
	// Create segment subdirectory based on bin number
	auto bin_dir = gDirectory;
	TString segment_name = Form("segment%d",segment);
	auto segment_dir = bin_dir->GetDirectory(segment_name);
	if (!segment_dir)
	    segment_dir = bin_dir->mkdir(segment_name);
	segment_dir->cd();
	tmph->Write();
	bin_dir->cd();

	delete tmph;
	// // plot one projection
	// if (i == 29 || i == 50 || i == 85) {
	//     auto pad = new TCanvas("temp", "", 400,400);
	//     tmph->Draw();
	//     pad->SaveAs(Form("%sexample_slice_%d_%s.pdf", outpref, i, h->GetName()));
	// }

	//tmph->Write();
    }
    auto gr = new TGraphErrors(vx.size(), vx.data(), vy.data(), 0, vy_err.data());

    gr->SetName(Form("%s_profile",h->GetName()));
    gr->SetMarkerSize(0.1);
    h->GetListOfFunctions()->Add(gr, "P");

    Fit(gr);


    return gr;
    //gr->Write();
}

void Fit(TGraphErrors* gr)
{
    // loop over multiple tpc boundaries
    int timeplot = 0;
    if ( strstr(gr->GetName(), "vs_t") )
	timeplot = 1;

    cout<<gr->GetName()<<": "<<endl;

    for (int i = 0; i < 4; ++i) {
	double low = boundaries[i] + 20.; // allow for a margin around APC/CPA centre location
	double hi = boundaries[i+1] - 20.;
	if (timeplot) {
	    hi *= gXtoT;
	    low *= gXtoT;
	}
	auto results = gr->Fit("expo", "SQ+", "", low, hi);
	auto fit = gr->GetFunction("expo");
	if (fit)
	    fit->SetName(Form("fit%d", i));
	double slope = results->Parameter(1);
	double slope_err = results->ParError(1);

	cout<<"    "<<slope<<" pm "<<slope_err<<endl;
    }

    for (auto fit : *gr->GetListOfFunctions()) {
	if(fit->InheritsFrom(TF1::Class()))
	    ((TF1*)fit)->SetLineColor(kTeal);
	    ((TF1*)fit)->SetLineWidth(1);
    }

}

int getMpv(TH1* h, double& mpv, double& err)
{
    auto result = FitLangau(h, mpv);
    err = result->ParError(1);
    return 1;
}



TFitResultPtr FitLangau(TH1* h, double &mpv)
{
    // first find local maximum with largest dQ/dx
    auto sp = new TSpectrum(4);
    h->SetAxisRange(100,450);
    int npeaks = sp->Search(h, 2, "nobackground", 0.3);
    int maxbin = h->GetMaximumBin();
    double maxloc = h->GetBinCenter(maxbin);
    FOR(i, npeaks) {
	if (sp->GetPositionX()[i] > maxloc) {
	    maxloc = sp->GetPositionX()[i];
	    maxbin = h->FindBin(maxloc);
	}
    }

    //fit = new TF1("gaus", "gaus");

    double dlow = 0.6; //30.;
    double dhi = 0.25; //70.;
    double low, hi;

    // find lower and upper bounds
    int nbins = h->GetNbinsX();
    int binlow = maxbin - 1;
    double maxval = h->GetBinContent(maxbin);
    for (; binlow > 0; --binlow) {
	if (h->GetBinContent(binlow) < maxval*dlow) break;
    }
    binlow++;
    int binhi = maxbin - 1;
    for (; binhi < nbins; ++binhi) {
	if (h->GetBinContent(binhi) < maxval*dhi) break;
    }

    low = h->GetBinLowEdge(binlow);
    hi = h->GetBinLowEdge(binhi);
    // low = maxloc - dlow;
    // hi = maxloc + dhi;

    auto fit = new TF1("langaus", langaufun, 0., 500., 4);
    fit->SetParNames("Width","MP","Area","GSigma");

    double norm = h->Integral(binlow, binhi) * h->GetBinWidth(1) * 1.5;
    double sv[4] = {10., 400., 300000., 10.}; // starting values for parameters: Landau scale, Landau MPV, Norm, Gauss sigma
    sv[1] = maxloc;
    sv[2] = norm;
    fit->SetParameters(sv);

    // set limits to Gaussian and Landau widths
    fit->SetParLimits(0, 5., 30.);
    fit->SetParLimits(3, 5., 30.);

    //fit = new TF1("landau", "landau");

    ///>>>> Do the fit <<<<
    auto result = h->Fit(fit, "SQ", "", low, hi);
    if (!gDoMPV)
	mpv = fit->GetMaximumX(result->Parameter(1), result->Parameter(1) + result->Parameter(3));
    else
	mpv = result->Parameter(1);

    //cout<<"chi2/ndf = "<<result->Chi2()<<"/"<<result->Ndf()<<endl;

    /// draw position of mpv
    double ymin = h->GetMinimum();
    double ymax = h->GetMaximum();
    auto l =  new TLine(mpv, ymin, mpv, ymax);
    l->SetLineColor(kRed-9);
    l->SetLineWidth(2);
    h->GetListOfFunctions()->Add(l);

    return result;
}



Double_t langaufun(Double_t *x, Double_t *par)
{
    //Fit parameters:
    //par[0]=Width (scale) parameter of Landau density
    //par[1]=Most Probable (MP, location) parameter of Landau density
    //par[2]=Total area (integral -inf to inf, normalization constant)
    //par[3]=Width (sigma) of convoluted Gaussian function
    //
    //In the Landau distribution (represented by the CERNLIB approximation),
    //the maximum is located at x=-0.22278298 with the location parameter=0.
    //This shift is corrected within this function, so that the actual
    //maximum is identical to the MP parameter.

    // Numeric constants
    Double_t invsq2pi = 0.3989422804014;   // (2 pi)^(-1/2)
    Double_t mpshift  = -0.22278298;       // Landau maximum location

    // Control constants
    Double_t np = 100.0;      // number of convolution steps
    Double_t sc =   5.0;      // convolution extends to +-sc Gaussian sigmas

    // Variables
    Double_t xx;
    Double_t mpc;
    Double_t fland;
    Double_t sum = 0.0;
    Double_t xlow,xupp;
    Double_t step;
    Double_t i;


    // MP shift correction
    mpc = par[1] - mpshift * par[0];

    // Range of convolution integral
    xlow = x[0] - sc * par[3];
    xupp = x[0] + sc * par[3];

    step = (xupp-xlow) / np;

    // Convolution integral of Landau and Gaussian by sum
    for(i=1.0; i<=np/2; i++) {
	xx = xlow + (i-.5) * step;
	fland = TMath::Landau(xx,mpc,par[0]) / par[0];
	sum += fland * TMath::Gaus(x[0],xx,par[3]);

	xx = xupp - (i-.5) * step;
	fland = TMath::Landau(xx,mpc,par[0]) / par[0];
	sum += fland * TMath::Gaus(x[0],xx,par[3]);
    }

    return (par[2] * step * sum * invsq2pi / par[3]);

}
