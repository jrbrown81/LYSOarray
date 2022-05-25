#define LYSOsorter_cxx
#include "LYSOsorter.h"
//#include <TH2.h>
//#include <TStyle.h>
//#include <TCanvas.h>
//#include <iostream>

void LYSOsorter::Loop(Int_t toProcess=0)
{
//	cout << "Switching to Batch mode" << endl;
//	gROOT->SetBatch(1);
	cout << "Executing Loop()" << endl;
//   In a ROOT session, you can do:
//      root> .L LYSOsorter.C
//	root> run("path/filename.root",nEntries_to_process)
//	if "nEntries_to_process" is omitted, whole file will be processed

	TString str=fChain->GetCurrentFile()->GetName();
	cout << "Reading data from file: " << str << endl;

	Long64_t nentries = fChain->GetEntriesFast();
	cout << "Found " << nentries << " entries in file" << endl;
	if(toProcess!=0) {
		nentries = toProcess;
		cout << "Processing " << nentries << " entries" << endl;
	}
	else cout << "Processing all entries" << endl;

	TH1I* qdc_h[512];
	for(int i=0;i<512;i++) {
		if(i<128) qdc_h[i]=new TH1I(Form("qdc%i_h",i),Form("QDC spectrum for chn %i",i),1100,-10,100);
		else qdc_h[i]=new TH1I(Form("qdc%i_h",i+512),Form("QDC spectrum for chn %i",i+512),1100,-10,100);
	}
	TH1I* tot_h[512];
	for(int i=0;i<512;i++) {
		if(i<128) tot_h[i]=new TH1I(Form("tot%i_h",i),Form("TOT spectrum for chn %i",i),500,0,5e6);
		else tot_h[i]=new TH1I(Form("tot%i_h",i+512),Form("TOT spectrum for chn %i",i+512),500,0,5e6);
	}

	TH2F* chnVsQDC_h = new TH2F("chnVsQDC_h","ChannelID vs. QDC value",550,-10,100,400,0,400);
	chnVsQDC_h->SetXTitle("QDC value");
	chnVsQDC_h->SetYTitle("channelID");
	TH2F* chnVsTOT_h = new TH2F("chnVsTOT_h","ChannelID vs. TOT value",550,-1e6,1e7,400,0,400);
	chnVsTOT_h->SetXTitle("TOT value");
	chnVsTOT_h->SetYTitle("channelID");

	TH1F* simpleSumEnergyQDC_h = new TH1F("simpleSumEnergyQDC_h","Simple Sum of all Energy spectra (QDC)",2200,-20,200);
	TH1F* sumEnergyQDC_h = new TH1F("sumEnergyQDC_h",Form("Total Energy (QDC) for mult#geq%i",minMult),2200,-20,200);
	TH1F* sumEnergyQDC2_h = new TH1F("sumEnergyQDC2_h",Form("Total Energy (QDC) for mult#geq%i and pixel energy #geq%.1f",minMult,minEnergy),2200,-20,200);
	
	TH1F* sumEnergyTOT_h = new TH1F("sumEnergyTOT_h",Form("Total Energy (TOT) for mult#geq%i",minMult),5500,-1e4,1e5);
	TH1F* sumTOT_h = new TH1F("sumTOT_h",Form("Sum of TOT signals for mult#geq%i",minMult),5500,-3e6,3e7);
	TH1I* mult_h = new TH1I("mult_h",Form("Number of SiPMs fired within time window (%.0f ns)",tWindow/1000.),300,0,300);
	mult_h->SetXTitle("# pixels");
	TH1F* timeDiff_h = new TH1F("timeDiff_h","Time difference between consecutive entries (ns)",1100,-50,500);
	timeDiff_h->SetXTitle("Time difference, ns");

	TH2F* sumQDCvsMult_h = new TH2F("sumQDCvsMult_h","Multiplicity vs. total energy (QDC)",1000,-100,1000,100,0,100);
	sumQDCvsMult_h->SetYTitle("multiplicity");
	sumQDCvsMult_h->SetXTitle("sum energy (QDC)");
	TH2F* sumTOTvsMult_h = new TH2F("sumTOTvsMult_h","Multiplicity vs. total energy (TOT)",1100,-1e4,1e5,100,0,100);
	sumTOTvsMult_h->SetYTitle("multiplicity");
	sumTOTvsMult_h->SetXTitle("sum energy (TOT)");

	TH2F* multVsTime_h = new TH2F("multVsTime_h","Multiplicity vs time window",10,0,10,300,0,300);
	multVsTime_h->SetXTitle("Coincidnce Time Window, ns");

	TH1F* sumEnergyArray_h[10];
	TH1F* sumEnergyArray2_h[12];
	TH2F* energyVsTime_h = new TH2F("energyVsTime_h","Sum Energy vs time window",10,0,10,220,-20,200);
	energyVsTime_h->SetXTitle("Coincidnce Time Window, ns");
	for(int i=0;i<10;i++) {
		multVsTime_h->GetXaxis()->SetBinLabel(i+1,Form("%.2f",tWindowArr[i]/1000.));
		energyVsTime_h->GetXaxis()->SetBinLabel(i+1,Form("%.2f",tWindowArr[i]/1000.));
		sumEnergyArray_h[i] = new TH1F(Form("sumEnergyArray_h_%i",i),Form("Sum energy using %.2f ns time window" ,tWindowArr[i]/1000.),2200,-20,200);
	}
	for(int i=0;i<12;i++) sumEnergyArray2_h[i] = new TH1F(Form("sumEnergyArray2_h_%i",i),Form("Sum energy using biggest %i pixels",i+2),2200,-20,200);

	TH2I* coincPlot_h = new TH2I("coincPlot_h","Coincidence Plot of largest energies",550,-10,100,550,-10,100);
	coincPlot_h->SetXTitle("Biggest energy");
	coincPlot_h->SetYTitle("2nd biggest energy");
	TH2I* coincPlot_21_22_h = new TH2I("coincPlot_21_22_h","Coincidence Plot of chn 22 vs chn 21",550,-10,100,550,-10,100);
	coincPlot_21_22_h->SetXTitle("energy (chn21)");
	coincPlot_21_22_h->SetYTitle("energy (chn22)");

	TH2I* chnCorrelation1_h = new TH2I("chnCorrelation1_h","Channel correlation plot (first vs current)",400,0,400,400,0,400);
	TH2I* chnCorrelation2_h = new TH2I("chnCorrelation2_h","Channel correlation plot (previous vs current)",400,0,400,400,0,400);
	TH2I* chnCorrelation3_h = new TH2I("chnCorrelation3_h","Channel correlation plot (largest energies)",400,0,400,400,0,400);
	TH1I* hitPattern_h = new TH1I("hitPattern_h","Channel hit pattern",400,0,400);
	
	TH2I* hitMap_h = new TH2I("hitMap_h","Hit map (all events)",16,0,53.76,16,0,53.76);
	hitMap_h->GetXaxis()->SetTitle("x (mm)");
	hitMap_h->GetYaxis()->SetTitle("y (mm)");
	
	TH2F* evtMap_h[100];
	for(int i=0;i<100;i++) {
		evtMap_h[i] = new TH2F(Form("evtMap_h_%i",i),Form("Hit map (event %i)",i),16,0,53.76,16,0,53.76);
		evtMap_h[i]->GetXaxis()->SetTitle("x (mm)");
		evtMap_h[i]->GetYaxis()->SetTitle("y (mm)");
		evtMap_h[i]->GetZaxis()->SetTitle("energy");
	}
	
	TH2I* chnMap_h = new TH2I("chnMap_h","Channel map",16,0,53.76,16,0,53.76);
	for(int i=0;i<256;i++) {
		if(i<64) chnMap_h->Fill(pixelX[i],pixelY[i],i);
		else if(i<128) chnMap_h->Fill(arraySize-pixelX[i-64],2*arraySize-pixelY[i-64],i);
		else if(i<192) chnMap_h->Fill(arraySize+pixelX[i-128],pixelY[i-128],i);
		else if(i<256) chnMap_h->Fill(2*arraySize-pixelX[i-192],2*arraySize-pixelY[i-192],i);
	//		else if(i<128) chnMap_h->Fill(pixelX[i-64]+arraySize,pixelY[i-64],i);
	//		else if(i<192) chnMap_h->Fill(pixelX[i-128],pixelY[i-128]+arraySize,i);
	//		else if(i<256) chnMap_h->Fill(pixelX[i-192]+arraySize,pixelY[i-192]+arraySize,i);
	}
	
	TH1F* intTime_h = new TH1F("intTime_h","Integration time (QDC mode only)",1000,0,2.5e3);
	intTime_h->SetXTitle("Integration time, ns");
	
	Long64_t prevTime=0;
	Long64_t tmpTime=0;
	Long64_t tmpTimeArr[10]={0,0,0,0,0,0,0,0,0,0};
	Long64_t timeDiff=0;
	Long64_t timeDiffArr[10]={0,0,0,0,0,0,0,0,0,0};
	Float_t sumEnergy=0;
	Float_t sumEnergy2=0;
	Long64_t sumTOT=0;
	Int_t mult=0;
	Int_t multArr[10]={0,0,0,0,0,0,0,0,0,0};
	Float_t sumEnergyArray[10]={0,0,0,0,0,0,0,0,0,0};
	
	Float_t tmpEnergy=0;
	
	Float_t energyList[512];
	Int_t chnArray[512];
	Int_t hitList[512];
	Int_t sortList[512];
	for(int i=0;i<512;i++) {
		energyList[i]=0;
		chnArray[i]=0;
		hitList[i]=0;
		sortList[i]=0;
	}
	
	Int_t tmpChn=0;
	Int_t prevChn=0;

   Long64_t nbytes = 0, nb = 0;
	Long64_t jentry=0;
	Long64_t event=0;
//	nentries=100;
// Loop over entries in tree
	for (jentry=0; jentry<nentries;jentry++) {
		if(jentry!=0 && jentry%100000==0) cout << jentry << " enties processed \r" << flush;
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
//       if (Cut(ientry) < 0) continue;
       
      timeDiff=time-tmpTime;
//      if(timeDiff<0) cout << jentry << endl;
      if(timeDiff<=tWindow) { // good coincidence
			sumEnergy+=energy;
			if(energy>minEnergy) sumEnergy2+=energy;
			sumTOT+=tot;
			mult++;
			chnCorrelation1_h->Fill(channelID,tmpChn);
			chnCorrelation2_h->Fill(channelID,prevChn);
			
			energyList[channelID]=energy;
//			chnArray[mult-1]=channelID;
			hitList[mult-1]=channelID;
			
		} else { // not good coincidence
// Fill histograms
			if(mult>=minMult) {
				sumEnergyQDC_h->Fill(sumEnergy);
				sumEnergyQDC2_h->Fill(sumEnergy2);
				sumTOT_h->Fill(sumTOT);
				sumEnergyTOT_h->Fill(sumEnergy);
			}
			sumQDCvsMult_h->Fill(sumEnergy,mult);
			sumTOTvsMult_h->Fill(sumEnergy,mult);
			mult_h->Fill(mult);
			
			TMath::Sort(512,energyList,sortList);
//			for(int i=0;i<mult;i++) {
			for(int i=0;i<=12;i++) {
				tmpEnergy+=energyList[sortList[i]];
				if(i!=0 && i<=12) sumEnergyArray2_h[i-1]->Fill(tmpEnergy);
			}
			tmpEnergy=0;
			
			if(mult>1) {
				coincPlot_h->Fill(energyList[sortList[1]],energyList[sortList[0]]);
//				coincPlot_h->Fill(energyList[sortList[0]],energyList[sortList[1]]);
				chnCorrelation3_h->Fill(sortList[1],sortList[0]);
				chnCorrelation3_h->Fill(sortList[0],sortList[1]);
				if(energyList[21]!=0&&energyList[22]!=0) coincPlot_21_22_h->Fill(energyList[21],energyList[22]);
			}
			
//			if(event<100) {
//				if(channelID<64) evtMap_h[event]->Fill(pixelX[channelID],pixelY[channelID]);
//				else if(channelID<128) evtMap_h[event]->Fill(arraySize-pixelX[channelID-64],2*arraySize-pixelY[channelID-64]);
//				else if(channelID>=256 && channelID<320) evtMap_h[event]->Fill(arraySize+pixelX[channelID-256],pixelY[channelID-256]);
//				else if(channelID<384) evtMap_h[event]->Fill(2*arraySize-pixelX[channelID-320],2*arraySize-pixelY[channelID-320]);
//			}

// Zero arrays
			for(int i=0;i<mult;i++) {
				energyList[hitList[i]]=0;
				hitList[i]=0;
			}
// Write entry to temporary variables
			energyList[channelID]=energy;
//			chnArray[0]=channelID;
			hitList[0]=channelID;
			
			sumEnergy=energy;
			if(energy>minEnergy) sumEnergy2=energy;
			else sumEnergy2=0;
			sumTOT=tot;
			mult=1;
			tmpTime=time;
			tmpChn=channelID;
			event++;
		}
      
// Test of how number of coincidenses evolves with time window
      for(int i=0;i<10;i++) {
			if((time-tmpTimeArr[i])<=tWindowArr[i]) {
				multArr[i]++;
				sumEnergyArray[i]+=energy;
			} else {
				multVsTime_h->Fill(i,multArr[i]);
				energyVsTime_h->Fill(i,sumEnergyArray[i]);
				sumEnergyArray_h[i]->Fill(sumEnergyArray[i]);
				
				multArr[i]=1;
				tmpTimeArr[i]=time;
				sumEnergyArray[i]=energy;
			}
		}
////////////

		chnVsQDC_h->Fill(energy,channelID);
		chnVsTOT_h->Fill(tot,channelID);
		simpleSumEnergyQDC_h->Fill(energy);
		if(channelID<128) {
			qdc_h[channelID]->Fill(energy);
			tot_h[channelID]->Fill(tot);
		}
		if(channelID>=512) {
			qdc_h[channelID-128]->Fill(energy);
			tot_h[channelID-128]->Fill(tot);
		}
		intTime_h->Fill(tot/1000);
		hitPattern_h->Fill(channelID);
		
		if(channelID<64) {
			hitMap_h->Fill(pixelX[channelID],pixelY[channelID]);
			if(event<100) evtMap_h[event]->Fill(pixelX[channelID],pixelY[channelID],energy);
		} else if(channelID<128) {
			hitMap_h->Fill(arraySize-pixelX[channelID-64],2*arraySize-pixelY[channelID-64]);
			if(event<100) evtMap_h[event]->Fill(arraySize-pixelX[channelID-64],2*arraySize-pixelY[channelID-64],energy);
		} else if(channelID>=256 && channelID<320) {
			hitMap_h->Fill(arraySize+pixelX[channelID-256],pixelY[channelID-256]);
			if(event<100) evtMap_h[event]->Fill(arraySize+pixelX[channelID-256],pixelY[channelID-256],energy);
		} else if(channelID<384) {
			hitMap_h->Fill(2*arraySize-pixelX[channelID-320],2*arraySize-pixelY[channelID-320]);
			if(event<100) evtMap_h[event]->Fill(2*arraySize-pixelX[channelID-320],2*arraySize-pixelY[channelID-320],energy);
		}
//		else if(channelID<128) hitMap_h->Fill(pixelX[channelID-64]+arraySize,pixelY[channelID-64]);
//		else if(channelID>=256 && channelID<320) hitMap_h->Fill(pixelX[channelID-256],pixelY[channelID-256]+arraySize);
//		else if(channelID<384) hitMap_h->Fill(pixelX[channelID-320]+arraySize,pixelY[channelID-320]+arraySize);

		timeDiff_h->Fill((time-prevTime)*1e-3); // in nanoseconds
		prevTime=time;
		prevChn=channelID;
		
   } // end of loop over entries
   cout << jentry << " enties processed \n";
   cout << event << " events found \n";
   
   TCanvas* qdcMode_c=new TCanvas("qdcMode_c","QDC mode");
   qdcMode_c->Divide(2,2);
   qdcMode_c->cd(1);
   qdcMode_c->GetPad(1)->SetLogy(1);
   sumEnergyQDC_h->Draw();
   qdcMode_c->cd(2);
   qdcMode_c->GetPad(2)->SetLogy(1);
   simpleSumEnergyQDC_h->Draw();
   qdcMode_c->cd(3);
	qdcMode_c->GetPad(3)->SetLogy(1);
	sumEnergyQDC2_h->Draw();
//	sumQDCvsMult_h->Draw("colz");
	qdcMode_c->cd(4);
	qdcMode_c->GetPad(4)->SetLogz(1);
	chnVsQDC_h->SetStats(0);
	chnVsQDC_h->Draw("colz");
	
	TCanvas* totMode_c=new TCanvas("totMode_c","TOT mode");
   totMode_c->Divide(2,2);
   totMode_c->cd(1);
   sumEnergyTOT_h->Draw();
   totMode_c->cd(2);
   sumTOTvsMult_h->Draw("colz");
	totMode_c->cd(3);
	chnVsTOT_h->Draw("colz");
	totMode_c->cd(4);
   sumTOT_h->Draw();

   TCanvas* mult_c=new TCanvas("mult_c","Timing and multiplicity plots");
   mult_c->Divide(2,2);
   mult_c->cd(1);
   mult_c->GetPad(1)->SetLogy(1);
   timeDiff_h->Draw();
   mult_c->cd(2);
   mult_c->GetPad(2)->SetLogz(1);
   energyVsTime_h->SetStats(0);
   energyVsTime_h->Draw("colz");
   mult_c->cd(3);
   mult_c->GetPad(3)->SetLogy(1);
   mult_h->Draw();
   mult_c->cd(4);
   mult_c->GetPad(4)->SetLogz(1);
   multVsTime_h->SetStats(0);
   multVsTime_h->Draw("colz");
   
   TCanvas* timeWindowTest_c = new TCanvas("timeWindowTest_c","Sum spectra with various timing windows");
   timeWindowTest_c->Divide(4,3);
   for(int i=0;i<10;i++) {
		timeWindowTest_c->cd(i+1);
		sumEnergyArray_h[i]->Draw();
	}
	
	TCanvas* sumTests_c = new TCanvas("sumTests_c","");
	sumTests_c->Divide(4,3);
	for(int i=0;i<12;i++) {
		sumTests_c->cd(i+1);
		gPad->SetLogy(1);
		sumEnergyArray2_h[i]->Draw("");
   }
   
   TCanvas* hitPatt_c=new TCanvas("hitPatt_c","Hit patterns etc.");
   hitPatt_c->Divide(2,2);
   hitPatt_c->cd(1);
   hitPattern_h->Draw();
   hitPatt_c->cd(2);
   chnCorrelation1_h->SetStats(0);
   chnCorrelation1_h->Draw("colz");
   hitPatt_c->cd(3);
   chnCorrelation2_h->SetStats(0);
   chnCorrelation2_h->Draw("colz");
   hitPatt_c->cd(4);
   hitMap_h->SetStats(0);
   hitMap_h->Draw("colz");
//   chnMap_h->Draw("text same");

	TCanvas* coinc_c=new TCanvas("coinc_c","coincidence plots");
	coinc_c->Divide(2,2);
	coinc_c->cd(1);
	coinc_c->GetPad(1)->SetLogz(1);
	chnCorrelation3_h->SetStats(0);
	chnCorrelation3_h->Draw("colz");
   coinc_c->cd(2);
   coinc_c->GetPad(2)->SetLogz(1);
   coincPlot_h->SetStats(0);
   coincPlot_h->Draw("colz");
   coinc_c->cd(3);
   coinc_c->GetPad(3)->SetLogz(1);
   coincPlot_21_22_h->SetStats(0);
   coincPlot_21_22_h->Draw("colz");
   
   TCanvas* qdc_c1=new TCanvas("qdc_c1","QDC plots 1");
   qdc_c1->Divide(16,8);
   TCanvas* qdc_c2=new TCanvas("qdc_c2","QDC plots 2");
   qdc_c2->Divide(16,8);
	for(int i=0;i<128;i++) {
		qdc_c1->cd(i+1);
		qdc_c1->GetPad(i+1)->SetLogy(1);
		qdc_h[i]->Draw();
		qdc_c2->cd(i+1);
		qdc_c2->GetPad(i+1)->SetLogy(1);
		qdc_h[i+128]->Draw();
	}
	TCanvas* tot_c1=new TCanvas("tot_c1","TOT plots 1");
   tot_c1->Divide(16,8);
   TCanvas* tot_c2=new TCanvas("tot_c2","TOT plots 2");
   tot_c2->Divide(16,8);
	for(int i=0;i<128;i++) {
		tot_c1->cd(i+1);
		tot_c1->GetPad(i+1)->SetLogy(1);
		tot_h[i]->Draw();
		tot_c2->cd(i+1);
		tot_c2->GetPad(i+1)->SetLogy(1);
		tot_h[i+128]->Draw();
	}
   
   TString str2=fChain->GetCurrentFile()->GetName();
   str2.ReplaceAll(".root","_out");
   gSystem->MakeDirectory(str2);
//   gSystem->cd(str2);
   str.ReplaceAll(".root","_out.root");
   TFile* outFile = new TFile(str,"recreate");
   cout << "Outputting to file: " << str << endl;
   sumEnergyQDC_h->Write();
   simpleSumEnergyQDC_h->Write();
   sumTOT_h->Write();
   sumEnergyTOT_h->Write();
   chnVsQDC_h->Write();
   chnVsTOT_h->Write();

   mult_h->Write();
   intTime_h->Write();
   multVsTime_h->Write();
   sumQDCvsMult_h->Write();
   sumTOTvsMult_h->Write();
   coincPlot_h->Write();
   coincPlot_21_22_h->Write();
   timeDiff_h->Write();
   chnCorrelation1_h->Write();
   chnCorrelation2_h->Write();
   chnCorrelation3_h->Write();
   hitPattern_h->Write();
   chnMap_h->Write();
	hitMap_h->Write();
	TCanvas* tmp_c=new TCanvas("tmp_c");
	TString str3;
   qdcMode_c->Write();
   totMode_c->Write();
   mult_c->Write();
   timeWindowTest_c->Write();
   sumTests_c->Write();
   hitPatt_c->Write();
   coinc_c->Write();
   qdc_c1->Write();
   qdc_c2->Write();
   tot_c1->Write();
   tot_c2->Write();

   outFile->mkdir("evtMaps/");
   outFile->cd("evtMaps/");
   gSystem->cd(str2);
   for(int i=0;i<100; i++) {
      evtMap_h[i]->Write();
      evtMap_h[i]->SetStats(0);
      evtMap_h[i]->Draw("colz");
      str3=evtMap_h[i]->GetName();
      str3.Append(".png");
      tmp_c->SaveAs(str3,"png");
   }
   outFile->Close();
   delete qdcMode_c;
   delete totMode_c;
   delete mult_c;
   delete timeWindowTest_c;
   delete sumTests_c;
   delete hitPatt_c;
   delete coinc_c;
   delete qdc_c1;
   delete qdc_c2;
   delete tot_c1;
   delete tot_c2;

//	save(step1,step2);

//   gROOT->SetBatch(0);
//   cout << "Return to normal mode" << endl;
}

