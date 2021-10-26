
void newFit(TGraph* gr);
TGraphErrors* getRatio(TGraphErrors* gr);
TGraphErrors* getResid(TGraphErrors* gr);
TGraphErrors* getResidNorm(TGraphErrors* gr);
TGraphErrors* getResidSigmaNorm(TGraphErrors* gr);

void SetSameYrange(TGraph** gr); // expects 3 pointers to the graphs

void treatFits(TGraphErrors* gr);
TCanvas* createCanvas(const char* name, const char* title);

const double gXtoT = 1./160.563; // converts cm to ms, calculated for field 0.5kV/cm and temperature 87K
const double boundaries[] = {-726.7681, -363.38405, 0., 363.38405, 726.7681};


#define FOR(i, size) for (int i = 0; i < size; ++i)

void
plotDataVsFit_expo_thetayxz_binned(const char *fname, int part, const char* outpref = "")
{
    gROOT->ForceStyle(1);

    // gStyle->SetTitleFont(43, "XY");
    // gStyle->SetTitleSize(18, "XY");
    // gStyle->SetTitleOffset(2, "Y");
    // gStyle->SetTitleOffset(4, "X");

    // gStyle->SetLabelFont(43, "XY");
    // gStyle->SetLabelSize(16, "XY");
    // gStyle->SetLabelOffset(0.015, "X");

    gStyle->SetNdivisions(505,"Y");

    gStyle->SetPadGridY(1);
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);


    TGaxis::SetMaxDigits(5);

    auto f = TFile::Open(fname, "read");
    TGraph* graphs[3][4] = {};
    TGraph* graphs_ratio[3][4] = {};
    TGraph* graphs_res[3][4] = {};
    TGraph* graphs_relres[3][4] = {};
    TGraph* graphs_relerrres[3][4] = {};

    // plot and save original expo fit
    auto c = new TCanvas("expo_fit","");//createCanvas("expo_fit", "Expo Fit");
    // loop over 3 planes
    for (int iplane = 0; iplane < 3; iplane++) {
	//c->cd(iplane+1);

	FOR(ibin, 4) {
	    auto gr = (TGraphErrors*)f->Get(Form("part%d/plane%d/Dqdx_vs_x_plane%d_%d_profile", part, iplane, iplane, ibin));
	    if (!gr) continue;

	    newFit(gr);

	    gr->SetTitle(Form("Plane %d Bin %d;x [cm];MPV [ADC]", iplane, ibin));

	    gr->Draw("AP");
	    treatFits(gr);

	    graphs[iplane][ibin] = gr;
	    c->SaveAs(Form("%s%s_plane%d_bin%d.pdf", outpref, c->GetName(), iplane, ibin));
	}
    }

	//SetSameYrange(graphs);

    // draw ratio
    // loop over 3 planes
    for (int iplane = 0; iplane < 3; iplane++) {
	FOR(ibin, 4) {
	    auto gr = (TGraphErrors*)graphs[iplane][ibin];
	    if (!gr) continue;
	    auto gr_ratio = getRatio(gr);

	    gr_ratio->SetTitle(Form("Plane %d Bin %d;x [cm];Data/Fit", iplane, ibin));

	    gr_ratio->Draw("AP");
	    graphs_ratio[iplane][ibin] = gr_ratio;
	    c->SaveAs(Form("%s%s_plane%d_bin%d_ratio.pdf", outpref, c->GetName(), iplane, ibin));
	}
    }
	//SetSameYrange(graphs);



    // draw residuals
    // loop over 3 planes
    for (int iplane = 0; iplane < 3; iplane++) {
	FOR(ibin, 4) {
	    auto gr = (TGraphErrors*)graphs[iplane][ibin];
	    if (!gr) continue;
	    auto gr_resid = getResid(gr);

	    gr_resid->SetTitle(Form("Plane %d;x [cm];Data-Fit [ADC]", iplane));

	    gr_resid->Draw("AP");
	    graphs_res[iplane][ibin] = gr_resid;
	    c->SaveAs(Form("%s%s_plane%d_bin%d_res.pdf", outpref, c->GetName(), iplane, ibin));
	}
    }


    // draw normalized residuals
    // loop over 3 planes
    for (int iplane = 0; iplane < 3; iplane++) {
	FOR(ibin, 4) {
	    auto gr = (TGraphErrors*)graphs[iplane][ibin];
	    if (!gr) continue;
	    auto gr_resid = getResidNorm(gr);

	    gr_resid->SetTitle(Form("Plane %d Bin %d;x [cm];(Data-Fit)/Fit", iplane, ibin));

	    gr_resid->Draw("AP");
	    graphs_relres[iplane][ibin] = gr_resid;
	    c->SaveAs(Form("%s%s_plane%d_bin%d_relres.pdf", outpref, c->GetName(), iplane, ibin));
	}
    }


    // draw error-normalized residuals
    // loop over 3 planes
    for (int iplane = 0; iplane < 3; iplane++) {
	FOR(ibin, 4) {
	    auto gr = (TGraphErrors*)graphs[iplane][ibin];
	    if (!gr) continue;
	    auto gr_resid = getResidSigmaNorm(gr);

	    gr_resid->SetTitle(Form("Plane %d Bin %d;x [cm];(Data-Fit)/Error", iplane, ibin));

	    gr_resid->Draw("AP");
	    graphs_relerrres[iplane][ibin] = gr_resid;
	    c->SaveAs(Form("%s%s_plane%d_bin%d_relerrres.pdf", outpref, c->GetName(), iplane, ibin));
	}
    }

}


