void skimTree()
{
    //***** Input tree *****
    auto tree = new TChain("analysistree/anatree");
    for (int i = 21; i<=30; i++) {
        TString topdir = Form("data/kumar/2000%02d00/", i);
        for (int j = 0; j<100; j++) {
            TString fname = topdir +
                Form("2000%02d%02d/MUSUN_dunefd_2000%02d%02d_gen_g4_detsim_reco_ana.root",i,j,i,j);
            int status = tree->Add(fname, 500);
        }
    }

    tree->SetBranchStatus("*",0);

    std::vector<TString> allowed = {
	"run",
	"subrun",
	"event",
	"no_hits",
	"geant_list_size",
	"geant_list_size_in_tpcAV",
	"pdg",
	"status",
	"Mass",
	"Eng",
	"EndE",
	"Px",
	"Py",
	"Pz",
	"P",
	"StartPointx",
	"StartPointy",
	"StartPointz",
	"StartT",
	"EndPointx",
	"EndPointy",
	"EndPointz",
	"EndT",
	"theta",
	"phi",
	"theta_xz",
	"theta_yz",
	"pathlen",
	"inTPCActive",
	"StartPointx_tpcAV",
	"StartPointy_tpcAV",
	"StartPointz_tpcAV",
	"StartT_tpcAV",
	"StartE_tpcAV",
	"StartP_tpcAV",
	"StartPx_tpcAV",
	"StartPy_tpcAV",
	"StartPz_tpcAV",
	"EndPointx_tpcAV",
	"EndPointy_tpcAV",
	"EndPointz_tpcAV",
	"EndT_tpcAV",
	"EndE_tpcAV",
	"EndP_tpcAV",
	"EndPx_tpcAV",
	"EndPy_tpcAV",
	"EndPz_tpcAV",
	"pathlen_drifted",
	"inTPCDrifted",
	"StartPointx_drifted",
	"StartPointy_drifted",
	"StartPointz_drifted",
	"StartT_drifted",
	"StartE_drifted",
	"StartP_drifted",
	"StartPx_drifted",
	"StartPy_drifted",
	"StartPz_drifted",
	"EndPointx_drifted",
	"EndPointy_drifted",
	"EndPointz_drifted",
	"EndT_drifted",
	"EndE_drifted",
	"EndP_drifted",
	"EndPx_drifted",
	"EndPy_drifted",
	"EndPz_drifted",
	"NumberDaughters",
	"Mother",
	"TrackId",
	"MergedId",
	"origin",
	"MCTruthIndex",
	"process_primary",
	"processname",
	"nshowers_emshower",
	"ntracks_pandoraTrack",
    };

    for (auto brname : allowed)
	tree->SetBranchStatus(brname, 1);

    // output file name
    auto outf = new TFile("data/skimmed_g4_merged_kumar_"
			  "MUSUN_dunefd_2000xxxx_gen_g4_detsim_reco_ana.root",
			  "recreate");
    auto newtree = tree->CloneTree();
    newtree->Print();

    outf->Write();
    outf->Close();
}