void run(TString string, Int_t toProcess=0)
{
	TFile f1(string);
	TTree* tree;
	f1.GetObject("data",tree);
	if(tree) cout << "data tree found in file: " << string << endl;
	else cout << "Error!" << endl;
	LYSOsorter pss(tree);
	
	cout << "Switching to Batch mode" << endl;
	gROOT->SetBatch(1);
	pss.Loop(toProcess);
	gROOT->SetBatch(0);
	cout << "Return to normal mode" << endl;

	delete tree;
}

/*int save(Float_t step1,Float_t step2)
{
	
	str.ReplaceAll(".root","_out.root");
	TFile* outFile = new TFile(str,"recreate");
	cout << "Outputting to file: " << str << endl;
	sumEnergyQDC_h->Write();
	sumTOT_h->Write();
	sumEnergyTOT_h->Write();
	chnVsQDC_h->Write();
	chnVsTOT_h->Write();
//   for(int i=0;i<512;i++) {
//		qdc_h[i]->Write();
//		tot_h[i]->Write();
//	}
	mult_h->Write();
	intTime_h->Write();
	multVsTime_h->Write();
	sumQDCvsMult_h->Write();
	sumTOTvsMult_h->Write();
	timeDiff_h->Write();
	chnCorrelation1_h->Write();
	chnCorrelation2_h->Write();
	hitPattern_h->Write();
	intTime_h->Write();
	qdcMode_c->Write();
	totMode_c->Write();
	mult_c->Write();
	hitPatt_c->Write();
	qdc_c1->Write();
	qdc_c2->Write();
	tot_c1->Write();
	tot_c2->Write();
	outFile->Close();
	delete qdcMode_c;
	delete totMode_c;
	delete mult_c;
	delete hitPatt_c;
	delete qdc_c1;
	delete qdc_c2;
	delete tot_c1;
	delete tot_c2;
	
	return 1;
}
*/
