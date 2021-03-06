#include <vector>
#include <iostream>
#include <assert.h>

#include <IOADatabase.hxx>
#include <IFictitiousPlane.hxx>
#include <IReconTrack.hxx>
#include <IReconTrackCand.hxx>

#include <ICOMETLog.hxx>
#include <ICOMETEvent.hxx>
#include <IHitSelection.hxx>
#include <IMCHit.hxx>
#include <IHit.hxx>
#include <IGeomInfo.hxx>
#include <IFieldManager.hxx>
#include <IGeoField.hxx>

#include <TCanvas.h>
#include <TH1F.h>
#include <TFile.h>
#include <TTree.h>
#include <TDirectory.h>
#include "TEllipse.h"


IFictitiousPlane::IFictitiousPlane(const char* name, const char* title)
  :ITracking(name, title)
{;}

IFictitiousPlane::~IFictitiousPlane()
{;}

int IFictitiousPlane::Init(){
  COMETInfo("//----------------------------------------------------------//");
  COMETInfo("// Initialize IFictitiousPlane");

  return 0;
}

int IFictitiousPlane::BeginOfEvent(){
  /// Field Map
  return 0;
}

int IFictitiousPlane::EndOfEvent()
{
  return 0;
}    

void IFictitiousPlane::LoadHitsAfterHT(COMET::IHandle<COMET::IHitSelection> hitHandle, IHoughTransform* hough){  
  std::vector<int> wireId       = hough->GetRecoWireId();
  std::vector<double> driftDist = hough->GetRecoDriftDist();
  std::vector<int> domain       = hough->GetRecoDomain();
  std::vector<bool> side        = hough->GetRecoSideHit();

  fnCALCDCHit=wireId.size();

  // Clear the arrays
  memset (fWireId,-1,sizeof(fWireId));
  memset (fWireLayerId,-1,sizeof(fWireLayerId));
  memset (fWireEnd0X,-1,sizeof(fWireEnd0X));
  memset (fWireEnd0Y,-1,sizeof(fWireEnd0Y));
  memset (fWireEnd0Z,-1,sizeof(fWireEnd0Z));
  memset (fWireEnd1X,-1,sizeof(fWireEnd1X));
  memset (fWireEnd1Y,-1,sizeof(fWireEnd1Y));
  memset (fWireEnd1Z,-1,sizeof(fWireEnd1Z));
  memset (fDriftDist,-1,sizeof(fDriftDist));
  memset (fDomain,-1,sizeof(fDomain));
  memset (fSide,-1,sizeof(fSide));


  assert(fnCALCDCHit==hough->GetNumberOfRecognizedHits());
  assert(wireId.size() == driftDist.size());
  assert(wireId.size() == domain.size());

  for (int i=0; i<fnCALCDCHit; i++){
    int wire = wireId.at(i);
    fWireId[i]      = wire;
    fWireLayerId[i] = COMET::IGeomInfo::Get().CDC().GetLayer(wire);
    fWireEnd0X[i]   = COMET::IGeomInfo::Get().CDC().GetWireEnd0(wire).X();
    fWireEnd0Y[i]   = COMET::IGeomInfo::Get().CDC().GetWireEnd0(wire).Y();
    fWireEnd0Z[i]   = COMET::IGeomInfo::Get().CDC().GetWireEnd0(wire).Z();
    fWireEnd1X[i]   = COMET::IGeomInfo::Get().CDC().GetWireEnd1(wire).X();
    fWireEnd1Y[i]   = COMET::IGeomInfo::Get().CDC().GetWireEnd1(wire).Y();
    fWireEnd1Z[i]   = COMET::IGeomInfo::Get().CDC().GetWireEnd1(wire).Z();
    fDriftDist[i]   = driftDist.at(i);
    fDomain[i]      = domain.at(i);
    fSide[i]        = side.at(i);
    //std::cout << i << "   " << fWireEnd0X[i] << "   " << fWireEnd0Y[i] << "   " << fWireEnd0Z[i] << "   " << fDomain[i] << std::endl;
  }
}  

