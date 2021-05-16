#define FOR(i, size) for (int i = 0; i < size; ++i)

void merge_meandqdx_vs_thetaxyz_hists()
{
    TString prefix = "plots/thetayxzbins/meandqdx/";
    TString outfname = "merged_meandqdx_vs_thetayxz_hists.root";

    TH3* result_dqdx[3] = {};
    TH3* result_nhits[3] = {};

    auto outf = TFile::Open(prefix + outfname, "recreate");

    FOR(i, 10) {
	TString fname = Form("batch_%d_meandqdx_vs_thetayxz_hists.root", 20003100 + i*100);
	auto f = TFile::Open(prefix+fname, "read");

	FOR(iplane, 3) {
	    auto hDqdx = (TH3F*)f->Get(Form("hDqdx_%d", iplane));
	    auto hNhits = (TH3F*)f->Get(Form("hNhits_%d", iplane));
	    if ( i == 0 ) {
		result_dqdx[iplane] = (TH3F*)hDqdx->Clone();
		result_nhits[iplane] = (TH3F*)hNhits->Clone();

		result_dqdx[iplane]->SetDirectory(outf);
		result_nhits[iplane]->SetDirectory(outf);

		continue;
	    }

	    hDqdx->Multiply(hNhits);
	    result_dqdx[iplane]->Add(hDqdx);
	    result_nhits[iplane]->Add(hNhits);
	}

	f->Close();
    }

    FOR(iplane, 3) {
	result_dqdx[iplane]->Divide(result_nhits[iplane]);
	// result_dqdx[iplane]->Write();
	// result_nhits[iplane]->Write();
    }

    outf->Write();
    outf->Close();
}
