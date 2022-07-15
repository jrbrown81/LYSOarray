#include <TString.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void readPETsysTempFile(TString filename="tempLog.txt", int mod2Plot=-1, bool requireRunNo=1) {


  int lineCount=0;
  int recordCount=0;
  int counter=0;
  vector<float> datime_v;
  vector<float> run_v;
  vector<float> temp_v[8][4];
  vector<bool> calFlag_v;
  vector<float> calDatime_v;
  vector<float> calRun_v;
  vector<float> calTemp_v[8][4];

  float temp;
  float tempBuffer[4];
  float run;
  int mod;
  string dummy;

  string line, word;
  ifstream infile(filename);
  TDatime datime;

  Bool_t newRecord=0;
  Bool_t goodRecord=0;
  Bool_t goodTime=0;
  Bool_t goodRun=0;
  Bool_t goodTemp=0;

  if(!infile.is_open())
    cout << "\nTemperature log file '" << filename << "' not found. \nExiting." << endl;
  else {
		cout << "\nReading temperature log from file: ''" << filename << "'\n" << endl;
    if(!requireRunNo) {
      cout << "----------------------------------------------------" << endl;
      cout << "!!! Warning, run numbers are not being required. !!!" << endl;
      cout << "!!! Any plots with run numbers in should be      !!!" << endl;
      cout << "!!! treated with suspicion                       !!!" << endl;
      cout << "----------------------------------------------------\n" << endl;
    }
		while(getline(infile,line)){
      lineCount++;
      if(line.find("---")!=string::npos) {
        line.erase(line.begin(),line.begin()+30);
        line.erase(line.end()-30,line.end());
        if(line.find("---")==string::npos) { // date/time line
          datime.Set(&line[0]);
          newRecord=1;
          goodTime=1;
          goodTemp=0;
          goodRecord=0;
        }
      } else if(line.rfind("0",0)==0) { // temperature lines
        istringstream iss(line);
        // int sensor=0;
        iss >> dummy >> dummy >> mod >> tempBuffer[0] >> dummy >> tempBuffer[1] >> dummy >> tempBuffer[2] >> dummy >> tempBuffer[3] >> dummy;
        goodTemp=1;
        if((!requireRunNo || goodRun) && goodTime && goodTemp && newRecord) goodRecord=1;
        if(goodRecord) {
          for(int s=0;s<4;s++) temp_v[mod][s].push_back(tempBuffer[s]);
          if(newRecord) {
            datime_v.push_back(datime.Convert());
            run_v.push_back(run);
            newRecord=0;
            goodRun=0;
            goodTime=0;
            goodTemp=0;
          }
        }
      } else {
        istringstream iss(line);
        while(getline(iss,word,' ')) {
          if(word.find("Run")!=string::npos) { // word contains "Run"
            run=stof(word.substr(3));
            calFlag_v.push_back(0);
            newRecord=1;
            goodRun=1;
            goodRecord=0;
            goodTime=0;
          } else if(word.find("Cal")!=string::npos) { // line contains "Cal"
            calFlag_v.back()=1;
          }
        }
      }
    }
  }

  for(int i=0;i<calFlag_v.size();i++) {
    if(calFlag_v[i]) {
      calRun_v.push_back(run_v[i]);
      calDatime_v.push_back(datime_v[i]);
      for(int m=0;m<8;m++)
        if(temp_v[m][0].size()!=0)
          for(int s=0;s<4;s++)
            calTemp_v[m][s].push_back(temp_v[m][s][i]);
    }
  }

  cout << "Runs: " << run_v.size() << endl;
  cout << "Calibration runs: " << calRun_v.size() << endl;
  cout << "Date and Time records: " <<  datime_v.size() << endl;
  mod=0;
  for(int m=0;m<8;m++) {
    if(temp_v[m][0].size()!=0) {
      mod++;
      if(mod==1) cout << "Temperature records: " << temp_v[m][0].size() << endl;
    }
  }
  cout << "Active modules: " << mod << endl << endl;

  TGraph* gr1[8][4];
  TGraph* gr2[8][4];
  TGraph* gr3[8][4];
  TGraph* gr4[8][4];
  for(int m=0;m<8;m++) {
    if(temp_v[m][0].size()!=0) {
      for(int s=0;s<4;s++) {
        gr1[m][s]=new TGraph(datime_v.size(),&datime_v[0],&temp_v[m][s][0]);
        gr1[m][s]->SetTitle(Form("Module %i, Sensor %i",m,s));
        gr1[m][s]->SetMarkerStyle(s+20+m*4);
        gr1[m][s]->GetXaxis()->SetTimeDisplay(1);
        gr1[m][s]->GetXaxis()->SetTimeOffset(0,"gmt");
        gr2[m][s]=new TGraph(run_v.size(),&run_v[0],&temp_v[m][s][0]);
        gr2[m][s]->SetTitle(Form("Module %i, Sensor %i",m,s));
        gr2[m][s]->SetMarkerStyle(s+20+m*4);
        gr3[m][s]=new TGraph(calRun_v.size(),&calDatime_v[0],&calTemp_v[m][s][0]);
        gr3[m][s]->SetTitle(Form("Module %i, Sensor %i",m,s));
        gr3[m][s]->SetMarkerStyle(s+20+m*4);
        gr3[m][s]->GetXaxis()->SetTimeDisplay(1);
        gr3[m][s]->GetXaxis()->SetTimeOffset(0,"gmt");
        gr4[m][s]=new TGraph(calRun_v.size(),&calRun_v[0],&calTemp_v[m][s][0]);
        gr4[m][s]->SetTitle(Form("Module %i, Sensor %i",m,s));
        gr4[m][s]->SetMarkerStyle(s+20+m*4);
      }
    }
  }

  TCanvas* c1=new TCanvas("c1","Temperature Logs");
  c1->Divide(2,2);
  TMultiGraph* mg1=new TMultiGraph();
  TMultiGraph* mg2=new TMultiGraph();
  TMultiGraph* mg3=new TMultiGraph();
  TMultiGraph* mg4=new TMultiGraph();
  if(mod2Plot==-1) {
    for(int m=0;m<8;m++) {
      for(int s=0;s<4;s++) {
        if(temp_v[m][s].size()!=0) mg1->Add(gr1[m][s],"lp");
        if(temp_v[m][s].size()!=0) mg2->Add(gr2[m][s],"lp");
        if(calTemp_v[m][s].size()!=0) mg3->Add(gr3[m][s],"lp");
        if(calTemp_v[m][s].size()!=0) mg4->Add(gr4[m][s],"lp");
      }
    }
    mg1->SetTitle("Temperature Log ; Date/Time ;Temperature (C)");
    mg2->SetTitle("Temperature Log ; Run # ;Temperature (C)");
    mg3->SetTitle("Temperature Log for Calibration Runs ; Date/Time ;Temperature (C)");
    mg4->SetTitle("Temperature Log for Calibration Runs ; Run # ;Temperature (C)");
  } else {
    for(int s=0;s<4;s++) {
      if(temp_v[mod2Plot][s].size()!=0) mg1->Add(gr1[mod2Plot][s],"lp");
      if(temp_v[mod2Plot][s].size()!=0) mg2->Add(gr2[mod2Plot][s],"lp");
      if(calTemp_v[mod2Plot][s].size()!=0) mg3->Add(gr3[mod2Plot][s],"lp");
      if(calTemp_v[mod2Plot][s].size()!=0) mg4->Add(gr4[mod2Plot][s],"lp");
    }
    mg1->SetTitle(Form("Module %i Temperature Log ; Date/Time ;Temperature (C)",mod2Plot));
    mg2->SetTitle(Form("Module %i Temperature Log ; Run # ;Temperature (C)",mod2Plot));
    mg3->SetTitle(Form("Module %i Temperature Log for Calibration Runs ; Date/Time ;Temperature (C)",mod2Plot));
    mg4->SetTitle(Form("Module %i Temperature Log for Calibration Runs ; Run # ;Temperature (C)",mod2Plot));
  }
  mg1->GetXaxis()->SetTimeDisplay(1);
  mg1->GetXaxis()->SetNdivisions(503,0);
  mg1->GetXaxis()->SetTimeFormat("%Y-%m-%d %H:%M");
  mg1->GetXaxis()->SetTimeOffset(0,"gmt");
  mg3->GetXaxis()->SetTimeDisplay(1);
  mg3->GetXaxis()->SetNdivisions(503,0);
  mg3->GetXaxis()->SetTimeFormat("%Y-%m-%d %H:%M");
  mg3->GetXaxis()->SetTimeOffset(0,"gmt");

  c1->cd(1);
  mg1->Draw("a pmc plc");
  c1->GetPad(1)->BuildLegend();
  c1->cd(2);
  mg2->Draw("a pmc plc");
  c1->GetPad(2)->BuildLegend();
  c1->cd(3);
  mg3->Draw("a pmc plc");
  c1->GetPad(3)->BuildLegend();
  c1->cd(4);
  mg4->Draw("a pmc plc");
  c1->GetPad(4)->BuildLegend();

  TCanvas* c2=new TCanvas("c2","Run Number to Time Conversion");
  TGraph* gr=new TGraph(run_v.size(),&datime_v[0],&run_v[0]);
  gr->GetXaxis()->SetTimeDisplay(1);
  gr->GetXaxis()->SetNdivisions(503,0);
  gr->GetXaxis()->SetTimeFormat("%Y-%m-%d %H:%M");
  gr->GetXaxis()->SetTimeOffset(0,"gmt");
  gr->SetTitle("Run Number to Time Conversion ; Date/Time ; Run #");
  gr->Draw("al*");

}
