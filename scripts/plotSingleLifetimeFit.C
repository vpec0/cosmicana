void plotSingleLifetimeFit()
{
    gROOT->ForceStyle(1);

    auto f = TFile::Open("plots/v08_34_00/elifetime/batch_20002100_fits.root", "read");

    auto gr = (TGraphErrors*) f->Get("20002100/part0/Dqdx_vs_x_plane2_profile");

    gStyle->SetOptStat(0);
    gStyle->SetLineWidth(2);
    gStyle->SetLineColor(kRed);
    gStyle->SetMarkerSize(1);

    gr->SetTitle("dQ/dx vs x plane 2;x [cm];dQ/dx [ADC/cm]");

    auto c = new TCanvas("c","", 1);
    gr->Draw("apez");

    gr->SetMarkerSize(0.7);
    gr->SetLineWidth(3);
    gr->SetLineColor(kBlack);

    for (auto fit: *gr->GetListOfFunctions()) {
	if (strstr(fit->GetName(), "fit")) {
	    auto ff = (TF1*)fit;
	    ff->SetLineColor(kRed);
	    ff->SetLineWidth(2);
	    cout<<"Slope: "<<ff->GetParameter(1)<<endl
		<<"Error: "<<ff->GetParError(1)<<endl
		<<"Rel.Error: "<<ff->GetParError(1)/ff->GetParameter(1)<<endl
		<<"Chi2/ndf: "<<ff->GetChisquare()
		<<"/"<<ff->GetNDF()<<endl;
	}
    }


    c->Print("plots/v08_34_00/elifetime/elifetime_examplefit_plane2_20002100_part0.pdf");
}
