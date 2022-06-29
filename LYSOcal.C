#include <TROOT.h>
#include <TFile.h>
#include <TLatex.h>
#include <TSpectrum.h>
#include <TFitResult.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TGraph.h>
#include <Riostream.h>
#include "fitPeak2.C"
#include "TRint.h"

void LYSOcal(TString filename,Double_t fitMin=37, Double_t fitMax=47,
  Double_t thresh=0.2, Int_t nToFit=4, Int_t pol=2, TString myOpt="I", Int_t chn=0)
{
  if(myOpt.Contains('Q')) gROOT->SetBatch(1);

  TFile* file = TFile::Open(filename);
  file->cd();

  TString str2=filename;
  str2.ReplaceAll("_out.root","_fits");
  gSystem->MakeDirectory(str2);
  gSystem->cd(str2);

  // TString str=filename;
  TString str="calibrationCoefficients";
  if(chn==0) str.Append(".csv");
  else str=Form("calibrationCoefficients_%i.csv",chn);
  // if(chn==0) str.Append(".csv");
  // else str.Append(chn);
  // str.ReplaceAll("_out.root","_linearity.csv");
  // str.ReplaceAll("_out.root","_linearity.csv");
  // ofstream out(str);
  ofstream out(str);

  // str2.Append(".root");
  // TFile* outFile = new TFile(str2,"recreate");
  // cout << "Outputting to file: " << str << endl;

  // TCanvas* lin_c;//=new TCanvas("lin_c","Linearity fits",0,0,800,800);
  TCanvas* lin_c=new TCanvas("lin_c","Linearity fits",0,0,800,800);
  // lin_c=new TCanvas("lin_c","Linearity fits",0,0,800,800);
  lin_c->Divide(1,2);
  lin_c->cd(1);
  TH1I *histo;
  TFitResultPtr myFit;
  Double_t cent;
  Double_t sigma;
  TSpectrum* spec;
  Int_t nFound;
  Double_t* xPos;	// array of peaks found, ordered by amplitude

  vector<Double_t> ordered;	// vector of peaks found re-ordered by energy (largest first)
  // Double_t energyArray[4]={1275,511,307,202};//,0}; Na22 lines
  // Double_t energyArray[4]={661.66,511,306.78,201.83};//,0}; Na22 and Cs137 lines
  Double_t energyArray[5]={661.66,511,122.06,59.5409,31.414};//,0}; Na22 and Cs137 lines
  // Double_t energyArray[4]={136.47,122.06,14.4,6.4};//,0}; 57Co 136 is much smaller than I thought. Am seeing escape peak at 122 - ~55 keV
  // Double_t energyArray[4]={122.06,67,14.4,6.4};//,0}; 57Co
  vector<Double_t> energyVec;
  // if(nToFit==4) for(int i=0;i<nToFit;i++) energyVec.push_back(energyArray[i]);
  // // if(nToFit==3) for(int i=1;i<nToFit+1;i++) energyVec.push_back(energyArray[i]);
  // if(nToFit==3) for(int i=0;i<nToFit;i++) energyVec.push_back(energyArray[i]); // for Co56
  if(nToFit==1) for(int i=1;i<nToFit+1;i++) energyVec.push_back(energyArray[i]);
  else for(int i=0;i<nToFit;i++) energyVec.push_back(energyArray[i]); // for Co56

  char answer;
  int	peakIndex=0;
  int peaksLeft=0;
  int nUsed=0;
  double min, max;
  TLine* tl=new TLine(0,0,1,1);
  TGraph *gr;
  // TF1* linFunc = new TF1("linFunc","pol2",0,8192);
  TF1* linFunc;
  Float_t p0,p1,p2;
  TH1F* p0_h=new TH1F("p0_h","p0 coefficient",1024,0,1024);
  TH1F* p1_h=new TH1F("p1_h","p1 coefficient",1024,0,1024);
  TH1F* p2_h=new TH1F("p2_h","p2 coefficient",1024,0,1024);

  Bool_t readyToFit=0;
  Bool_t nextPeak=0;

  Int_t startChn, endChn;
  if(chn==0) {
    startChn=256;
    endChn=1024;
  }
  else {
    startChn=chn;
    endChn=chn+1;
  }

  for(int chnID=startChn; chnID<endChn; chnID++) {
  // for(int chnID=0; chnID<1024; chnID++) {
  // for(int chnID=256; chnID<1024; chnID++) {
  // for(int chnID=256; chnID<260; chnID++) {
    if(answer=='q') break;
    if(!myOpt.Contains('Q')) {
      cout << "#########################################" << endl;
      cout << "\n Processing channel " << chnID << "...";
    }
    histo = (TH1I*)gDirectory->Get(Form("qdc%i_h",chnID));
    if(histo->GetEntries()>100) {
      // lin_c=new TCanvas("lin_c","Linearity fits",0,0,800,800);

      myFit = fitPeak2(histo,fitMin,fitMax,myOpt,"","");
      cent=myFit->Parameter(3);
      sigma=myFit->Parameter(4);
      // lin_c->Clear();
      lin_c->cd(1);
    	gPad->SetLogy(1);
      histo->GetXaxis()->SetRangeUser(1,100);
    	histo->Draw();
    	spec=new TSpectrum();
    	// histo->GetXaxis()->UnZoom();
      nFound=spec->Search(histo,sigma,"",thresh);
      if(!myOpt.Contains('Q')) cout << endl << nFound << " peaks found..." << endl;

      vector<Double_t> qdcToUse;		// array of peaks to use for calibration and linearity correction
      vector<Double_t> EtoUse;

     	xPos=spec->GetPositionX();	// array of peaks found, ordered by amplitude
    	vector<Double_t> ordered;	// array of peaks found re-ordered by energy (largest first)
    	Int_t index[nFound];
    	peakIndex=0;
      peaksLeft=nFound;
    	nUsed=0;

    	TMath::Sort(nFound,xPos,index);

    	for(int i=0;i<nFound;i++) ordered.push_back(xPos[index[i]]);
    	if(myOpt.Contains("I")) {
    		if(!myOpt.Contains('Q')) {
          cout << "First guess at peaks to use for calibration: " << endl;
      		cout << "Index	Energy (keV)	Channel" << endl;
      		for(int i=0;i<nToFit;i++) cout << i << "	" << energyVec[i] << "		" << ordered[i] << endl;
        }
    	}

    	// histo->GetXaxis()->SetRangeUser(0,ordered[0]*1.1);
    	histo->GetXaxis()->SetRangeUser(ordered[nFound-1]*0.9,ordered[0]*1.1);
    	gPad->Update();
    	if(myOpt.Contains("V")) cout << endl << gPad->GetUymax() << " " << pow(10,gPad->GetUymax()) << endl;
    	min=pow(10,gPad->GetUymin());
    	max=pow(10,gPad->GetUymax());
    	gPad->Update();
    	tl=new TLine(0,0,1,1);
      tl->SetLineColor(2);
      tl->SetLineStyle(2);
      tl->SetLineWidth(2);

      // lin_c->cd(1);
    	gPad->SetLogy(1);
    	histo->GetXaxis()->SetRangeUser(ordered[nToFit-1]-ordered[0]*0.1,ordered[0]*1.1);
    	// histo->GetXaxis()->SetRangeUser(0,qdcToUse[0]*1.1);
    	histo->DrawCopy();
    	for(int i=0;i<nToFit;i++) tl->DrawLine(ordered[i],min,ordered[i],max);

// Draw peaks and fit
      gr=new TGraph(ordered.size(),&ordered[0],&energyVec[0]);
    	lin_c->cd(2);
    	gr->GetXaxis()->SetLimits(ordered[nToFit-1]-ordered[0]*0.1,ordered[0]*1.1);
    	// gr->GetXaxis()->SetLimits(0,qdcToUse[0]*1.1);
    	gr->Draw("a*");
    	gr->GetXaxis()->SetRangeUser(ordered[nToFit-1]-ordered[0]*0.1,ordered[0]*1.1);
    	// gr->GetXaxis()->SetRangeUser(0,qdcToUse[0]*1.1);
      gr->GetYaxis()->SetRangeUser(0,energyVec[0]*1.1);
    	gr->SetTitle("Calibration");
    	gr->GetXaxis()->SetTitle("channel");
    	gr->GetYaxis()->SetTitle("energy (keV)");
      if(pol==1) linFunc = new TF1("linFunc","pol1",0,8192);
      if(pol==2) linFunc = new TF1("linFunc","pol2",0,8192);
      if(nToFit==1) linFunc->FixParameter(0,0);
    	gr->Fit("linFunc","q");
    	gr->Draw("a*");
      gPad->Update();
/////////////////////

// interactively find the right peaks
      if(myOpt.Contains("I")) {
        cout << "Accept these peaks (y(es)/n(o)/s(kip)/q(uit)): ";
      	cin >> answer;
        if(answer=='q') break; // break out of loop over channels, i.e. abort!!!
        if(answer=='s') continue; // go to next channel
      	// while(answer!='q') {
      	while(!readyToFit) {
      		if(answer=='y') {
            for(int i=0;i<nToFit;i++) {
              qdcToUse.push_back(ordered[i]);
              EtoUse.push_back(energyVec[i]);
              nUsed=nToFit;
            }
      			cout << "Using these peaks..." << endl;
      			// answer='c';
            readyToFit=1;
      		} else if(answer=='n') {
            lin_c->Clear();
            lin_c->Divide(1,2);
            lin_c->cd(1);
            gPad->SetLogy(1);
            histo->GetXaxis()->SetRangeUser(ordered[nFound-1]*0.9,ordered[0]*1.1);
          	histo->Draw();

      			cout << "Which peaks to use..." << endl;

      			for(int i=0; i<nToFit; i++) {
      				cout << "Use " << energyVec[i] << " keV? (y/n): ";
      				cin >> answer;
              if(answer=='q') break;
              if(answer=='s') break;
      				if(answer=='n') continue; // continue to next peak
      				else if(answer=='y') {
      					while(!nextPeak) {
      					// while(answer!='c') {
                // if(peakIndex>=nFound) {
                  if(peaksLeft<1) {
                    cout << "No peaks left. Exiting." << endl;
                    // answer='s';
                    break; // break out of 'while(!readyToFit)'
                  }
      						tl->DrawLine(ordered[peakIndex],min,ordered[peakIndex],max);
      						gPad->Update();
      						cout << "	Use channel: " << ordered[peakIndex] <<" ? (y/n): ";
      						cin >> answer;
                  if(answer=='q') break;
                  if(answer=='s') continue;
                  if(answer=='y') {
                    peaksLeft--;
      							qdcToUse.push_back(ordered[peakIndex]);
      							EtoUse.push_back(energyVec[i]);
      							peakIndex++;
      							nUsed++;
      							break; // break out of 'while(!nextPeak)'
      						} else if(answer=='n') {
                    peaksLeft--;
      							peakIndex++;
      							continue; // try next peak
      						}
      						// else if(answer=='s') break; // break out of 'while(!readyToFit)'
      						// else cout << "try again" << endl;
      					}
      					// if(answer=='s') break; // break out of 'for(int i=0; i<nToFit; i++)'
      					// else continue;
      				}
      				// else if(answer=='s') break; // break out of 'for(int i=0; i<nToFit; i++)'
      			}
      			cout << endl << "New peaks to use for calibration: " << endl;
      			cout << "Energy (keV)	Channel" << endl;
      			for(int i=0;i<nUsed;i++) cout << EtoUse[i] << "		" << qdcToUse[i] << endl;
      			break; // break out of 'while(!readyToFit)'
      		} // end of 'if(answer=='n')', "Accept these peaks?"
      		else {
      			cout << "Try again (y/n): ";
      			cin >> answer;
            if(answer=='q') break;
            if(answer=='s') continue;
      		}
      	} // end of 'while(!readyToFit)'
        if(nUsed!=0) readyToFit=1;
      // }
      // if(answer=='q') break; // break out of Loop over channels
      } // end of interctive bit
      else for(int i=0;i<nToFit;i++) {
        qdcToUse.push_back(ordered[i]);
        EtoUse.push_back(energyVec[i]);
        nUsed=nToFit;
        readyToFit=1;
      }

      lin_c->Clear();
      lin_c->Divide(1,2);

     	// lin_c->cd(1);
    	// histo->GetXaxis()->SetRangeUser(fitMin*0.75,fitMax*1.25);
    	// histo->DrawCopy();

// if(answer!='s') {
      if(readyToFit) {
      	lin_c->cd(1);
      	gPad->SetLogy(1);
      	histo->GetXaxis()->SetRangeUser(qdcToUse[nUsed-1]-qdcToUse[0]*0.1,qdcToUse[0]*1.1);
      	// histo->GetXaxis()->SetRangeUser(0,qdcToUse[0]*1.1);
      	histo->DrawCopy();
      	for(int i=0;i<nUsed;i++) tl->DrawLine(qdcToUse[i],min,qdcToUse[i],max);

        gr=new TGraph(qdcToUse.size(),&qdcToUse[0],&EtoUse[0]);
      	lin_c->cd(2);
      	gr->GetXaxis()->SetLimits(qdcToUse[nUsed-1]-qdcToUse[0]*0.1,qdcToUse[0]*1.1);
      	// gr->GetXaxis()->SetLimits(0,qdcToUse[0]*1.1);
      	gr->Draw("a*");
      	gr->GetXaxis()->SetRangeUser(qdcToUse[nUsed-1]-qdcToUse[0]*0.1,qdcToUse[0]*1.1);
      	// gr->GetXaxis()->SetRangeUser(0,qdcToUse[0]*1.1);
        gr->GetYaxis()->SetRangeUser(0,EtoUse[0]*1.1);
      	gr->SetTitle("Calibration");
      	gr->GetXaxis()->SetTitle("channel");
      	gr->GetYaxis()->SetTitle("energy (keV)");
        if(pol==1) TF1* linFunc = new TF1("linFunc","pol1",0,8192);
        if(pol==2) TF1* linFunc = new TF1("linFunc","pol2",0,8192);
        if(nToFit==1) linFunc->FixParameter(0,0);
      	gr->Fit("linFunc","q");
      	gr->Draw("a*");
        gPad->Update();

        // cout << endl << "Writing results to file '" << name << "_linearity.dat'" << endl;
        p0=linFunc->GetParameter(0);
        p1=linFunc->GetParameter(1);
        if(pol==2) p2=linFunc->GetParameter(2);
        if(pol==1) p2=0;
        out << chnID << "," << p0 << "," << p1 << "," << p2 << endl;
        if(!myOpt.Contains('Q')) {
          cout << "Fit result:" << endl;
          cout << "ChannelID  p0  p1  p2" << endl;
          cout << chnID << "  " << p0 << "  " << p1 << "  " << p2 << endl;
        }
        // lin_c->Write();
        lin_c->SaveAs(Form("qdc_%i_linearity.png",chnID));
        // lin_c->SaveAs(Form("qdc_%i_linearity.pdf",chnID));
        p0_h->Fill(chnID,p0);
        p1_h->Fill(chnID,p1);
        if(pol==2) p2_h->Fill(chnID,p2);

        // if(myOpt.Contains("I")) {
        //   cout << "Press 'y' to continue: ";
        //   cin >> answer;
        // }
      } // end of 'readyToFit'
    } else if(!myOpt.Contains('Q')) cout << "No histogram." << endl; // end of 'if(histo->GetEntries()>100)'
    readyToFit=0;
  } // end of loop over channels
  out.close();

  // gROOT->SetBatch(0);

  TCanvas* coef_c=new TCanvas("coef_c","Calibration Coeffiecnts");
  coef_c->Divide(1,3);
  coef_c->cd(1);
  p0_h->Draw("");
  coef_c->cd(2);
  p1_h->Draw("");
  coef_c->cd(3);
  p2_h->Draw("");
  // if(pol==2) p2_h->Draw("");
  // coef_c->SaveAs("calibrationCoefficients.png");
  if(chn==0) coef_c->SaveAs("calibrationCoefficients.pdf");

  // coef_c->Write();
  // outFile->Close();

} // end of LYSOcal

void Usage()
{
	cout << endl << "LYSOcal(TString filename, Double_t fitMin, Double_t fitMax, Double_t thresh=0.0005, Int_t nToFit=4, Int_t pol=2, TString myOpt=\"Q\") \n\n"
		<< "Use 'fitMin' and 'fitMax' to set range to fit 511 keV peak in 'histo'. \n"
		<< "Use 'nToFit' to limit number of peaks to use for linearity correction. \n"
    << "'nToFit=3' will skip the 1275 keV peak. 'nToFit=1' uses only 511 keV and fixes the intercept at 0. \n"
    << "'thresh' can be used to adjust minimum amplitude of peaks that will be fitted.\n"
    << "'Use pol=2 for a quadriatic fit, or pol=1 for linear fit.\n'"
    << "Use 'myOpt=\"I\"' to interactively select the correct peaks." << endl;
}

void usage() { Usage();}
void Help() { Usage();}
void help() { Usage();}
