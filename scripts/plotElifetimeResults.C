static TString gOutpref;

enum {
    kWithLabels = 0,
    kNoLabels = 1
};

void draw_hist(TH1* h, TString hname, const int with_labels);
void draw_means(TH1* h);

void
plotElifetimeResults(const char* fname, const char* outpref, float NOMINAL_LIFETIME = 3.)
// const char* dataversion="v08_34_00",
// 		     const char* cut="",
// 		     const char* prefix = "",
// 		     const char* suffix = "")
{
    gOutpref = outpref;


    gStyle->SetNdivisions(505, "y");

    //auto dataversion = "v08_50_00";
    // TString base_dir = Form("plots/%s/no_lt_diffusion_100_jobs/elifetime/%s", dataversion, cut);
    // TString fname = Form("%s/%sbatch_*0%s_fits.root",
    // 			 base_dir.Data(), prefix, suffix);
    auto tree = new TChain("elifetime");

    tree->Add(fname);

    TH1* hists[3][4];
    TH1* hists_err[3][4];
    TH1* hists_qcqa[3][4];
    TH1* hists_qcqa_err[3][4];
    for (int i = 0; i < 3; i++) {
	for (int j = 0; j < 4; j++) {
	    hists[i][j] = new TH1F( Form("h%d_%d", i, j),
				    Form("Plane %d, Segment %d", i,j),
				    200, NOMINAL_LIFETIME*0.5, NOMINAL_LIFETIME*1.5);
	    tree->Draw(Form("tau_e[%d][%d]>>+h%d_%d", i, j, i, j));

	    hists_err[i][j] = new TH1F( Form("herr%d_%d", i, j),
					Form("Plane %d, Segment %d", i,j),
					200, 0., NOMINAL_LIFETIME*0.5 );
	    tree->Draw(Form("tau_e_error[%d][%d]>>+herr%d_%d", i, j, i, j));
	    hists_qcqa[i][j] = new TH1F( Form("hqcqa%d_%d", i, j),
				    Form("Plane %d, Segment %d", i,j),
				    200, 0, 1 );
	    tree->Draw(Form("qcqa[%d][%d]>>+hqcqa%d_%d", i, j, i, j));

	    hists_qcqa_err[i][j] = new TH1F( Form("hqcqaerr%d_%d", i, j),
					Form("Plane %d, Segment %d", i,j),
					200, 0., .1 );
	    tree->Draw(Form("qcqa_error[%d][%d]>>+hqcqaerr%d_%d", i, j, i, j));
	}
    }

    vector<double> vx, vmean, vmean_err, vstddev, vstddev_err, vqcqamean, vqcqamean_err, vqcqastddev, vqcqastddev_err;
    vector<TString> vlabels;

    for (int i = 0; i < 3; i++) {
	cout<<"Plane: "<<i<<endl;
	for (int j = 0; j < 4; j++) {
	    // tau_e
	    double mean = hists[i][j]->GetMean();
	    double mean_err = hists[i][j]->GetMeanError();

	    double stddev = hists[i][j]->GetStdDev();
	    double stddev_err = hists[i][j]->GetStdDevError();

	    vmean.push_back(mean);
	    vmean_err.push_back(mean_err);
	    vstddev.push_back(stddev);
	    vstddev_err.push_back(stddev_err);

	    // QC/QA
	    mean = hists_qcqa[i][j]->GetMean();
	    mean_err = hists_qcqa[i][j]->GetMeanError();
	    stddev = hists_qcqa[i][j]->GetStdDev();
	    stddev_err = hists_qcqa[i][j]->GetStdDevError();

	    vqcqamean.push_back(mean);
	    vqcqamean_err.push_back(mean_err);
	    vqcqastddev.push_back(stddev);
	    vqcqastddev_err.push_back(stddev_err);

	    vlabels.push_back(hists[i][j]->GetTitle());

	    cout<<"  Segment "<<j<<": ";
	    cout<<mean<<endl;
	}
    }


    gStyle->SetPadBottomMargin(0.5);
    gStyle->SetPadLeftMargin(0.15);
    gStyle->SetPadRightMargin(0.15);
    gStyle->SetTitleOffset(1.3,"y");
    gStyle->SetPadGridX(1);
    gStyle->SetPadGridY(1);
    gStyle->SetOptStat(0);
    gStyle->SetMarkerSize(1);
    gStyle->SetHistLineWidth(2);


    auto c = new TCanvas("c", "", 1);
    auto hmean = new TH1F("hmean",";;Mean e-lifetime [ms]", vmean.size(), 0., vmean.size());
    auto hstddev = new TH1F("hstddev",";;Std.deviation of e-lifetime [ms]", vstddev.size(), 0., vstddev.size());
    auto hqcqamean = new TH1F("hqcqamean",";;Mean Q_{C}/Q_{A}", vqcqamean.size(), 0., vqcqamean.size());
    auto hqcqastddev = new TH1F("hqcqastddev",";;Std.deviation of Q_{C}/Q_{A}", vqcqastddev.size(), 0., vqcqastddev.size());
    //h->SetMarkerStyle(kFullCircle);
    //h->SetMarkerSize(1.);
    //h->SetLineWidth(2.);
    auto ax_mean = hmean->GetXaxis();
    ax_mean->SetLabelSize(0.08);
    auto ax_stddev = hstddev->GetXaxis();
    ax_stddev->SetLabelSize(0.08);
    for (int i = 0; i < vmean.size(); i++) {
	hmean->SetBinContent(i+1, vmean.at(i));
	hmean->SetBinError(i+1, vmean_err.at(i));
	ax_mean->SetBinLabel(i+1 ,vlabels.at(i));

	hstddev->SetBinContent(i+1, vstddev.at(i));
	hstddev->SetBinError(i+1, vstddev_err.at(i));
	ax_stddev->SetBinLabel(i+1 ,vlabels.at(i));
    }

    // QCQA
    ax_mean = hqcqamean->GetXaxis();
    ax_mean->SetLabelSize(0.08);
    ax_stddev = hqcqastddev->GetXaxis();
    ax_stddev->SetLabelSize(0.08);
    for (int i = 0; i < vmean.size(); i++) {
	hqcqamean->SetBinContent(i+1, vqcqamean.at(i));
	hqcqamean->SetBinError(i+1, vqcqamean_err.at(i));
	ax_mean->SetBinLabel(i+1, vlabels.at(i));

	hqcqastddev->SetBinContent(i+1, vqcqastddev.at(i));
	hqcqastddev->SetBinError(i+1, vqcqastddev_err.at(i));
	ax_stddev->SetBinLabel(i+1, vlabels.at(i));
    }

    vector< pair<TH1*, TString> > to_draw;
    to_draw.push_back(make_pair(hmean,       "mean_lifetime"));
    to_draw.push_back(make_pair(hstddev,     "stddev_lifetime"));
    to_draw.push_back(make_pair(hqcqamean,   "mean_qcqa"));
    to_draw.push_back(make_pair(hqcqastddev, "stddev_qcqa"));

    for (auto hpair: to_draw) {
	draw_hist(hpair.first, hpair.second, kWithLabels);
	draw_hist(hpair.first, hpair.second, kNoLabels);
    }



    // write out all the cumulated histograms
    auto outf = TFile::Open(Form("%slifetime_fits_result_hists.root",outpref), "recreate");

    for (int i = 0; i < 3; i++) {
	for (int j = 0; j < 4; j++) {
	    hists[i][j]->Write();
	    hists_err[i][j]->Write();
	    hists_qcqa[i][j]->Write();
	    hists_qcqa_err[i][j]->Write();
	}
    }

    outf->Close();
}


