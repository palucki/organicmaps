#include "routing_common/bicycle_model.hpp"

#include "indexer/classificator.hpp"
#include "indexer/feature.hpp"

namespace bicycle_model
{
using namespace routing;

// See model specifics in different countries here:
//   https://wiki.openstreetmap.org/wiki/OSM_tags_for_routing/Access-Restrictions
// Document contains proposals for some countries, but we assume that some kinds of roads are ready for bicycle routing,
// but not listed in tables in the document. For example, steps are not listed, paths, roads and services features also
// can be treated as ready for bicycle routing. These road types were added to lists below.

// See road types here:
//   https://wiki.openstreetmap.org/wiki/Key:highway

// Heuristics:
// For less bicycle roads we add fine by setting smaller value of weight speed, and for more bicycle roads we
// set greater values of weight speed. Algorithm picks roads with greater weight speed first,
// preferencing a more bicycle roads over less bicycle.
// As result of such heuristic road is not totally the shortest, but it avoids non bicycle roads, which were
// not marked as "hwtag=nobicycle" in OSM.

HighwayBasedFactors const kDefaultFactors = GetOneFactorsForBicycleAndPedestrianModel();

SpeedKMpH constexpr kSpeedOffroadKMpH = {1.5 /* weight */, 3.0 /* eta */};
SpeedKMpH constexpr kSpeedDismountKMpH = {2.0 /* weight */, 2.0 /* eta */};
SpeedKMpH constexpr kSpeedOnFootwayKMpH = {5.0 /* weight */, 7.0 /* eta */};

HighwayBasedSpeeds const kDefaultSpeeds = {
    // {highway class : InOutCitySpeedKMpH(in city(weight, eta), out city(weight eta))}
    {HighwayType::HighwayTrunk, InOutCitySpeedKMpH(SpeedKMpH(3.0, 18.0))},
    {HighwayType::HighwayTrunkLink, InOutCitySpeedKMpH(SpeedKMpH(3.0, 18.0))},
    {HighwayType::HighwayPrimary, InOutCitySpeedKMpH(SpeedKMpH(10.0, 18.0), SpeedKMpH(14.0, 18.0))},
    {HighwayType::HighwayPrimaryLink, InOutCitySpeedKMpH(SpeedKMpH(10.0, 18.0), SpeedKMpH(14.0, 18.0))},
    {HighwayType::HighwaySecondary, InOutCitySpeedKMpH(SpeedKMpH(15.0, 18.0), SpeedKMpH(20.0, 18.0))},
    {HighwayType::HighwaySecondaryLink, InOutCitySpeedKMpH(SpeedKMpH(15.0, 18.0), SpeedKMpH(20.0, 18.0))},
    {HighwayType::HighwayTertiary, InOutCitySpeedKMpH(SpeedKMpH(15.0, 18.0), SpeedKMpH(20.0, 18.0))},
    {HighwayType::HighwayTertiaryLink, InOutCitySpeedKMpH(SpeedKMpH(15.0, 18.0), SpeedKMpH(20.0, 18.0))},
    {HighwayType::HighwayService, InOutCitySpeedKMpH(SpeedKMpH(12.0, 18.0))},
    {HighwayType::HighwayUnclassified, InOutCitySpeedKMpH(SpeedKMpH(12.0, 18.0))},
    {HighwayType::HighwayRoad, InOutCitySpeedKMpH(SpeedKMpH(10.0, 12.0))},
    {HighwayType::HighwayTrack, InOutCitySpeedKMpH(SpeedKMpH(8.0, 12.0))},
    {HighwayType::HighwayPath, InOutCitySpeedKMpH(SpeedKMpH(6.0, 12.0))},
    {HighwayType::HighwayBridleway, InOutCitySpeedKMpH(SpeedKMpH(4.0, 12.0))},
    {HighwayType::HighwayCycleway, InOutCitySpeedKMpH(SpeedKMpH(30.0, 20.0))},
    {HighwayType::HighwayResidential, InOutCitySpeedKMpH(SpeedKMpH(8.0, 10.0))},
    {HighwayType::HighwayLivingStreet, InOutCitySpeedKMpH(SpeedKMpH(7.0, 8.0))},
    // Steps have obvious inconvenience of a bike in hands.
    {HighwayType::HighwaySteps, InOutCitySpeedKMpH(SpeedKMpH(1.0, 1.0))},
    {HighwayType::HighwayPedestrian, InOutCitySpeedKMpH(kSpeedDismountKMpH)},
    {HighwayType::HighwayFootway, InOutCitySpeedKMpH(kSpeedDismountKMpH)},
    {HighwayType::ManMadePier, InOutCitySpeedKMpH(kSpeedOnFootwayKMpH)},
    {HighwayType::RouteFerry, InOutCitySpeedKMpH(SpeedKMpH(3.0, 20.0))},
};

// Default, no bridleway.
VehicleModel::LimitsInitList const kDefaultOptions = {
    // {HighwayType, passThroughAllowed}
    {HighwayType::HighwayTrunk, true},
    {HighwayType::HighwayTrunkLink, true},
    {HighwayType::HighwayPrimary, true},
    {HighwayType::HighwayPrimaryLink, true},
    {HighwayType::HighwaySecondary, true},
    {HighwayType::HighwaySecondaryLink, true},
    {HighwayType::HighwayTertiary, true},
    {HighwayType::HighwayTertiaryLink, true},
    {HighwayType::HighwayService, true},
    {HighwayType::HighwayUnclassified, true},
    {HighwayType::HighwayRoad, true},
    {HighwayType::HighwayTrack, true},
    {HighwayType::HighwayPath, true},
    // HighwayBridleway is missing
    {HighwayType::HighwayCycleway, true},
    {HighwayType::HighwayResidential, true},
    {HighwayType::HighwayLivingStreet, true},
    {HighwayType::HighwaySteps, true},
    {HighwayType::HighwayPedestrian, true},
    {HighwayType::HighwayFootway, true},
    {HighwayType::ManMadePier, true},
    {HighwayType::RouteFerry, true}
};

// Same as defaults except trunk and trunk_link are not allowed
VehicleModel::LimitsInitList NoTrunk()
{
  VehicleModel::LimitsInitList res;
  res.reserve(kDefaultOptions.size() - 2);
  for (auto const & e : kDefaultOptions)
  {
    if (e.m_type != HighwayType::HighwayTrunk && e.m_type != HighwayType::HighwayTrunkLink)
      res.push_back(e);
  }
  return res;
}

// Same as defaults except pedestrian is allowed
HighwayBasedSpeeds NormalPedestrianSpeed()
{
  HighwayBasedSpeeds res = kDefaultSpeeds;
  res.Replace(HighwayType::HighwayPedestrian, InOutCitySpeedKMpH(kSpeedOnFootwayKMpH));
  return res;
}

// Same as defaults except bridleway is allowed
VehicleModel::LimitsInitList AllAllowed()
{
  auto res = kDefaultOptions;
  res.push_back({HighwayType::HighwayBridleway, true});
  return res;
}

// Same as defaults except pedestrian and footway are allowed
HighwayBasedSpeeds NormalPedestrianAndFootwaySpeed()
{
  HighwayBasedSpeeds res = kDefaultSpeeds;
  InOutCitySpeedKMpH const footSpeed(kSpeedOnFootwayKMpH);
  res.Replace(HighwayType::HighwayPedestrian, footSpeed);
  res.Replace(HighwayType::HighwayFootway, footSpeed);
  return res;
}

HighwayBasedSpeeds DismountPathSpeed()
{
  HighwayBasedSpeeds res = kDefaultSpeeds;
  res.Replace(HighwayType::HighwayPath, InOutCitySpeedKMpH(kSpeedDismountKMpH));
  return res;
}

HighwayBasedSpeeds PreferFootwaysToRoads()
{
  HighwayBasedSpeeds res = kDefaultSpeeds;

  // Decrease secondary/tertiary weight speed (-20% from default).
  InOutCitySpeedKMpH roadSpeed = InOutCitySpeedKMpH(SpeedKMpH(12.0, 18.0), SpeedKMpH(16.0, 18.0));
  res.Replace(HighwayType::HighwaySecondary, roadSpeed);
  res.Replace(HighwayType::HighwaySecondaryLink, roadSpeed);
  res.Replace(HighwayType::HighwayTertiary, roadSpeed);
  res.Replace(HighwayType::HighwayTertiaryLink, roadSpeed);

  // Increase footway speed to make bigger than other roads (+20% from default roads).
  InOutCitySpeedKMpH footSpeed = InOutCitySpeedKMpH(SpeedKMpH(18.0, 18.0), SpeedKMpH(20.0, 18.0));
  res.Replace(HighwayType::HighwayPedestrian, footSpeed);
  res.Replace(HighwayType::HighwayFootway, footSpeed);

  return res;
}

// No trunk, No pass through living_street and service
VehicleModel::LimitsInitList UkraineOptions()
{
  auto res = NoTrunk();
  for (auto & e : res)
  {
    if (e.m_type == HighwayType::HighwayLivingStreet || e.m_type == HighwayType::HighwayService)
      e.m_isPassThroughAllowed = false;
  }
  return res;
}

VehicleModel::SurfaceInitList const kBicycleSurface = {
  // {{surfaceType, surfaceType}, {weightFactor, etaFactor}}
  {{"psurface", "paved_good"}, {1.0, 1.0}},
  {{"psurface", "paved_bad"}, {0.8, 0.8}},
  {{"psurface", "unpaved_good"}, {1.0, 1.0}},
  {{"psurface", "unpaved_bad"}, {0.3, 0.3}}
};
}  // namespace bicycle_model

