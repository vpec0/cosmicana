
void sliceAndFit(TH2* h, const char* outpref);
void fit(TGraphErrors* gr);
int getMpv(TH1* h, double& mpv, double& err);
TFitResultPtr FitLangau(TH1* h, double& mpv);
Double_t langaufun(Double_t *x, Double_t *par);

const double gXtoT = 1./160.563; // converts cm to ms, calculated for field 0.5kV/cm and temperature 87K


void fitLifeTime_langau(const char* fname, const char* outpref)
{
    gROOT->ForceStyle(1);
    gStyle->SetOptStat(0);
    gStyle->SetHistMinimumZero(1);
    gStyle->SetHistTopMargin(0.15);
    gStyle->SetPadTopMargin(0.15);
    gStyle->SetPadBottomMargin(0.15);
    gStyle->SetPadRightMargin(0.15);
    gStyle->SetPadLeftMargin(0.15);
    gStyle->SetTitleOffset(1.2, "x");
    gStyle->SetTitleOffset(1.3, "y");
    gStyle->SetTitleY(0.96);
    gStyle->SetNdivisions(508);


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


    vector<TString> histnames = {
	"Dqdx_vs_x",
	"Dqdx_vs_x_plane0",
	"Dqdx_vs_x_plane1",
	"Dqdx_vs_x_plane2",
	"Dqdx_vs_t",
	"Dqdx_vs_t_plane0",
	"Dqdx_vs_t_plane1",
	"Dqdx_vs_t_plane2",
    };

    auto f = TFile::Open(fname, "read");

    auto outf = TFile::Open(Form("%sfits.root", outpref), "recreate"); // to store all the langau fits, lifetime profile graphs, and the final fit

    auto c = new TCanvas("lifetime_fits","", 1200, 600);
    c->Divide(4,2);

    int ipad = 1;
    vector<TH2*> hists;

    for (auto hname : histnames ) {
	if (hists.size() >= 4) break; // process only "vs x" histograms, don't care about "vs t"
	outf->mkdir(Form("%s_dir", hname.Data()))->cd();

	auto h = (TH2*) f->Get(hname);

	c->cd(ipad);
	h->Rebin2D(1,2);
	h->SetAxisRange(0.,450., "y");
	sliceAndFit(h, outpref);
	h->Draw("colz");

	h->Write(Form("%s_wfit",h->GetName()), TObject::kOverwrite);

	hists.push_back(h);

	ipad++;
	auto pad = new TCanvas("temp","",800,600);
	h->Draw("colz hist");
	pad->Print(Form("%s%s_%s.png", outpref, c->GetName(), hname.Data()));
    }
    outf->cd();

    c->SaveAs(Form("%s%s_all.png", outpref, c->GetName()));

    // for (auto h : hists)
    // 	h->Rebin2D(4,2);
    // c->SaveAs(Form("%s%s.pdf", outpref, c->GetName()));

    // save results of expo fits into a graph
    c = new TCanvas("slopes","", 2);

    double mean = 0.;
    vector<double> x, y, y_err;
    int igr = 0;
    for (auto h : hists) {
	TGraph* gr = 0;
	for(auto obj: *h->GetListOfFunctions()) {
	    if (obj->InheritsFrom(TGraph::Class())) {
		gr = (TGraph*) obj;
		break;
	    }
	}
	for (auto fit: *gr->GetListOfFunctions()) {
	    if(fit->InheritsFrom(TF1::Class())) {
		double par = ((TF1*)fit)->GetParameter(1);
		double par_err = ((TF1*)fit)->GetParError(1);
		if (y.size()<16) {
		    par /= gXtoT;
		    par_err /= gXtoT;
		}
		par = 1./abs(par);
		par_err = par_err * par * par;

		mean += par;

		y.push_back(par);
		y_err.push_back(par_err);
	    }
	}

	// Draw the graph
	auto pad = new TCanvas("temp","",800,600);
	gr->SetMarkerSize(1);
	for (auto fit: *gr->GetListOfFunctions()) {
	    if(fit->InheritsFrom(TF1::Class())) {
		((TF1*)fit)->SetLineWidth(0);
	    }
	}
	if (igr < 4)
	    gr->SetTitle(";x [cm];dQ/dx MPV");
	else
	    gr->SetTitle(";t [ms];dQ/dx MPV");
	gr->Draw("AP");
	pad->Print(Form("%slifetime_fits_gr_%s.pdf", outpref, h->GetName()));

	// draw with the fits
	for (auto fit: *gr->GetListOfFunctions()) {
	    if(fit->InheritsFrom(TF1::Class())) {
		((TF1*)fit)->SetLineWidth(2);
		((TF1*)fit)->SetLineColor(kRed);
	    }
	}
	gr->Draw("AP");
	pad->Print(Form("%slifetime_fits_gr_wfits_%s.pdf", outpref, h->GetName()));

	//gr->Write();

	igr++;
    }

    mean /= y.size();

    cout<<y.size()<<endl;

    cout<<"Mean life time: "<<mean<<endl;

    int i = 0;
    for (auto pt: y)
	x.push_back(i++);
    auto gr = new TGraphErrors(x.size(), x.data(), y.data(), 0, y_err.data());
    gr->SetNameTitle("AllFitsLifeTimes","Electron lifetime;Fit index;#tau [ms]");
    c->cd();
    gr->Draw("AP");
    c->Update();

    gr->SetMaximum(3.05);
    gr->SetMinimum(2.65);

    // draw expected coef line
    double expected = 3.; // input lifetime is 3 ms; need to double check
    auto line = new TLine(gr->GetXaxis()->GetXmin(), expected,
			  gr->GetXaxis()->GetXmax(), expected);
    line->SetLineColor(kRed);
    line->Draw();

    auto leg = new TLegend(0.3, 0.78, 0.7, 0.84);
    leg->AddEntry(line,"Expected @ 3 ms", "L");
    leg->Draw();

    c->SaveAs(Form("%s%s.pdf", outpref, c->GetName()));


    gr->Write();


    // save corrected plots
    for (auto hname : histnames ) {
	auto h = (TH2*) f->Get(Form("%s_corr", hname.Data()));
	h->Rebin2D(1,2);
	h->SetAxisRange(200.,450., "y");

	auto pad = new TCanvas("temp","",600,600);
	h->Draw("colz hist");
	pad->Print(Form("%slifetime_fits_%s.png", outpref, h->GetName()));
    }

    // save dE/dx plots
    for (auto hname : histnames ) {
	hname.ReplaceAll("Dqdx", "Dedx");
	auto h = (TH2*) f->Get(hname.Data());
	//h->Rebin2D(1,1);
	h->SetAxisRange(1.25,3., "y");

	auto pad = new TCanvas("temp","",600,600);
	h->Draw("colz hist");
	pad->Print(Form("%slifetime_fits_%s.png", outpref, h->GetName()));
    }


    outf->Close();
}


