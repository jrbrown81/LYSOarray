/////////////////////////////////////////////////////////////////////////
//////////////////////////// LYSOsimpleSorter ///////////////////////////
// Simple code to read petsys ROOT files from LYSO array and generare
// histograms, with and without energy calibration applied.
// Energy calibration is read in from a csv file.
//
//   In a ROOT session, you can do:
//      root> .L LYSOsimpleSorter.C
//	root> run("path/filename.root",nEntries_to_process)
//	if "nEntries_to_process" is omitted, whole file will be processed
/////////////////////////////////////////////////////////////////////////

#define LYSOsimpleSorter_cxx
#include "LYSOsimpleSorter.h"
#include <TSystem.h>

void LYSOsimpleSorter::Loop(Int_t toProcess=0)
{
	cout << "Executing Loop()" << endl;

	TString str=fChain->GetCurrentFile()->GetName();
	cout << "Reading data from file: " << str << endl;

	Long64_t nentries = fChain->GetEntriesFast();
	cout << "Found " << nentries << " entries in file" << endl;
	if(toProcess!=0) {
		nentries = toProcess;
		cout << "Processing " << nentries << " entries" << endl;
	}
	else cout << "Processing all entries" << endl;

	TH1I* qdc_h[1024];
	TH1I* energy_h[1024];
	for(int i=0;i<1024;i++) {
		qdc_h[i]=new TH1I(Form("qdc%i_h",i),Form("QDC spectrum for chn %i",i),1100,-10,100);
		energy_h[i]=new TH1I(Form("energy%i_h",i),Form("Energy spectrum for chn %i",i),2100,-100,2000);
	}
	TH1I* totalEnergy_h=new TH1I("totalEnergy_h","Total Energy Spectrum of all pixels (keV)",2000,0,2000);

	TH2F* chnVsQDC_h = new TH2F("chnVsQDC_h","ChannelID vs. QDC value",550,-10,100,1024,0,1024);
	chnVsQDC_h->SetXTitle("QDC value");
	chnVsQDC_h->SetYTitle("channelID");
	TH2F* chnVsEnergy_h = new TH2F("chnVsEnergy_h","ChannelID vs. Energy (keV)",500,0,2000,1024,0,1024);
	chnVsEnergy_h->SetXTitle("Energy (keV)");
	chnVsEnergy_h->SetYTitle("channelID");

	TH1I* hitPattern_h = new TH1I("hitPattern_h","Channel hit pattern",1024,0,1024);

	// TH2I* hitMap_h = new TH2I("hitMap_h","Hit map (all events)",16,0,53.76,16,0,53.76);
	// hitMap_h->GetXaxis()->SetTitle("x (mm)");
	// hitMap_h->GetYaxis()->SetTitle("y (mm)");

	TH2I* chnMap_h = new TH2I("chnMap_h","Channel map",16,0,53.76,16,0,53.76);
	for(int i=0;i<256;i++) {
		if(i<64) chnMap_h->Fill(pixelX[i],pixelY[i],i);
		else if(i<128) chnMap_h->Fill(arraySize-pixelX[i-64],2*arraySize-pixelY[i-64],i);
		else if(i<192) chnMap_h->Fill(arraySize+pixelX[i-128],pixelY[i-128],i);
		else if(i<256) chnMap_h->Fill(2*arraySize-pixelX[i-192],2*arraySize-pixelY[i-192],i);
	}

	TH1F* intTime_h = new TH1F("intTime_h","Integration time (QDC mode only)",1000,0,2.5e3);
	intTime_h->SetXTitle("Integration time, ns");

	Double_t eCal=0;

	Long64_t nbytes = 0, nb = 0;
	Long64_t jentry=0;
// Loop over entries in tree
	for (jentry=0; jentry<nentries;jentry++) {
		if(jentry!=0 && jentry%10000==0) cout << jentry << " enties processed \r" << flush;
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
//       if (Cut(ientry) < 0) continue;

		if(calCoef[channelID][1]!=0) eCal=calCoef[channelID][0]+calCoef[channelID][1]*energy+calCoef[channelID][2]*energy*energy;
		else eCal=energy;

		chnVsQDC_h->Fill(energy,channelID);
		chnVsEnergy_h->Fill(eCal,channelID);
		qdc_h[channelID]->Fill(energy);
		energy_h[channelID]->Fill(eCal);
		totalEnergy_h->Fill(eCal);
		intTime_h->Fill(tot/1000);
		hitPattern_h->Fill(channelID);

		// if(channelID<64) {
		// 	hitMap_h->Fill(pixelX[channelID],pixelY[channelID]);
		// } else if(channelID<128) {
		// 	hitMap_h->Fill(arraySize-pixelX[channelID-64],2*arraySize-pixelY[channelID-64]);
		// } else if(channelID>=256 && channelID<320) {
		// 	hitMap_h->Fill(arraySize+pixelX[channelID-256],pixelY[channelID-256]);
		// } else if(channelID<384) {
		// 	hitMap_h->Fill(2*arraySize-pixelX[channelID-320],2*arraySize-pixelY[channelID-320]);
		// }

   } // end of loop over entries
   cout << jentry << " enties processed \n";
   // cout << event << " events found \n";

   TCanvas* qdcMode_c=new TCanvas("qdcMode_c","QDC mode");
   qdcMode_c->Divide(2,2);
   qdcMode_c->cd(1);
   qdcMode_c->GetPad(1)->SetLogy(1);
   qdcMode_c->cd(2);
   qdcMode_c->GetPad(2)->SetLogy(1);
   qdcMode_c->cd(3);
   qdcMode_c->GetPad(3)->SetLogy(1);
   qdcMode_c->cd(4);
   qdcMode_c->GetPad(4)->SetLogz(1);
   chnVsQDC_h->SetStats(0);
   chnVsQDC_h->Draw("colz");

	 TCanvas* hitPatt_c=new TCanvas("hitPatt_c","Hit patterns etc.");
	 hitPatt_c->Divide(2,2);
	 hitPatt_c->cd(1);
	 hitPattern_h->Draw();
	 hitPatt_c->cd(2);
	  hitPatt_c->cd(3);
	  hitPatt_c->cd(4);
   // hitMap_h->SetStats(0);
   // hitMap_h->Draw("colz");
//   chnMap_h->Draw("text same");

   TCanvas* qdc_c[8];
	 for(int i=0;i<8;i++) {
		 qdc_c[i]=new TCanvas(Form("qdc_c%i",i),Form("QDC plots %i",i));
		 qdc_c[i]->Divide(16,8);
	 }
	 for(int i=0;i<1024;i++) {
			qdc_c[i/128]->cd(i%128+1);
		 	qdc_c[i/128]->GetPad(i%128+1)->SetLogy(1);
		 	qdc_h[i]->Draw();
	 }

   str.ReplaceAll(".root","_out.root");
   TFile* outFile = new TFile(str,"recreate");
   cout << "Outputting to file: " << str << endl;

   chnVsQDC_h->Write();
   chnVsEnergy_h->Write();

   intTime_h->Write();

   hitPattern_h->Write();
   chnMap_h->Write();
	// hitMap_h->Write();
   qdcMode_c->Write();
   hitPatt_c->Write();
   for(int i=0;i<8;i++) qdc_c[i]->Write();
	 for(int i=0;i<1024;i++) {
		 qdc_h[i]->Write();
		 energy_h[i]->Write();
	 }
	 totalEnergy_h->Write();

   outFile->Close();
   delete qdcMode_c;
   delete hitPatt_c;
   for(int i=0;i<8;i++) delete qdc_c[i];

}

