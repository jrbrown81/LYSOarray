//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Dec  9 14:37:03 2019 by ROOT version 6.16/00
// from TTree data/Event List
// found on file: 22Na_BGO_5_Dec_2019_29V_single.root
//////////////////////////////////////////////////////////

#ifndef LYSOsimpleSorter_h
#define LYSOsimpleSorter_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include <sstream>

// Some global variables //
	Float_t arraySize=26.88;
	Float_t pixelX[64] = { 25.18,21.82,15.1,21.82,18.46,25.18,15.1,21.82,18.46,11.74,25.18,15.1,21.82,	18.46,25.18,15.1,21.82,25.18,25.18,21.82,25.18,21.82,21.82,25.18,15.1,18.46,15.1,18.46,	18.46,18.46,15.1,15.1,18.46,11.74,8.38,8.38,11.74,8.38,8.38,11.74,8.38,11.74,1.66,	5.02,1.66,5.02,5.02,1.66,1.66,5.02,11.74,1.66,5.02,5.02,1.66,1.66,5.02,8.38,	8.38,11.74,1.66,8.38,11.74,5.02
	};
	Float_t pixelY[64] = { 21.82,21.82,18.46,25.18,25.18,15.1,25.18,18.46,21.82,15.1,25.18,15.1,15.1,18.46,18.46,	21.82,5.02,1.66,5.02,1.66,11.74,11.74,8.38,8.38,1.66,1.66,5.02,15.1,5.02,8.38,8.38,11.74,	11.74,11.74,11.74,8.38,8.38,5.02,15.1,5.02,1.66,1.66,11.74,11.74,8.38,5.02,8.38,5.02,1.66,	1.66,25.18,15.1,15.1,18.46,18.46,21.82,21.82,25.18,18.46,21.82,25.18,21.82,18.46,25.18
	};
  Double_t calCoef[1024][3];

// Header file for the classes stored in the TTree if any.
class LYSOsimpleSorter {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Float_t         step1;	// value of parameter 1 (when doing parameter scans)
   Float_t         step2;	// value of parameter 2 (when doing parameter scans)
   Long64_t        time;	// time of event, in picoseconds
   UInt_t          channelID;
   Float_t         tot;		// In QDC mode this is the integration time in nanoseconds
   Float_t         energy;
   UShort_t        tacID;
   Int_t           xi;
   Int_t           yi;
   Float_t         x;
   Float_t         y;
   Float_t         z;
   Float_t         tqT;	// the fine timing of the chn (crossing time threshold), in TDC clock units (200 MHz => 5ns
   Float_t         tqE;

   // List of branches
   TBranch        *b_step1;   //!
   TBranch        *b_step2;   //!
   TBranch        *b_time;   //!
   TBranch        *b_channelID;   //!
   TBranch        *b_tot;   //!
   TBranch        *b_energy;   //!
   TBranch        *b_tacID;   //!
   TBranch        *b_xi;   //!
   TBranch        *b_yi;   //!
   TBranch        *b_x;   //!
   TBranch        *b_y;   //!
   TBranch        *b_z;   //!
   TBranch        *b_tqT;   //!
   TBranch        *b_tqE;   //!

   LYSOsimpleSorter(TTree *tree=0);
   virtual ~LYSOsimpleSorter();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(Int_t toProcess);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);

};

#endif

#ifdef LYSOsimpleSorter_cxx
LYSOsimpleSorter::LYSOsimpleSorter(TTree *tree) : fChain(0)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("22Na_BGO_5_Dec_2019_29V_single.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("22Na_BGO_5_Dec_2019_29V_single.root");
      }
      f->GetObject("data",tree);

   }
   Init(tree);
}

LYSOsimpleSorter::~LYSOsimpleSorter()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t LYSOsimpleSorter::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t LYSOsimpleSorter::LoadTree(Long64_t entry)
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

void LYSOsimpleSorter::Init(TTree *tree)
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

   fChain->SetBranchAddress("step1", &step1, &b_step1);
   fChain->SetBranchAddress("step2", &step2, &b_step2);
   fChain->SetBranchAddress("time", &time, &b_time);
   fChain->SetBranchAddress("channelID", &channelID, &b_channelID);
   fChain->SetBranchAddress("tot", &tot, &b_tot);
   fChain->SetBranchAddress("energy", &energy, &b_energy);
   fChain->SetBranchAddress("tacID", &tacID, &b_tacID);
   fChain->SetBranchAddress("xi", &xi, &b_xi);
   fChain->SetBranchAddress("yi", &yi, &b_yi);
   fChain->SetBranchAddress("x", &x, &b_x);
   fChain->SetBranchAddress("y", &y, &b_y);
   fChain->SetBranchAddress("z", &z, &b_z);
   fChain->SetBranchAddress("tqT", &tqT, &b_tqT);
   fChain->SetBranchAddress("tqE", &tqE, &b_tqE);

   Notify();
}

Bool_t LYSOsimpleSorter::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

//   str=fChain->GetCurrentFile()->GetName();
//	cout << "Reading data from file: " << str << endl;

   return kTRUE;
}

void LYSOsimpleSorter::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t LYSOsimpleSorter::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.

//	if(step1==20 && step2==20)   return 1;
//	else return -1;

	return 1;
}
#endif // #ifdef LYSOsorter_cxx
