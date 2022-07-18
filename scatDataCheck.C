{
  TCanvas* c1=new TCanvas("c1","c1");
  c1->Divide(1,2);
  c1->cd(1);
  TH1F* energyScat_h=new TH1F("energyScat_h","energyScat_h",2000,0,2000);
  TH1F* energyScatGood_h=new TH1F("energyScatGood_h","energyScatGood_h",2000,0,2000);
  tree2->Draw("energyScat>>energyScat_h","energyScat!=0");
  tree2->Draw("energyScat>>energyScatGood_h","energyScat!=0 && hitsAM0==2 && hitsAM1==2 && energyAM0>400 && energyAM0<700 && energyAM1>400 && energyAM1<700");
  c1->cd(1);
  energyScat_h->Draw();
  c1->cd(2);
  energyScatGood_h->SetLineColor(2);
  energyScatGood_h->Draw("");
  c1->cd(1);
  energyScatGood_h->Draw("same");

}