void readCalFile(TString calFileName)
{
// Read calibration file
	string line, word;
	Int_t col=0;
	ifstream calFile(calFileName);

	Int_t chn=-1;
	for(int i=0;i<1024;i++) calCoef[i][1]=0;

	if(calFile.is_open()){
		cout << "Reading calibration from file: " << calFileName << endl;
		while(getline(calFile,line)){
			istringstream iss(line);
			while(getline(iss,word,',')){
				if(col==0) chn=(Int_t)stoi(word);
				else calCoef[chn][col-1]=(Double_t)stof(word);
				col++;
			}
			chn=-1;
			col=0;
		}
	} else cout << "Calibtaion file: " << calFileName << " not found!" << endl;
	// for(int i=0;i<1024;i++) {
	// 	if(calibrated[i]==1) {
	// 		cout << i << " ";
	// 		for(int j=0;j<3;j++) cout << calCoef[i][j] << " ";
	// 		cout << endl;
	// 	}
	// }
}

void run(TString inFile, TString calFileName="calFile.csv", Int_t toProcess=0)
{

	TFile f1(inFile);
	TTree* tree;
	f1.GetObject("data",tree);
	if(tree) cout << "data tree found in file: " << inFile << endl;
	else cout << "Error!" << endl;
	LYSOsimpleSorter pss(tree);

	cout << "Switching to Batch mode" << endl;
	gROOT->SetBatch(1);

	readCalFile(calFileName);

	pss.Loop(toProcess);
	gROOT->SetBatch(0);
	cout << "Return to normal mode" << endl;

	delete tree;
}