void draw_hist(TH1* h, TString hname, const int with_labels)
{
    h->LabelsOption("v");
    if (with_labels == kNoLabels)
	h->GetXaxis()->SetLabelSize(0);

    h->Draw("PEX0");

    draw_means(h);

    if (with_labels == kWithLabels)
	gPad->SaveAs(Form("%s%s_fits.pdf", gOutpref.Data(), hname.Data()));
    else if (with_labels == kNoLabels)
	gPad->SaveAs(Form("%s%s_fits_noxlabels.pdf", gOutpref.Data(), hname.Data()));
}

void draw_means(TH1* h)
{
    int nbins = h->GetNbinsX();
    for (int i = 0; i < 3; ++i) {
	double mean = 0.;
	double weight = 0.;
	for (int j = 0; j < 4; ++j) {
	    mean += h->GetBinContent(i*4+j+1) / h->GetBinError(i*4+j+1);
	    weight += 1/h->GetBinError(i*4+j+1);
	}
	mean /= weight;
	double low = h->GetBinLowEdge(i*4+1);
	double hi = h->GetBinLowEdge((i+1)*4+1);
	auto line = new TLine(low, mean, hi, mean);
	line->SetLineColor(kRed);
	line->Draw();

	// draw the value and its error
	gPad->Update();
	// double up = h->GetYaxis()->GetXmax();
	double up = gPad->GetUymax();
	double down = gPad->GetUymin();

	double x = (low + hi) * 0.5;
	auto tt = new TText(x, up + 0.08*(up-down), Form("%.4g", mean));
	tt->SetTextAlign(21);
	tt->Draw();
    }
}