void IFictitiousPlane::AddRandomHitPairs(int n, int domain){
  assert(n<=4);
  for (int i=0; i<n; i++){ 
   
    struct SingleHit hit_lo, hit_up;
    struct HitPair hitPair;

    std::vector<int> lowerIndicies;
    std::vector<int> upperIndicies;
    for (int i_hit=0; i_hit<fnCALCDCHit; i_hit++){
      if      (fWireLayerId[i_hit]==i   && fDomain[i_hit]==domain) lowerIndicies.push_back(i_hit);
      else if (fWireLayerId[i_hit]==i+1 && fDomain[i_hit]==domain) upperIndicies.push_back(i_hit);
    }
    //if (lowerIndicies.empty() || upperIndicies.empty()) continue;

    int i_lo = lowerIndicies.at(rand() % lowerIndicies.size());
    int i_up = upperIndicies.at(rand() % upperIndicies.size());

    TVector3 wireEnd0_lo = TVector3(fWireEnd0X[i_lo], fWireEnd0Y[i_lo], fWireEnd0Z[i_lo]);
    TVector3 wireEnd1_lo = TVector3(fWireEnd1X[i_lo], fWireEnd1Y[i_lo], fWireEnd1Z[i_lo]);

    TVector3 wireEnd0_up = TVector3(fWireEnd0X[i_up], fWireEnd0Y[i_up], fWireEnd0Z[i_up]);
    TVector3 wireEnd1_up = TVector3(fWireEnd1X[i_up], fWireEnd1Y[i_up], fWireEnd1Z[i_up]);

    hit_lo.wireEnd0  = wireEnd0_lo;
    hit_lo.wireEnd1  = wireEnd1_lo;
    hit_lo.driftDist = fDriftDist[i_lo];
    hit_lo.wireId    = fWireId[i_lo];
    hit_lo.layerId   = fWireLayerId[i_lo];
    hit_lo.domain    = fDomain[i_lo];

    hit_up.wireEnd0  = wireEnd0_up;
    hit_up.wireEnd1  = wireEnd1_up;
    hit_up.driftDist = fDriftDist[i_up];
    hit_up.wireId    = fWireId[i_up];
    hit_up.layerId   = fWireLayerId[i_up];
    hit_up.domain    = fDomain[i_up];

    // If fHitPairs is not empty, use the previous h2 as h1
    if (i!=0){ 
      hit_lo = (fHitPairs.at(i-1)).h2;
      //if (domain==2) hit_lo = (fHitPairsDomain2.at(i-1)).h2;
    }

    TVector3 POCA = GetPOCAofTwoWires(hit_lo.wireEnd0, hit_lo.wireEnd1, hit_up.wireEnd0, hit_up.wireEnd1);
    TVector3 cVec = GetVectorCrossingCenter(hit_lo.wireEnd0, hit_lo.wireEnd1, hit_up.wireEnd0, hit_up.wireEnd1,POCA);

    hitPair.h1 = hit_lo;
    hitPair.h2 = hit_up;
    hitPair.cV = cVec;

    fHitPairs.push_back(hitPair);
    //if (domain==2) fHitPairsDomain2.push_back(hitPair);
  }
}

void IFictitiousPlane::AddSideHitPairs(int n, int domain){
  assert(n<=4);
  for (int i=0; i<n; i++){ 
   
    struct SingleHit hit_lo, hit_up;
    struct HitPair hitPair;

    int i_lo;
    int i_up;
    for (int i_hit=0; i_hit<fnCALCDCHit; i_hit++){
      if      (fWireLayerId[i_hit]==i   && fDomain[i_hit]==domain && fSide[i_hit]==1) i_lo=i_hit;
      else if (fWireLayerId[i_hit]==i+1 && fDomain[i_hit]==domain && fSide[i_hit]==1) i_up=i_hit;
    }
    //if (lowerIndicies.empty() || upperIndicies.empty()) continue;

    TVector3 wireEnd0_lo = TVector3(fWireEnd0X[i_lo], fWireEnd0Y[i_lo], fWireEnd0Z[i_lo]);
    TVector3 wireEnd1_lo = TVector3(fWireEnd1X[i_lo], fWireEnd1Y[i_lo], fWireEnd1Z[i_lo]);

    TVector3 wireEnd0_up = TVector3(fWireEnd0X[i_up], fWireEnd0Y[i_up], fWireEnd0Z[i_up]);
    TVector3 wireEnd1_up = TVector3(fWireEnd1X[i_up], fWireEnd1Y[i_up], fWireEnd1Z[i_up]);

    hit_lo.wireEnd0  = wireEnd0_lo;
    hit_lo.wireEnd1  = wireEnd1_lo;
    hit_lo.driftDist = fDriftDist[i_lo];
    hit_lo.wireId    = fWireId[i_lo];
    hit_lo.layerId   = fWireLayerId[i_lo];
    hit_lo.domain    = fDomain[i_lo];

    hit_up.wireEnd0  = wireEnd0_up;
    hit_up.wireEnd1  = wireEnd1_up;
    hit_up.driftDist = fDriftDist[i_up];
    hit_up.wireId    = fWireId[i_up];
    hit_up.layerId   = fWireLayerId[i_up];
    hit_up.domain    = fDomain[i_up];

    // If fHitPairs is not empty, use the previous h2 as h1
    if (i!=0){ 
      hit_lo = (fHitPairs.at(i-1)).h2;
      //if (domain==2) hit_lo = (fHitPairsDomain2.at(i-1)).h2;
    }

    TVector3 POCA = GetPOCAofTwoWires(hit_lo.wireEnd0, hit_lo.wireEnd1, hit_up.wireEnd0, hit_up.wireEnd1);
    TVector3 cVec = GetVectorCrossingCenter(hit_lo.wireEnd0, hit_lo.wireEnd1, hit_up.wireEnd0, hit_up.wireEnd1,POCA);

    hitPair.h1 = hit_lo;
    hitPair.h2 = hit_up;
    hitPair.cV = cVec;

    fHitPairs.push_back(hitPair);
    //if (domain==2) fHitPairsDomain2.push_back(hitPair);
  }
  std::cout << "Initial Position: " << fCDCEnterX << "  " << fCDCEnterY << "  " << fCDCEnterZ << std::endl;
  std::cout << "Initial Momentum: " << fCDCEnterPx << "  " << fCDCEnterPy << "  " << fCDCEnterPz << std::endl;
}

