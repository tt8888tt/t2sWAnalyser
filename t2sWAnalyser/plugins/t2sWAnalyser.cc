// -*- C++ -*-
//
// Package:    t2sWAnalyser/t2sWAnalyser
// Class:      t2sWAnalyser
// 
/**\class t2sWAnalyser t2sWAnalyser.cc t2sWAnalyser/t2sWAnalyser/plugins/t2sWAnalyser.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  DaJeong Jeon
//         Created:  Tue, 26 Dec 2017 08:50:08 GMT
//
//
#include <memory>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/Candidate/interface/VertexCompositeCandidate.h"

#include <TTree.h>
#include <TFile.h>
#include <TLorentzVector.h>

using namespace std;
using namespace reco;
using namespace edm;

class t2sWAnalyser : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit t2sWAnalyser(const edm::ParameterSet&);
      ~t2sWAnalyser();

   private:
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;

      edm::EDGetTokenT<std::vector<reco::Vertex> > vertexToken_;
      edm::EDGetTokenT<reco::GenParticleCollection > genToken_;
      edm::EDGetTokenT<std::vector<VertexCompositeCandidate> > kshortToken_;
      edm::EDGetTokenT<std::vector<VertexCompositeCandidate> > lambdaToken_;

      TH1D* h_nevents;

      TTree* ttree_;

      float b_kshort_vz, b_pv_z;
      bool b_isFromTop, b_isFromSquark;
      TLorentzVector b_gen_kshort;

      TLorentzVector b_matched_kshort_gen, b_matched_kshort_reco;
};

t2sWAnalyser::t2sWAnalyser(const edm::ParameterSet& iConfig)
{
    vertexToken_ = consumes<std::vector<reco::Vertex>>(iConfig.getParameter<edm::InputTag>("vertex"));
    genToken_ = consumes<std::vector<reco::GenParticle>>(iConfig.getParameter<edm::InputTag>("genParticle"));
    kshortToken_ = consumes<std::vector<VertexCompositeCandidate>>(iConfig.getParameter<edm::InputTag>("Kshort_V0"));
    lambdaToken_ = consumes<std::vector<VertexCompositeCandidate>>(iConfig.getParameter<edm::InputTag>("Lambda_V0"));

    usesResource("TFileService");
    edm::Service<TFileService> fs;
    h_nevents = fs->make<TH1D>("nevents", "nevents", 1, 0, 1);
    ttree_ = fs->make<TTree>("tree", "tree");
    ttree_->Branch("kshort_vz", &b_kshort_vz, "kshort_vz/F");
    ttree_->Branch("pv_z", &b_pv_z, "pv_z/F");
    ttree_->Branch("isFromTop", &b_isFromTop, "isFromTop/O");
    ttree_->Branch("isFromSquark", &b_isFromSquark, "isFromSquark/O");
    ttree_->Branch("gen_kshort", "TLorentzVector", &b_gen_kshort);

    ttree_->Branch("matched_kshort_gen", "TLorentzVector", &b_matched_kshort_gen);
    ttree_->Branch("matched_kshort_reco", "TLorentzVector", &b_matched_kshort_reco);
}

t2sWAnalyser::~t2sWAnalyser(){}

void t2sWAnalyser::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    h_nevents->Fill(0.5);

    Handle<vector<reco::Vertex> > vertex;
    iEvent.getByToken(vertexToken_, vertex);

    Handle<reco::GenParticleCollection> genParticle;
    iEvent.getByToken(genToken_, genParticle);
    if (genParticle->size() == 0) { cout << "no gen" << endl; return; }

    Handle<vector<VertexCompositeCandidate>> kshort;
    Handle<vector<VertexCompositeCandidate>> lambda;
    iEvent.getByToken(kshortToken_, kshort);
    iEvent.getByToken(lambdaToken_, lambda);

    // gen
    vector<reco::GenParticle> gen_kshorts;
    for (unsigned i=0; i<genParticle->size(); i++) {
        auto gen = genParticle->at(i);
        if ( abs(gen.pdgId()) != 310) continue;
         
        b_isFromTop = false;
        b_isFromSquark = false;
        TLorentzVector b_matched_kshsort_gen;
        TLorentzVector b_matched_kshsort_reco;
        TLorentzVector gen_kshort;

        b_kshort_vz = gen.vz();
        b_pv_z = vertex->at(0).z();

        vector<const reco::Candidate*> mothers;
        auto mom = gen.mother(0);
        while (mom->numberOfMothers() != 0) {
            auto momtmp = mom->mother(0);
            if (momtmp->pdgId() != mom->pdgId() ) { mothers.push_back(momtmp); }
            mom = momtmp;
        }
        b_isFromTop = any_of(mothers.begin(), mothers.end(), [](auto m){return abs(m->pdgId()) == 6;});
        b_isFromSquark = any_of(mothers.begin(), mothers.end(), [](auto m){return abs(m->pdgId()) == 3;});

        if (!b_isFromTop || !b_isFromSquark) {ttree_->Fill(); continue; }
        TLorentzVector kshorttlv(gen.momentum().x(), gen.momentum().y(), gen.momentum().z(), gen.energy() );
        b_gen_kshort = kshorttlv;
        gen_kshorts.push_back(gen);
        if (kshort->size() == 0) {ttree_->Fill(); continue; }

        TLorentzVector gentlv;
        TLorentzVector recotlv;
        for (auto reco : *kshort) {
            if (reco::deltaR(gen, reco) > 0.5) continue;
            gentlv.SetPtEtaPhiE(gen.momentum().x(), gen.momentum().y(), gen.momentum().z(), gen.energy());
            recotlv.SetPtEtaPhiE(reco.momentum().x(), reco.momentum().y(), reco.momentum().z(), reco.energy());
            b_matched_kshort_gen = gentlv;
            b_matched_kshort_reco = recotlv;
        }
        ttree_->Fill();
    }

}

DEFINE_FWK_MODULE(t2sWAnalyser);
