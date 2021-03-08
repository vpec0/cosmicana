#ifndef FILEHANDLER_H
#define FILEHANDLER_H

class FileHandler
{
public:
    /// default constructor
    FileHandler(){};
    /// default destructor
    ~FileHandler(){};

    static size_t attachFiles(TChain* tree, const char* fname,
			      size_t batchNo, size_t Nruns, size_t startRun,
			      const char* data_version = "v08_34_00",
			      const char* source = "")
    {
	size_t size = 0;

	if (!strcmp(fname, "")) { // no input given
	    cout<<"Will add production files from batch "
		<<batchNo<<" to the chain (unchecked)."<<endl;
	    TString batch = Form("%lu", batchNo);
	    TString topdir = "/data/dune/calibration/cosmic_muons/dunetpc_";
	    topdir.Append(data_version).Append("/");
	    if (!strcmp("kumar", source)) {
		topdir = "/data/kumar/dune/cosmic/largeproduction/";
		topdir.Append(data_version).Append("/data/");
	    } else if (!strcmp("viktor", source)) {
		topdir = "/data/vpec/dune/cosmics/";
		topdir.Append(data_version).Append("/");
	    } else if (!strcmp("fnal", source)) {
		topdir = "xroot://fndca1.fnal.gov:1094/pnfs/fnal.gov/usr/dune/persistent/users/calibration/cosmic_muons/sheffield_prod/dunetpc_";
		topdir.Append(data_version).Append("/");
	    }
	    topdir.Append(batch + "/");
	    cout<<"Batch directory "<<topdir<<endl;
	    for (size_t j = 0; j<Nruns; j++) {
		TString runNo = Form("%lu", batchNo + j + startRun);
		TString fname = topdir + runNo + "/MUSUN_dunefd_" + runNo + "_gen_g4_detsim_reco_ana.root";
		//cout<<"Adding run "<<runNo<<", file "<<fname<<endl;
		int status = tree->Add(fname, TTree::kMaxEntries);
	    }
	    tree->GetEntry(0);
	    size = tree->GetTree()->GetEntries();
	    size *= tree->GetNtrees();
	    cout<<"Added "<<tree->GetNtrees()<<" trees"<<endl;
	    cout<<"Estimated number of entries: "<<size<<endl;
	} else { // input given
	    cout<<"Adding "<<fname<<" to the chain."<<endl;
	    int status = tree->Add(fname, -1);
	    if (!status) { // try to look in the base dir
		tree->SetName("anatree");
		status = tree->Add(fname, -1);
	    }
	    cout<<"Status: "<<status<<endl;
	    size = tree->GetEntries();
	}

	return size;
    };

};

#endif
