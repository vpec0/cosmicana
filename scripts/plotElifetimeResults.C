#define NOMINAL_LIFETIME 20.


void
plotElifetimeResults(const char* fname, const char* outpref)
// const char* dataversion="v08_34_00",
// 		     const char* cut="",
// 		     const char* prefix = "",
// 		     const char* suffix = "")
{
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

    hmean->LabelsOption("v");
    hmean->Draw("PEX0");
    c->SaveAs(Form("%smean_lifetime_fits.pdf", outpref));

    hstddev->LabelsOption("v");
    hstddev->Draw("PEX0");
    c->SaveAs(Form("%sstddev_lifetime_fits.pdf", outpref));

    hqcqamean->LabelsOption("v");
    hqcqamean->Draw("PEX0");
    c->SaveAs(Form("%smean_qcqa_fits.pdf", outpref));

    hqcqastddev->LabelsOption("v");
    hqcqastddev->Draw("PEX0");
    c->SaveAs(Form("%sstddev_qcqa_fits.pdf", outpref));

    // save versions without x-axis labels
    hmean->GetXaxis()->SetLabelSize(0);
    hmean->Draw("PEX0");
    c->SaveAs(Form("%smean_lifetime_fits_noxlabels.pdf", outpref));

    hstddev->GetXaxis()->SetLabelSize(0);
    hstddev->Draw("PEX0");
    c->SaveAs(Form("%sstddev_lifetime_fits_noxlabels.pdf", outpref));

    hqcqamean->GetXaxis()->SetLabelSize(0);
    hqcqamean->Draw("PEX0");
    c->SaveAs(Form("%smean_qcqa_fits_noxlabels.pdf", outpref));

    hqcqastddev->GetXaxis()->SetLabelSize(0);
    hqcqastddev->Draw("PEX0");
    c->SaveAs(Form("%sstddev_qcqa_fits_noxlabels.pdf", outpref));


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