void sliceAndFit(TH2* h, const char* outpref)
{
    auto n = h->GetNbinsX();
    //auto gr = new TGraphErrors(n);
    //cout<<"Hist "<<h->GetName()<<" with "<<n<<" bins on X axis"<<endl;
    const double boundaries[] = {-726.7681, -363.38405, 0., 363.38405, 726.7681};
    vector<double> vx, vy, vy_err;


    for (int i = 0; i < n; ++i) {

	// skip this bin if too close to the boundaries (APA/CPA)
	auto x = h->GetXaxis()->GetBinCenter(i+1);
	double tmpx = x;
	if (strstr(h->GetName(), "vs_t"))
	    tmpx /= gXtoT;
	if (tmpx < boundaries[0] || tmpx > boundaries[4]) continue;
	int too_close = 0;
	for (auto boundary: boundaries) {
	    if ( abs(tmpx-boundary) < 20. ) {
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

	// plot one projection
	if (i == 29 || i == 50 || i == 85) {
	    auto pad = new TCanvas("temp", "", 400,400);
	    tmph->Draw();
	    pad->SaveAs(Form("%sexample_slice_%d_%s.pdf", outpref, i, h->GetName()));
	}

	tmph->Write();
    }
    auto gr = new TGraphErrors(vx.size(), vx.data(), vy.data(), 0, vy_err.data());

    gr->SetName(Form("%s_profile",h->GetName()));
    gr->SetMarkerSize(0.1);
    h->GetListOfFunctions()->Add(gr, "P");

    fit(gr);

    gr->Write();
}

void fit(TGraphErrors* gr)
{
    const double boundaries[] = {-726.7681, -363.38405, 0., 363.38405, 726.7681};
    const double TPC_Z_SIZE = 232.39;


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
    h->SetAxisRange(50,500);
    int maxbin = h->GetMaximumBin();
    double maxloc = h->GetBinCenter(maxbin);

    //fit = new TF1("gaus", "gaus");

    auto fit = new TF1("langaus", langaufun, 0., 500., 4);
    fit->SetParNames("Width","MP","Area","GSigma");
    double norm = h->GetEntries() * h->GetBinWidth(1);


    double sv[4] = {10., 400., 300000., 10.}; // starting values for parameters: Landau scale, Landau MPV, Norm, Gauss sigma
    sv[1] = maxloc;
    sv[2] = norm;
    fit->SetParameters(sv);

    //fit = new TF1("landau", "landau");

    double dlow = 0.2; //30.;
    double dhi = 0.2; //70.;
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

    /// Do the fit
    auto result = h->Fit(fit, "LSQ", "", low, hi);
    mpv = fit->GetMaximumX(result->Parameter(1), result->Parameter(1) + result->Parameter(3));
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
