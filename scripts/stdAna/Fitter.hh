#ifndef FITTER_HH
#define FITTER_HH


class Fitter
{
public:
    Fitter() {};

public:
    static TGraphErrors* sliceAndFit(TH2* h);
    static void Fit(TGraphErrors* gr);
    static TFitResultPtr FitLangau(TH1* h, double &mpv);

private:
    static Double_t langaufun(Double_t *x, Double_t *par);
    static int getMpv(TH1* h, double& mpv, double& err);

public:
    constexpr static const double gXtoT = 1./160.563; // converts cm to ms, calculated for field 0.5kV/cm and temperature 87K
    constexpr static const double TPC_Z_SIZE = 232.39;

    static int nboundaries;
    static const double* boundaries;

    static int gDoMPV;

};



#endif

#ifdef FITTER_CXX

#ifndef FOR
#define FOR(i, size) for (int i = 0; i < size; ++i)
#endif

const double* Fitter::boundaries = (const double[]){-726.7681, -363.38405, 0., 363.38405, 726.7681};
int Fitter::nboundaries = 4;

int Fitter::gDoMPV = 0;


TGraphErrors* Fitter::sliceAndFit(TH2* h)
{
    cout<<"N boundaries: "<<nboundaries<<endl;
    FOR(i, nboundaries) {
	cout<<"  "<<boundaries[i]<<endl;
    }

    auto n = h->GetNbinsX();
    cout<<"Hist "<<h->GetName()<<" with "<<n<<" bins on X axis"<<endl;

    vector<double> vx, vy, vy_err;

    auto top_dir = (TDirectory*)gDirectory;

    for (int i = 0; i < n; ++i) {
	// int perquarter = n/4;
	// if (i%perquarter > 30) continue;

	// skip this bin if too close to the boundaries (APA/CPA)
	auto x = h->GetXaxis()->GetBinCenter(i+1);
	double tmpxlow = h->GetXaxis()->GetBinLowEdge(i+1);;
	double tmpxup = h->GetXaxis()->GetBinUpEdge(i+1);;
	if (tmpxlow < boundaries[0] || tmpxup > boundaries[nboundaries-1]) {
	    //cout<<i<<"-th bin is out of range: low = "<<tmpxlow<<", up = "<<tmpxup<<", skipping."<<endl;
	    continue;
	}
	int too_close = 0;
	int segment = -1;
	FOR(iboundary, nboundaries) {
	    auto boundary = boundaries[iboundary];
	    if (tmpxlow > boundary) {
		segment++;
		continue;
	    }
	    if ( tmpxup > boundary ) {
		too_close = 1;
		break;
	    }
	}
	if (too_close) continue; // bin crosses boundary

	auto tmph = h->ProjectionY(Form("%s_%d",h->GetName(), i), i+1, i+1);
	double low = h->GetXaxis()->GetBinLowEdge(i+1);
	double hi = h->GetXaxis()->GetBinLowEdge(i+2);
	tmph->SetTitle( Form("x in %.0f cm to %.0f cm;dQ/dx [ADC/cm]", low, hi) );
	// set local range around expected dQ/dx value = ~300
	tmph->SetAxisRange(100,450);

	//cout<<"Created slice "<<i<<" from x = "<<low<<" cm to "<<hi<<" cm, with "<<tmph->GetEntries()<<" entries."<<endl;

	double mpv, mpverr;
	getMpv(tmph, mpv, mpverr);

	//cout<<"After the slice fit, MPV = "<<mpv<<"."<<endl;
	vx.push_back(x);
	vy.push_back(mpv);
	vy_err.push_back(mpverr);


	// store the Landau*Gauss distribution and fit into the output file
	// Create segment subdirectory based on bin number
	TString segment_name = Form("segment%d",segment);
	auto segment_dir = top_dir->GetDirectory(segment_name);
	if (!segment_dir)
	    segment_dir = top_dir->mkdir(segment_name);
	segment_dir->cd();
	tmph->Write();
	top_dir->cd();

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

void Fitter::Fit(TGraphErrors* gr)
{
    // loop over multiple tpc boundaries
    int timeplot = 0;
    if ( strstr(gr->GetName(), "vs_t") )
	timeplot = 1;

    cout<<gr->GetName()<<": "<<endl;

    for (int i = 0; i < nboundaries - 1; ++i) {
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

int Fitter::getMpv(TH1* h, double& mpv, double& err)
{
    auto result = FitLangau(h, mpv);
    err = result->ParError(1);
    return 1;
}

TFitResultPtr Fitter::FitLangau(TH1* h, double &mpv)
{
    h->SetAxisRange(100,450);
    // make rebinned copy of the histogram to make faster peak search and a pre-fit
    auto hcopy = (TH1*)h->Clone("tmpclone");
    const int REBIN = 4;
    hcopy->Rebin(REBIN);
    hcopy->SetAxisRange(100,450);

    // first find local maximum with largest dQ/dx
    //   - this is sensitive to secondary peaks due to fluctuations
    auto sp = new TSpectrum(4);
    int npeaks = sp->Search(hcopy, 3, "nobackground", 0.4);
    int maxbin = hcopy->GetMaximumBin();
    double maxloc = hcopy->GetBinCenter(maxbin);
    FOR(i, npeaks) {
	if (sp->GetPositionX()[i] > maxloc) {
	    maxloc = sp->GetPositionX()[i];
	    maxbin = hcopy->FindBin(maxloc);
	}
    }

    //fit = new TF1("gaus", "gaus");

    double dlow = 0.6; //0.6; //30.;
    double dhi = 0.5; //0.25; //70.;
    double low, hi;

    // find lower and upper bounds
    int nbins = hcopy->GetNbinsX();
    int binlow = maxbin - 1;
    double maxval = hcopy->GetBinContent(maxbin);
    for (; binlow > 0; --binlow) {
	if (hcopy->GetBinContent(binlow) < maxval*dlow) break;
    }
    binlow++;
    int binhi = maxbin - 1;
    for (; binhi < nbins; ++binhi) {
	if (hcopy->GetBinContent(binhi) < maxval*dhi) break;
    }

    low = hcopy->GetBinLowEdge(binlow);
    hi = hcopy->GetBinLowEdge(binhi);
    // low = maxloc - dlow;
    // hi = maxloc + dhi;

    auto fit = new TF1("langaus", langaufun, 0., 500., 4);
    fit->SetParNames("Width","MP","Area","GSigma");

    double norm = hcopy->Integral(binlow, binhi) * hcopy->GetBinWidth(1) * 1.5;
    double sv[4] = {10., 400., 300000., 10.}; // starting values for parameters: Landau scale, Landau MPV, Norm, Gauss sigma
    sv[1] = maxloc;
    sv[2] = norm;
    fit->SetParameters(sv);

    // set limits to Gaussian and Landau widths
    fit->SetParLimits(0, 5., 30.);
    fit->SetParLimits(3, 5., 30.);

    //fit = new TF1("landau", "landau");

    ///>>>> Do the fit <<<<

    //return 0;
    // do a faster prefit - make a rebinned histogram
    auto result = hcopy->Fit(fit, "SQ", "", low, hi);

    //gPad->SaveAs("prefit.pdf");

    if (result->IsValid()) {
    	// recalculate fit range
    	double mean = fit->GetParameter(1);
    	double sigma0 = fit->GetParameter(0);
    	double sigma3 = fit->GetParameter(3);

    	double sigma = sigma0*sigma0 + sigma3*sigma3;
    	sigma = sqrt(sigma);
    	// cout<<"Landau sigma: "<<sigma0<<endl
    	// 	<<"Gauss  sigma: "<<sigma3<<endl
    	// 	<<"total  width: "<<sigma<<endl;

    	low = mean - 2.*sigma;
    	hi = mean + 5.*sigma;
    }
    else {
    	fit->SetParameters(sv);
    }
    delete hcopy;

    // account for different bin widths in the original histogram and the copy
    fit->SetParameter(2, fit->GetParameter(2)/REBIN);

    result = h->Fit(fit, "SLQ", "", low, hi);
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

Double_t Fitter::langaufun(Double_t *x, Double_t *par)
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


#endif // FITTER_CXX
