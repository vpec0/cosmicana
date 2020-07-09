
TGraphErrors* getRatio(TGraphErrors* gr);
TGraphErrors* getResid(TGraphErrors* gr);
TGraphErrors* getResidNorm(TGraphErrors* gr);
TGraphErrors* getResidSigmaNorm(TGraphErrors* gr);

void SetSameYrange(TGraph** gr); // expects 3 pointers to the graphs

TCanvas* createCanvas(const char* name, const char* title);


void
plotDataVsFit_expo(const char *fname, int batch, int part, const char* outpref = "")
{
    gROOT->ForceStyle(1);

    gStyle->SetTitleFont(43, "XY");
    gStyle->SetTitleSize(18, "XY");
    gStyle->SetTitleOffset(2, "Y");
    gStyle->SetTitleOffset(4, "X");

    gStyle->SetLabelFont(43, "XY");
    gStyle->SetLabelSize(16, "XY");
    gStyle->SetLabelOffset(0.015, "X");

    gStyle->SetNdivisions(505,"Y");

    gStyle->SetPadGridY(1);
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);


    TGaxis::SetMaxDigits(5);

    auto f = TFile::Open(fname, "read");
    TGraph* graphs[3];

    // plot and save original expo fit
    auto c = createCanvas("expo_fit", "Expo Fit");
    // loop over 3 planes
    for (int iplane = 0; iplane < 3; iplane++) {
	auto gr = (TGraphErrors*)f->Get(Form("%d/part%d/Dqdx_vs_x_plane%d_profile", batch, part, iplane));
	if (iplane==0)
	    gr->SetTitle(Form("Plane %d;;MPV [ADC]", iplane));
	if (iplane==2)
	    gr->SetTitle(";x [cm];");

	for (int i =0; i < 4; i++) {
	    auto fit = gr->GetFunction(Form("fit%d",i));
	    fit->SetLineColor(kRed);
	    fit->SetLineWidth(2);
	}

	c->cd(iplane+1);
	gr->Draw("AP");
	graphs[iplane] = gr;
    }
    SetSameYrange(graphs);
    c->SaveAs(Form("%s%s.pdf", outpref, c->GetName()));


    // draw ratio
    c = createCanvas("ratio", "Ratio");
    // loop over 3 planes
    for (int iplane = 0; iplane < 3; iplane++) {
	auto gr = (TGraphErrors*)f->Get(Form("%d/part%d/Dqdx_vs_x_plane%d_profile", batch, part, iplane));
	auto gr_ratio = getRatio(gr);

	if (iplane==0)
	    gr_ratio->SetTitle(Form("Plane %d;;Data/Fit", iplane));
	if (iplane==2)
	    gr_ratio->SetTitle(";x [cm];");

	c->cd(iplane+1);
	gr_ratio->Draw("AP");
	graphs[iplane] = gr_ratio;
    }
    SetSameYrange(graphs);
    c->SaveAs(Form("%s%s.pdf", outpref, c->GetName()));



    // draw residuals
    c = createCanvas("resid", "Residuals");
    // loop over 3 planes
    for (int iplane = 0; iplane < 3; iplane++) {
	auto gr = (TGraphErrors*)f->Get(Form("%d/part%d/Dqdx_vs_x_plane%d_profile", batch, part, iplane));
	auto gr_resid = getResid(gr);

	if (iplane==0)
	    gr_resid->SetTitle(Form("Plane %d;;Data-Fit [ADC]", iplane));
	if (iplane==2)
	    gr_resid->SetTitle(";x [cm];");

	c->cd(iplane+1);
	gr_resid->Draw("AP");
	graphs[iplane] = gr_resid;
    }
    SetSameYrange(graphs);
    c->SaveAs(Form("%s%s.pdf", outpref, c->GetName()));


    // draw normalized residuals
    c = createCanvas("residNorm", "Normalized Residuals");
    // loop over 3 planes
    for (int iplane = 0; iplane < 3; iplane++) {
	auto gr = (TGraphErrors*)f->Get(Form("%d/part%d/Dqdx_vs_x_plane%d_profile", batch, part, iplane));
	auto gr_resid = getResidNorm(gr);

	if (iplane==0)
	    gr_resid->SetTitle(Form("Plane %d;;(Data-Fit)/Fit", iplane));
	if (iplane==2)
	    gr_resid->SetTitle(";x [cm];");

	c->cd(iplane+1);
	gr_resid->Draw("AP");
	graphs[iplane] = gr_resid;
    }
    SetSameYrange(graphs);
    c->SaveAs(Form("%s%s.pdf", outpref, c->GetName()));


    // draw error-normalized residuals
    c = createCanvas("residSigmaNorm", "Error-Normalized Residuals");
    // loop over 3 planes
    for (int iplane = 0; iplane < 3; iplane++) {
	auto gr = (TGraphErrors*)f->Get(Form("%d/part%d/Dqdx_vs_x_plane%d_profile", batch, part, iplane));
	auto gr_resid = getResidSigmaNorm(gr);

	if (iplane==0)
	    gr_resid->SetTitle(Form("Plane %d;;(Data-Fit)/Error", iplane));
	if (iplane==2)
	    gr_resid->SetTitle(";x [cm];");

	c->cd(iplane+1);
	gr_resid->Draw("AP");
	graphs[iplane] = gr_resid;
    }
    SetSameYrange(graphs);
    c->SaveAs(Form("%s%s.pdf", outpref, c->GetName()));

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
