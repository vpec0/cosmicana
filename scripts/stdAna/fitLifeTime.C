
void sliceAndFit(TH2* h, const char* outpref);
void fit(TGraph* gr);

const double gXtoT = 1./160.563; // converts cm to ms, calculated for field 0.5kV/cm and temperature 87K


void fitLifeTime(const char* fname, const char* outpref)
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

    auto f = TFile::Open(fname, "update");

    auto c = new TCanvas("lifetime_fits","", 1200, 600);
    c->Divide(4,2);

    int ipad = 1;
    TH2* hists[8] = {};

    for (auto hname : histnames ) {
	auto h = (TH2*) f->Get(hname);

	c->cd(ipad);
	h->Rebin2D(1,2);
	h->SetAxisRange(0.,450., "y");
	sliceAndFit(h, outpref);
	h->Draw("colz");

	h->Write(Form("%s_wfit",h->GetName()), TObject::kOverwrite);

	hists[ipad-1] = h;

	ipad++;
	auto pad = new TCanvas("temp","",800,600);
	h->Draw("colz hist");
	pad->Print(Form("%s%s_%s.png", outpref, c->GetName(), hname.Data()));
    }

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


	igr++;
    }

    mean /= y.size();

    cout<<y.size()<<endl;

    cout<<"Mean life time: "<<mean<<endl;

    int i = 0;
    for (auto pt: y)
	x.push_back(i++);
    auto gr = new TGraphErrors(x.size(), x.data(), y.data(), 0, y_err.data());
    gr->SetTitle("Electron lifetime;Fit index;#tau [ms]");
    c->cd();
    gr->Draw("AP");
    c->Update();

    gr->SetMaximum(3.05);
    gr->SetMinimum(2.75);

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


}


void sliceAndFit(TH2* h, const char* outpref)
{
    auto n = h->GetNbinsX();
    auto gr = new TGraph(n);

    //cout<<"Hist "<<h->GetName()<<" with "<<n<<" bins on X axis"<<endl;

    for (int i = 0; i < n; ++i) {
	auto tmph = h->ProjectionY("tmph", i+1, i+1);
	// set local range around expected dQ/dx value = ~300
	tmph->SetAxisRange(100,450);
	auto maxbin = tmph->GetMaximumBin();
	if (maxbin <= 0)
	    continue;
	auto mpv = tmph->GetBinCenter(maxbin);
	auto x = h->GetXaxis()->GetBinCenter(i+1);
	gr->SetPoint(i, x, mpv);


	// plot one projection
	if (i == 29 || i == 50 || i == 85) {
	    auto pad = new TCanvas("temp", "", 400,400);
	    double low = h->GetXaxis()->GetBinLowEdge(i+1);
	    double hi = h->GetXaxis()->GetBinLowEdge(i+2);
	    tmph->SetTitle( Form("x in %.0f cm to %.0f cm;dQ/dx [ADC/cm]", low, hi) );
	    tmph->Draw();
	    pad->SaveAs(Form("%sexample_slice_%d_%s.pdf", outpref, i, h->GetName()));
	}
    }

    gr->SetName(Form("%s_profile",h->GetName()));
    gr->SetMarkerSize(0.1);
    h->GetListOfFunctions()->Add(gr, "P");

    fit(gr);
}

void fit(TGraph* gr)
{
    const double boundaries[] = {-726.7681, -363.38405, 0., 363.38405, 726.7681};
    const double TPC_Z_SIZE = 232.39;


    // loop over multiple tpc boundaries
    int timeplot = 0;
    if ( strstr(gr->GetName(), "vs_t") )
	timeplot = 1;

    cout<<gr->GetName()<<": "<<endl;

    for (int i = 0; i < 4; ++i) {
	double low = boundaries[i] + 10; // allow for a margin around APC/CPA centre location
	double hi = boundaries[i+1] - 10;
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
