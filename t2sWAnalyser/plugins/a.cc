std::vector<std::pair<int, int> > GEMSimpleModel::simulateClustering(const GEMEtaPartition* roll,
                                     const PSimHit* simHit, const int bx,
                                     CLHEP::HepRandomEngine* engine)
{
  const LocalPoint& hit_entry(simHit->entryPoint());
  const LocalPoint& hit_exit(simHit->exitPoint());

  float hit_entry_smeardX = hit_entry.x()-fabs(CLHEP::RandGaussQ::shoot(0, 0.1));
  float hit_exit_smeardX = hit_exit.x()+fabs(CLHEP::RandGaussQ::shoot(0, 0.1));
  if (hit_entry.x()>hit_exit.x()) {
    hit_entry_smeardX = hit_entry.x()+fabs(CLHEP::RandGaussQ::shoot(0, 0.1));
    hit_exit_smeardX = hit_exit.x()-fabs(CLHEP::RandGaussQ::shoot(0, 0.1));
  }

  LocalPoint inPoint(hit_entry_smeardX, hit_entry.y(), hit_entry.z());
  LocalPoint outPoint(hit_exit_smeardX, hit_exit.y(), hit_exit.z());

  int clusterStart = roll->strip(inPoint);
  int clusterEnd = roll->strip(outPoint);

  std::vector < std::pair<int, int> > cluster_;
  cluster_.clear();
  for (int i = clusterStart; i<= clusterEnd ; i++) {
    cluster_.emplace_back(i, bx);
  }

  return cluster_;
}
