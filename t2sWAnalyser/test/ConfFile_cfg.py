import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
        'root://cms-xrdr.sdfarm.kr:1094///xrd/store/mc/RunIISummer16DR80Premix/TT_TuneCUETP8M2T4_13TeV-powheg-pythia8/AODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/120000/00047D71-E7BA-E611-A259-001E4F1B8E39.root'
    )
)

process.TFileService = cms.Service("TFileService",fileName = cms.string("out.root"))

process.demo = cms.EDAnalyzer('t2sWAnalyser',
    vertex        = cms.InputTag('offlinePrimaryVertices'),
    genParticle   = cms.InputTag('genParticles'),
    Kshort_V0     = cms.InputTag('generalV0Candidates', 'Kshort'),
    Lambda_V0     = cms.InputTag('generalV0Candidates', 'Lambda')
)


process.p = cms.Path(process.demo)