void newFit(TGraph* gr)
{
    auto fitF = new TF1("expo", "[0] * TMath::Exp([1]*(x - [3])) + [2]");

    for (auto item: *gr->GetListOfFunctions()) {
	if(item->InheritsFrom(TF1::Class()))
	    gr->GetListOfFunctions()->Remove(item);
    }


    for (int i = 0; i < 4; ++i) {
	double low = boundaries[i] + 20.; // allow for a margin around APC/CPA centre location
	double hi = boundaries[i+1] - 20.;

	fitF->SetParameters(gr->Eval(low), (-1. + 2*(i%2))/200., 0., low);
	fitF->FixParameter(3, low);

	auto results = gr->Fit(fitF, "S+", "", low, hi);
	auto fit = gr->GetFunction("expo");
	if (fit)
	    fit->SetName(Form("fit%d", i));

	// double slope = results->Parameter(1);
	// double slope_err = results->ParError(1);

	// cout<<"    "<<slope<<" pm "<<slope_err<<endl;
    }

}


TGraphErrors* getRatio(TGraphErrors* gr)
{
    // get all 4 fits from the graph
    vector<TF1*> fits;
    for (int i = 0; i < 4; i++) {
	fits.push_back(gr->GetFunction(Form("fit%d", i)));
    }

    // get fit's lower range limits
    vector<double> limits;
    for (int i = 0; i < 4; i++) {
	limits.push_back(fits[i]->GetXmax());
    }


    // create ratio of data and fit
    auto limit = limits.begin();
    auto fit = fits.begin();
    auto gr_ratio = new TGraphErrors(gr->GetN());
    gr_ratio->SetNameTitle(gr->GetName(), gr->GetTitle());

    for (int i = 0; i < gr->GetN(); i++) {
	double x = gr->GetX()[i];
	if (x > *limit) {
	    fit++;
	    limit++;
	}

	double y = gr->GetY()[i];
	double y_err = gr->GetEY()[i];
	double fity = (*fit)->Eval(x);

	double ratio = y/fity;
	double ratio_err = y_err/fity;

	gr_ratio->SetPoint(i, x, ratio);
	gr_ratio->SetPointError(i, 0, ratio_err);
    }

    return gr_ratio;
}

TGraphErrors* getResid(TGraphErrors* gr)
{
    // get all 4 fits from the graph
    vector<TF1*> fits;
    for (int i = 0; i < 4; i++) {
	fits.push_back(gr->GetFunction(Form("fit%d", i)));
    }

    // get fit's lower range limits
    vector<double> limits;
    for (int i = 0; i < 4; i++) {
	limits.push_back(fits[i]->GetXmax());
    }


    // create resid of data and fit
    auto limit = limits.begin();
    auto fit = fits.begin();
    auto gr_resid = new TGraphErrors(gr->GetN());
    gr_resid->SetNameTitle(gr->GetName(), gr->GetTitle());

    for (int i = 0; i < gr->GetN(); i++) {
	double x = gr->GetX()[i];
	if (x > *limit) {
	    fit++;
	    limit++;
	}

	double y = gr->GetY()[i];
	double y_err = gr->GetEY()[i];
	double fity = (*fit)->Eval(x);

	double resid = y-fity;
	double resid_err = y_err;

	gr_resid->SetPoint(i, x, resid);
	gr_resid->SetPointError(i, 0, resid_err);
    }

    return gr_resid;
}

TGraphErrors* getResidNorm(TGraphErrors* gr)
{
    // get all 4 fits from the graph
    vector<TF1*> fits;
    for (int i = 0; i < 4; i++) {
	fits.push_back(gr->GetFunction(Form("fit%d", i)));
    }

    // get fit's lower range limits
    vector<double> limits;
    for (int i = 0; i < 4; i++) {
	limits.push_back(fits[i]->GetXmax());
    }


    // create resid of data and fit
    auto limit = limits.begin();
    auto fit = fits.begin();
    auto gr_resid = new TGraphErrors(gr->GetN());
    gr_resid->SetNameTitle(gr->GetName(), gr->GetTitle());

    for (int i = 0; i < gr->GetN(); i++) {
	double x = gr->GetX()[i];
	if (x > *limit) {
	    fit++;
	    limit++;
	}

	double y = gr->GetY()[i];
	double y_err = gr->GetEY()[i];
	double fity = (*fit)->Eval(x);

	double resid = (y-fity)/fity;
	double resid_err = y_err/fity;

	gr_resid->SetPoint(i, x, resid);
	gr_resid->SetPointError(i, 0, resid_err);
    }

    return gr_resid;
}