void IFictitiousPlane::DrawHitsOnFictitiousPlane(TCanvas* canvas){
  canvas->cd();
  canvas->DrawFrame(-10,-10,10,10);
  TVector2 ref;
  ref = TVector2(0.,0.);
  for (int i=0; i<fHitPairs.size() ; i++){
    if (i==0){
      double d_lo = ((fHitPairs.at(i)).h1).driftDist;
      TEllipse *HitCircle_lo = new TEllipse(ref.X(),ref.Y(), d_lo,d_lo);
      std::cout << ref.X() << "  " << ref.Y() << "  " << d_lo << std::endl;
      HitCircle_lo->Draw();
    }
    TVector3 cVec = fHitPairs.at(i).cV;
    ref = TVector2(ref.X()+cVec(1), ref.Y()+cVec(2));
    double d_up = ((fHitPairs.at(i)).h2).driftDist;
    TEllipse *HitCircle_up = new TEllipse(ref.X(),ref.Y(), d_up,d_up);
    std::cout << ref.X() << "  " << ref.Y() << "  " << d_up << std::endl;
    HitCircle_up->Draw();
  }
}

/*
TVector3 GetPOCAofTwoWires(TVector3 wireEnd0_lo, TVector3 wireEnd1_lo, TVector3 wireEnd0_up, TVector3 wireEnd1_up){
  TVector3 u = wireEnd1_lo-wireEnd0_lo;
  TVector3 v = wireEnd1_up-wireEnd0_up;
  TVector3 w = wireEnd0_lo-wireEnd0_up;
  double a,b,c,d,e,s,t;
  a = u * u; 
  b = u * v;
  c = v * v;
  d = u * w;
  e = v * w;
  
  s = (b*e-c*d)/(a*c-b*b);
  t = (a*e-b*d)/(a*c-b*b);

  TVector3 pC=wireEnd0_lo + s * u;
  TVector3 qC=wireEnd0_up + t * v;

  // DOCA 
  //std::cout << "DOCA: " << (w+s*u-t*v).Mag() << std::endl;
  //std::cout << "pC: " << pC(0) << "  " << pC(1) << "  " << pC(2) << std::endl;
  //std::cout << "qC: " << qC(0) << "  " << qC(1) << "  " << qC(2) << std::endl;
  
  return (pC+qC)*0.5;
}

TVector3 GetVectorCrossingCenter(TVector3 wireEnd0_lo, TVector3 wireEnd1_lo, TVector3 wireEnd0_up, TVector3 wireEnd1_up, TVector3 POCA){
  TVector3 u = wireEnd1_lo-wireEnd0_lo;
  TVector3 v = wireEnd1_up-wireEnd0_up;
  
  double u_t   = (POCA(0)-wireEnd0_lo(0))/(wireEnd1_lo(0)-wireEnd0_lo(0));
  double v_t   = (POCA(0)-wireEnd0_up(0))/(wireEnd1_up(0)-wireEnd0_up(0));

  TVector3 c1 = wireEnd0_lo + u_t*u;
  TVector3 c2 = wireEnd0_up + v_t*v;

  //std::cout << "CVector: " << c1(0)-c2(0) << "  " << c1(1)-c2(1) << "  " << c1(2)-c2(2) << std::endl;
  return c2-c1;
}
*/
