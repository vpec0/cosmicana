//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Jul  3 12:49:48 2019 by ROOT version 6.16/00
// from TTree muon_tree/Truth data for all cosmic muons entering the TPC
// found on file: CosmicAna_protodune_sp_geo.root
//////////////////////////////////////////////////////////

#ifndef CosmicAna_h
#define CosmicAna_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

const size_t MAXTRAJPOINTS = 3000;

class CosmicAna {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           Event;
   Int_t           Run;
   Int_t           SubRun;
   Double_t        startPos[4];
   Double_t        endPos[4];
   Double_t        startMom[4];
   Double_t        endMom[4];
   UShort_t        exited;
   UInt_t          n_points;
   Double_t        sp_pos[MAXTRAJPOINTS][3];   //[n_points]
   Double_t        sp_en[MAXTRAJPOINTS];   //[n_points]
   Double_t        dedx[MAXTRAJPOINTS];   //[n_points]
   Double_t        dedx_pos[MAXTRAJPOINTS][3];   //[n_points]
   std::string    *endProcess;

   // List of branches
   TBranch        *b_Event;   //!
   TBranch        *b_Run;   //!
   TBranch        *b_SubRun;   //!
   TBranch        *b_startPos;   //!
   TBranch        *b_endPos;   //!
   TBranch        *b_startMom;   //!
   TBranch        *b_endMom;   //!
   TBranch        *b_exited;   //!
   TBranch        *b_n_points;   //!
   TBranch        *b_sp_pos;   //!
   TBranch        *b_sp_en;   //!
   TBranch        *b_dedx;   //!
   TBranch        *b_dedx_pos;   //!
   TBranch        *b_endProcess;   //!

   CosmicAna(TTree *tree=0);
   virtual ~CosmicAna();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef CosmicAna_cxx
CosmicAna::CosmicAna(TTree *tree) : fChain(0)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("CosmicAna_protodune_sp_geo.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("CosmicAna_protodune_sp_geo.root");
      }
      TDirectory * dir = (TDirectory*)f->Get("CosmicAna_protodune_sp_geo.root:/CosmicAna");
      dir->GetObject("muon_tree",tree);

   }
   Init(tree);
}

CosmicAna::~CosmicAna()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t CosmicAna::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t CosmicAna::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void CosmicAna::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("Event", &Event, &b_Event);
   fChain->SetBranchAddress("Run", &Run, &b_Run);
   fChain->SetBranchAddress("SubRun", &SubRun, &b_SubRun);
   fChain->SetBranchAddress("startPos", startPos, &b_startPos);
   fChain->SetBranchAddress("endPos", endPos, &b_endPos);
   fChain->SetBranchAddress("startMom", startMom, &b_startMom);
   fChain->SetBranchAddress("endMom", endMom, &b_endMom);
   fChain->SetBranchAddress("exited", &exited, &b_exited);
   fChain->SetBranchAddress("n_points", &n_points, &b_n_points);
   fChain->SetBranchAddress("sp_pos", sp_pos, &b_sp_pos);
   fChain->SetBranchAddress("sp_en", sp_en, &b_sp_en);
   fChain->SetBranchAddress("dedx", dedx, &b_dedx);
   fChain->SetBranchAddress("dedx_pos", dedx_pos, &b_dedx_pos);
   fChain->SetBranchAddress("endProcess", &endProcess, &b_endProcess);
   Notify();
}

Bool_t CosmicAna::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void CosmicAna::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t CosmicAna::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef CosmicAna_cxx