namespace routing
{
BicycleModel::BicycleModel()
  : BicycleModel(bicycle_model::kDefaultOptions)
{
}

BicycleModel::BicycleModel(VehicleModel::LimitsInitList const & limits)
  : BicycleModel(limits, bicycle_model::kDefaultSpeeds)
{
}

BicycleModel::BicycleModel(VehicleModel::LimitsInitList const & limits, HighwayBasedSpeeds const & speeds)
  : VehicleModel(classif(), limits, bicycle_model::kBicycleSurface, {speeds, bicycle_model::kDefaultFactors})
{
  // No bridleway in default.
  ASSERT_EQUAL(bicycle_model::kDefaultOptions.size(), bicycle_model::kDefaultSpeeds.size() - 1, ());

  std::vector<std::string> hwtagYesBicycle = {"hwtag", "yesbicycle"};

  auto const & cl = classif();
  m_noType = cl.GetTypeByPath({"hwtag", "nobicycle"});
  m_yesType = cl.GetTypeByPath(hwtagYesBicycle);
  m_bidirBicycleType = cl.GetTypeByPath({"hwtag", "bidir_bicycle"});
  m_onedirBicycleType = cl.GetTypeByPath({"hwtag", "onedir_bicycle"});

  // Assign 90% of max cycleway speed for bicycle=yes to keep choosing most preferred cycleway.
  double const factor = 0.9;
  AddAdditionalRoadTypes(cl, {
      {std::move(hwtagYesBicycle), {m_maxModelSpeed.m_inCity * factor, m_maxModelSpeed.m_outCity * factor}}
  });
}

bool BicycleModel::IsBicycleBidir(feature::TypesHolder const & types) const
{
  return types.Has(m_bidirBicycleType);
}

bool BicycleModel::IsBicycleOnedir(feature::TypesHolder const & types) const
{
  return types.Has(m_onedirBicycleType);
}

SpeedKMpH BicycleModel::GetSpeed(FeatureType & f, SpeedParams const & speedParams) const
{
  return VehicleModel::GetSpeedWihtoutMaxspeed(f, speedParams);
}

bool BicycleModel::IsOneWay(FeatureType & f) const
{
  feature::TypesHolder const types(f);

  if (IsBicycleOnedir(types))
    return true;

  if (IsBicycleBidir(types))
    return false;

  return VehicleModel::IsOneWay(f);
}

SpeedKMpH const & BicycleModel::GetOffroadSpeed() const { return bicycle_model::kSpeedOffroadKMpH; }

// If one of feature types will be disabled for bicycles, features of this type will be simplified
// in generator. Look FeatureBuilder1::IsRoad() for more details.
// static
BicycleModel const & BicycleModel::AllLimitsInstance()
{
  static BicycleModel const instance(bicycle_model::AllAllowed(), bicycle_model::NormalPedestrianAndFootwaySpeed());
  return instance;
}

// static
SpeedKMpH BicycleModel::DismountSpeed()
{
  return bicycle_model::kSpeedDismountKMpH;
}

BicycleModelFactory::BicycleModelFactory(
    CountryParentNameGetterFn const & countryParentNameGetterFn)
  : VehicleModelFactory(countryParentNameGetterFn)
{
  using namespace bicycle_model;
  using std::make_shared;

  // Names must be the same with country names from countries.txt
  m_models[""] = make_shared<BicycleModel>(kDefaultOptions);

  m_models["Australia"] = make_shared<BicycleModel>(AllAllowed(), NormalPedestrianAndFootwaySpeed());
  m_models["Austria"] = make_shared<BicycleModel>(NoTrunk(), DismountPathSpeed());
  // Belarus law demands to use footways for bicycles where possible.
  m_models["Belarus"] = make_shared<BicycleModel>(kDefaultOptions, PreferFootwaysToRoads());
  m_models["Belgium"] = make_shared<BicycleModel>(NoTrunk(), NormalPedestrianSpeed());
  m_models["Brazil"] = make_shared<BicycleModel>(AllAllowed());
  m_models["Denmark"] = make_shared<BicycleModel>(NoTrunk());
  m_models["France"] = make_shared<BicycleModel>(NoTrunk(), NormalPedestrianSpeed());
  m_models["Finland"] = make_shared<BicycleModel>(kDefaultOptions, NormalPedestrianSpeed());
  m_models["Hungary"] = make_shared<BicycleModel>(NoTrunk());
  m_models["Iceland"] = make_shared<BicycleModel>(AllAllowed(), NormalPedestrianAndFootwaySpeed());
  m_models["Ireland"] = make_shared<BicycleModel>(AllAllowed());
  m_models["Italy"] = make_shared<BicycleModel>(kDefaultOptions, NormalPedestrianSpeed());
  m_models["Netherlands"] = make_shared<BicycleModel>(NoTrunk());
  m_models["Norway"] = make_shared<BicycleModel>(AllAllowed(), NormalPedestrianAndFootwaySpeed());
  m_models["Oman"] = make_shared<BicycleModel>(AllAllowed());
  m_models["Philippines"] = make_shared<BicycleModel>(AllAllowed(), NormalPedestrianSpeed());
  m_models["Poland"] = make_shared<BicycleModel>(NoTrunk());
  m_models["Romania"] = make_shared<BicycleModel>(AllAllowed());
  // Note. Despite the fact that according to https://wiki.openstreetmap.org/wiki/OSM_tags_for_routing/Access-Restrictions
  // passing through service and living_street with a bicycle is prohibited it's allowed according to Russian traffic rules.
  m_models["Russian Federation"] = make_shared<BicycleModel>(kDefaultOptions, NormalPedestrianAndFootwaySpeed());
  m_models["Slovakia"] = make_shared<BicycleModel>(NoTrunk());
  m_models["Spain"] = make_shared<BicycleModel>(NoTrunk(), NormalPedestrianSpeed());
  m_models["Sweden"] = make_shared<BicycleModel>(kDefaultOptions, NormalPedestrianSpeed());
  m_models["Switzerland"] = make_shared<BicycleModel>(NoTrunk(), NormalPedestrianAndFootwaySpeed());
  m_models["Ukraine"] = make_shared<BicycleModel>(UkraineOptions());
  m_models["United Kingdom"] = make_shared<BicycleModel>(AllAllowed());
  m_models["United States of America"] = make_shared<BicycleModel>(AllAllowed(), NormalPedestrianSpeed());
}
}  // routing