TGraphErrors* getResidSigmaNorm(TGraphErrors* gr)
{
    // get all 4 fits from the graph
    vector<TF1*> fits;
    for (int i = 0; i < 4; i++) {
	fits.push_back(gr->GetFunction(Form("fit%d", i)));
    }

    // get fit's lower range limits
    vector<double> limits;
    for (int i = 0; i < 4; i++) {
	limits.push_back(fits[i]->GetXmax());
    }


    // create resid of data and fit
    auto limit = limits.begin();
    auto fit = fits.begin();
    auto gr_resid = new TGraphErrors(gr->GetN());
    gr_resid->SetNameTitle(gr->GetName(), gr->GetTitle());

    for (int i = 0; i < gr->GetN(); i++) {
	double x = gr->GetX()[i];
	if (x > *limit) {
	    fit++;
	    limit++;
	}

	double y = gr->GetY()[i];
	double y_err = gr->GetEY()[i];
	double fity = (*fit)->Eval(x);

	double resid = (y-fity)/y_err;
	double resid_err = 0.;

	gr_resid->SetPoint(i, x, resid);
	gr_resid->SetPointError(i, 0, resid_err);
    }

    return gr_resid;
}

void SetSameYrange(TGraph** gr)
// expects 3 pointers to the graphs
{
    double min = 9999.;
    double max = -9999.;

    for (int i = 0; i < 3; i++) {
	auto axis = gr[i]->GetYaxis();
	if (min > axis->GetXmin())
	    min = axis->GetXmin();
	if (max < axis->GetXmax())
	    max = axis->GetXmax();
    }

    for (int i = 0; i < 3; i++) {
	auto axis = gr[i]->GetYaxis();
	axis->SetRangeUser(min, max);
    }

    gPad->Modified();
    gPad->Update();
}

void treatFits(TGraphErrors* gr)
{
    // make the line more visible
    for (int i =0; i < 4; i++) {
	auto fit = gr->GetFunction(Form("fit%d",i));
	fit->SetLineColor(kRed);
	fit->SetLineWidth(2);
    }

    // create fit labels
    for (int i =0; i < 4; i++) {
	auto fit = gr->GetFunction(Form("fit%d",i));
	double tau = 1./fit->GetParameter(1) * gXtoT;
	double tau_err = fit->GetParError(1) * tau * tau/ gXtoT;
	auto label = new TLatex(0.28+i*0.1475, 0.8*(1 - gPad->GetTopMargin()) + 0.2*gPad->GetBottomMargin(), Form("#splitline{%.3f}{#pm %.3f}", tau, tau_err));
	label->SetNDC();
	label->SetTextFont(43);
	label->SetTextSize(12);
	label->SetTextAlign(22);
	label->Draw();
    }


}

TCanvas* createCanvas(const char* name, const char* title)
{
    double  c_bottom_margin = 0.15;
    double c_top_margin = 0.15;
    double right_margin = 0.15;
    double left_margin = 0.15;
    double pad_h = (1. - c_top_margin - c_bottom_margin)/3.;
    double pad_overlap = 0.02; // overlap of 2 pads in parent canvas relative size

    double x1 = 0.;
    double x2 = 1.;
    double y[4] = {1.,
		  1.-pad_h-c_top_margin,
		  1.-2*pad_h-c_top_margin,
		  0.};
    double y_overlap[4] = {0., pad_overlap, pad_overlap, 0.};

    double top_margin[3] = {c_top_margin/(pad_h+c_top_margin+pad_overlap),
			    pad_overlap/(pad_h+2.*pad_overlap),
			    pad_overlap/(pad_h+c_bottom_margin+pad_overlap)};
    double bottom_margin[3] = {pad_overlap/(pad_h+c_top_margin+pad_overlap),
			       pad_overlap/(pad_h+2.*pad_overlap),
			       c_bottom_margin/(pad_h+c_bottom_margin+pad_overlap)};


    auto c = new TCanvas(name, title, 1);
    TPad* pad[3] = {};
    for (int i = 0; i < 3; i++) {
	pad[i] = new TPad(Form("%s_%d", c->GetName(), i+1),"", x1, y[i+1]-y_overlap[i+1], x2, y[i]+y_overlap[i]);
	pad[i]->SetNumber(i+1); pad[i]->Draw();
	pad[i]->SetTopMargin(top_margin[i]);
	pad[i]->SetBottomMargin(bottom_margin[i]);
	pad[i]->SetLeftMargin(left_margin);
	pad[i]->SetRightMargin(right_margin);
	pad[i]->SetFillStyle(4000);
    }

    return c;
}
