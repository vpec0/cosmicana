
void
plotElifetimeResults(const char* dataversion="v08_34_00",
		     const char* cut="",
		     const char* prefix = "",
		     const char* suffix = "")
{
    //auto dataversion = "v08_50_00";
    TString base_dir = Form("plots/%s/elifetime/%s", dataversion, cut);
    TString fname = Form("%s/%sbatch_2000*00%s_fits.root",
			 base_dir.Data(), prefix, suffix);
    auto tree = new TChain("elifetime");

    tree->Add(fname);

    TH1* hists[3][4];
    TH1* hists_err[3][4];
    for (int i = 0; i < 3; i++) {
	for (int j = 0; j < 4; j++) {
	    hists[i][j] = new TH1F( Form("h%d_%d", i, j),
				    Form("Plane %d, Segment %d", i,j),
				    100, 2.5,3.5 );
	    tree->Draw(Form("tau_e[%d][%d]>>+h%d_%d", i, j, i, j));

	    hists_err[i][j] = new TH1F( Form("herr%d_%d", i, j),
					Form("Plane %d, Segment %d", i,j),
					200, 0., .01 );
	    tree->Draw(Form("tau_e_error[%d][%d]>>+herr%d_%d", i, j, i, j));
	}
    }

    vector<double> vx, vmean, vmean_err, vstddev, vstddev_err;
    vector<TString> vlabels;

    for (int i = 0; i < 3; i++) {
	cout<<"Plane: "<<i<<endl;
	for (int j = 0; j < 4; j++) {
	    double mean = hists[i][j]->GetMean();
	    double mean_err = hists[i][j]->GetMeanError();

	    double stddev = hists[i][j]->GetStdDev();
	    double stddev_err = hists[i][j]->GetStdDevError();

	    vmean.push_back(mean);
	    vmean_err.push_back(mean_err);
	    vstddev.push_back(stddev);
	    vstddev_err.push_back(stddev_err);

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

    hmean->LabelsOption("v");
    hmean->Draw("PEX0");
    c->SaveAs(Form("%s/%smean_lifetime%s_fits_%s.pdf",
		   base_dir.Data(), prefix, suffix, dataversion));

    hstddev->LabelsOption("v");
    hstddev->Draw("PEX0");
    c->SaveAs(Form("%s/%sstddev_lifetime%s_fits_%s.pdf",
		   base_dir.Data(), prefix, suffix, dataversion));

    // write out all the cumulated histograms
    auto outf = TFile::Open(Form("%s/%slifetime%s_fits_result_hists_%s.root",
				 base_dir.Data(), prefix, suffix, dataversion),
			    "recreate");

    for (int i = 0; i < 3; i++) {
	for (int j = 0; j < 4; j++) {
	    hists[i][j]->Write();
	    hists_err[i][j]->Write();
	}
    }

    outf->Close();
}
