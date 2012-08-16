#include <getopt.h>
#include <iostream>
#include <fstream>
#include <noise/noise.h>
#include "noiseutils.h"

using namespace noise;

double SOUTH_COORD = -90;
double NORTH_COORD = 90;
double WEST_COORD = -180;
double EAST_COORD = 180;
int GRID_WIDTH = 512;
int GRID_HEIGHT = 256;
int CUR_SEED = 1;
double PLANET_CIRCUMFERENCE = 10917000.0;
double MIN_ELEV = -3000.0;
double MAX_ELEV = 28000.0;
double CONTINENT_FREQUENCY = 1.0;
double CONTINENT_LACUNARITY = 2.208984375;
double MOUNTAIN_LACUNARITY = 2.142578125;
double HILLS_LACUNARITY = 2.162109375;
double PLAINS_LACUNARITY = 2.314453125;
double BADLANDS_LACUNARITY = 2.212890625;
double MOUNTAINS_TWIST = 1.0;
double HILLS_TWIST = 1.0;
double BADLANDS_TWIST = 1.0;
double SEA_LEVEL = 0.0;
double SHELF_LEVEL = -0.375;
double MOUNTAINS_AMOUNT = 0.5;
double BADLANDS_AMOUNT = 0.03125;
double TERRAIN_OFFSET = 1.0;
double MOUNTAIN_GLACIATION = 1.375;
double RIVER_DEPTH = 0.0234375;

// computed properties. will be set later
double HILLS_AMOUNT = (1.0 + MOUNTAINS_AMOUNT) / 2.0;
double CONTINENT_HEIGHT_SCALE = (1.0 - SEA_LEVEL) / 4.0;

utils::Image destImage;
utils::WriterBMP bitmapWriter;
utils::NoiseMapBuilderSphere planet;
utils::NoiseMap elevGrid;

double resInMeters;
double seaLevelInMeters;

char cachedir[1024];
char outfile[1024];

int main(int argc, char **argv);
void initParams(int argc, char **argv);
void createGeometry(void);
void getTopography(void);
void getNormals(void);
void getSpecular(void);
void getTexture(void);

int main (int argc, char **argv) {
  initParams(argc, argv);

  // computed properties
  double HILLS_AMOUNT = (1.0 + MOUNTAINS_AMOUNT) / 2.0;
  double CONTINENT_HEIGHT_SCALE = (1.0 - SEA_LEVEL) / 4.0;
  createGeometry();

  getTopography();
  getTexture();
  getSpecular();
  getNormals();

  return 0;
}

void initParams(int argc, char **argv) {
  static const char *optString = "a:b:c:d:e:f:g:h:i:j:k:l:m:n:o:p:q:r:s:t:u:v:w:x:y:z:A:B:C:";
  static const struct option longOpts[] = {
    {"north", required_argument, NULL, 'a'},
    {"south", required_argument, NULL, 'b'},
    {"east", required_argument, NULL, 'c'},
    {"west", required_argument, NULL, 'd'},
    {"width", required_argument, NULL, 'e'},
    {"height", required_argument, NULL, 'f'},
    {"seed", required_argument, NULL, 'g'},
    {"circumference", required_argument, NULL, 'h'},
    {"minelev", required_argument, NULL, 'i'},
    {"maxelev", required_argument, NULL, 'j'},
    {"continentsfreq", required_argument, NULL, 'k'},
    {"continentslacu", required_argument, NULL, 'l'},
    {"mountainslacu", required_argument, NULL, 'm'},
    {"mountainstwist", required_argument, NULL, 'n'},
    {"hillslacu", required_argument, NULL, 'o'},
    {"hillstwist", required_argument, NULL, 'p'},
    {"plainslacu", required_argument, NULL, 'q'},
    {"badlandslacu", required_argument, NULL, 'r'},
    {"badlandstwist", required_argument, NULL, 's'},
    {"sealevel", required_argument, NULL, 't'},
    {"shelflevel", required_argument, NULL, 'u'},
    {"mountainsamount", required_argument, NULL, 'v'},
    {"badlandsamount", required_argument, NULL, 'w'},
    {"terrainoffset", required_argument, NULL, 'x'},
    {"mountainglaciation", required_argument, NULL, 'y'},
    {"riverdepth", required_argument, NULL, 'z'},
    {"cachedir", required_argument, NULL, 'A'},
    {"outfile", required_argument, NULL, 'B'},
    {NULL, required_argument, NULL, 0}
  };

  int longIndex;
  int opt = getopt_long(argc, argv, optString, longOpts, &longIndex);

  while (opt != -1) {
    switch (opt) {
      case 'a': NORTH_COORD = strtod(optarg, NULL); break;
      case 'b': SOUTH_COORD = strtod(optarg, NULL); break;
      case 'c': EAST_COORD = strtod(optarg, NULL); break;
      case 'd': WEST_COORD = strtod(optarg, NULL); break;
      case 'e': GRID_WIDTH = atoi(optarg); break;
      case 'f': GRID_HEIGHT = atoi(optarg); break;
      case 'g': CUR_SEED = atoi(optarg); break;
      case 'h': PLANET_CIRCUMFERENCE = strtod(optarg, NULL); break;
      case 'i': MIN_ELEV = strtod(optarg, NULL); break;
      case 'j': MAX_ELEV = strtod(optarg, NULL); break;
      case 'k': CONTINENT_FREQUENCY = strtod(optarg, NULL); break;
      case 'l': CONTINENT_LACUNARITY = strtod(optarg, NULL); break;
      case 'm': MOUNTAIN_LACUNARITY = strtod(optarg, NULL); break;
      case 'n': MOUNTAINS_TWIST = strtod(optarg, NULL); break;
      case 'o': HILLS_LACUNARITY = strtod(optarg, NULL); break;
      case 'p': HILLS_TWIST = strtod(optarg, NULL); break;
      case 'q': PLAINS_LACUNARITY = strtod(optarg, NULL); break;
      case 'r': BADLANDS_LACUNARITY = strtod(optarg, NULL); break;
      case 's': BADLANDS_TWIST = strtod(optarg, NULL); break;
      case 't': SEA_LEVEL = strtod(optarg, NULL); break;
      case 'u': SHELF_LEVEL = strtod(optarg, NULL); break;
      case 'v': MOUNTAINS_AMOUNT = strtod(optarg, NULL); break;
      case 'w': BADLANDS_AMOUNT = strtod(optarg, NULL); break;
      case 'x': TERRAIN_OFFSET = strtod(optarg, NULL); break;
      case 'y': MOUNTAIN_GLACIATION = strtod(optarg, NULL); break;
      case 'z': RIVER_DEPTH = strtod(optarg, NULL); break;
      case 'A': strcpy(cachedir, optarg); break;
      case 'B': strcpy(outfile, optarg); break;
    }
    opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
  }

}

void createGeometry() {
  module::Perlin baseContinentDef_pe0;
  baseContinentDef_pe0.SetSeed (CUR_SEED + 0);
  baseContinentDef_pe0.SetFrequency (CONTINENT_FREQUENCY);
  baseContinentDef_pe0.SetPersistence (0.5);
  baseContinentDef_pe0.SetLacunarity (CONTINENT_LACUNARITY);
  baseContinentDef_pe0.SetOctaveCount (14);
  baseContinentDef_pe0.SetNoiseQuality (QUALITY_STD);

  module::Curve baseContinentDef_cu;
  baseContinentDef_cu.SetSourceModule (0, baseContinentDef_pe0);
  baseContinentDef_cu.AddControlPoint (-2.0000 + SEA_LEVEL,-1.625 + SEA_LEVEL);
  baseContinentDef_cu.AddControlPoint (-1.0000 + SEA_LEVEL,-1.375 + SEA_LEVEL);
  baseContinentDef_cu.AddControlPoint ( 0.0000 + SEA_LEVEL,-0.375 + SEA_LEVEL);
  baseContinentDef_cu.AddControlPoint ( 0.0625 + SEA_LEVEL, 0.125 + SEA_LEVEL);
  baseContinentDef_cu.AddControlPoint ( 0.1250 + SEA_LEVEL, 0.250 + SEA_LEVEL);
  baseContinentDef_cu.AddControlPoint ( 0.2500 + SEA_LEVEL, 1.000 + SEA_LEVEL);
  baseContinentDef_cu.AddControlPoint ( 0.5000 + SEA_LEVEL, 0.250 + SEA_LEVEL);
  baseContinentDef_cu.AddControlPoint ( 0.7500 + SEA_LEVEL, 0.250 + SEA_LEVEL);
  baseContinentDef_cu.AddControlPoint ( 1.0000 + SEA_LEVEL, 0.500 + SEA_LEVEL);
  baseContinentDef_cu.AddControlPoint ( 2.0000 + SEA_LEVEL, 0.500 + SEA_LEVEL);

  module::Perlin baseContinentDef_pe1;
  baseContinentDef_pe1.SetSeed (CUR_SEED + 1);
  baseContinentDef_pe1.SetFrequency (CONTINENT_FREQUENCY * 4.34375);
  baseContinentDef_pe1.SetPersistence (0.5);
  baseContinentDef_pe1.SetLacunarity (CONTINENT_LACUNARITY);
  baseContinentDef_pe1.SetOctaveCount (11);
  baseContinentDef_pe1.SetNoiseQuality (QUALITY_STD);

  module::ScaleBias baseContinentDef_sb;
  baseContinentDef_sb.SetSourceModule (0, baseContinentDef_pe1);
  baseContinentDef_sb.SetScale (0.375);
  baseContinentDef_sb.SetBias (0.625);

  module::Min baseContinentDef_mi;
  baseContinentDef_mi.SetSourceModule (0, baseContinentDef_sb);
  baseContinentDef_mi.SetSourceModule (1, baseContinentDef_cu);

  module::Clamp baseContinentDef_cl;
  baseContinentDef_cl.SetSourceModule (0, baseContinentDef_mi);
  baseContinentDef_cl.SetBounds (-1.0, 1.0);

  module::Cache baseContinentDef;
  baseContinentDef.SetSourceModule (0, baseContinentDef_cl);

  module::Turbulence continentDef_tu0;
  continentDef_tu0.SetSourceModule (0, baseContinentDef);
  continentDef_tu0.SetSeed (CUR_SEED + 10);
  continentDef_tu0.SetFrequency (CONTINENT_FREQUENCY * 15.25);
  continentDef_tu0.SetPower (CONTINENT_FREQUENCY / 113.75);
  continentDef_tu0.SetRoughness (13);

  module::Turbulence continentDef_tu1;
  continentDef_tu1.SetSourceModule (0, continentDef_tu0);
  continentDef_tu1.SetSeed (CUR_SEED + 11);
  continentDef_tu1.SetFrequency (CONTINENT_FREQUENCY * 47.25);
  continentDef_tu1.SetPower (CONTINENT_FREQUENCY / 433.75);
  continentDef_tu1.SetRoughness (12);

  module::Turbulence continentDef_tu2;
  continentDef_tu2.SetSourceModule (0, continentDef_tu1);
  continentDef_tu2.SetSeed (CUR_SEED + 12);
  continentDef_tu2.SetFrequency (CONTINENT_FREQUENCY * 95.25);
  continentDef_tu2.SetPower (CONTINENT_FREQUENCY / 1019.75);
  continentDef_tu2.SetRoughness (11);

  module::Select continentDef_se;
  continentDef_se.SetSourceModule (0, baseContinentDef);
  continentDef_se.SetSourceModule (1, continentDef_tu2);
  continentDef_se.SetControlModule (baseContinentDef);
  continentDef_se.SetBounds (SEA_LEVEL - 0.0375, SEA_LEVEL + 1000.0375);
  continentDef_se.SetEdgeFalloff (0.0625);

  module::Cache continentDef;
  continentDef.SetSourceModule (0, continentDef_se);

  module::Turbulence terrainTypeDef_tu;
  terrainTypeDef_tu.SetSourceModule (0, continentDef);
  terrainTypeDef_tu.SetSeed (CUR_SEED + 20);
  terrainTypeDef_tu.SetFrequency (CONTINENT_FREQUENCY * 18.125);
  terrainTypeDef_tu.SetPower (CONTINENT_FREQUENCY / 20.59375 * TERRAIN_OFFSET);
  terrainTypeDef_tu.SetRoughness (3);

  module::Terrace terrainTypeDef_te;
  terrainTypeDef_te.SetSourceModule (0, terrainTypeDef_tu);
  terrainTypeDef_te.AddControlPoint (-1.00);
  terrainTypeDef_te.AddControlPoint (SHELF_LEVEL + SEA_LEVEL / 2.0);
  terrainTypeDef_te.AddControlPoint (1.00);

  module::Cache terrainTypeDef;
  terrainTypeDef.SetSourceModule (0, terrainTypeDef_te);

  module::RidgedMulti mountainBaseDef_rm0;
  mountainBaseDef_rm0.SetSeed (CUR_SEED + 30);
  mountainBaseDef_rm0.SetFrequency (1723.0);
  mountainBaseDef_rm0.SetLacunarity (MOUNTAIN_LACUNARITY);
  mountainBaseDef_rm0.SetOctaveCount (4);
  mountainBaseDef_rm0.SetNoiseQuality (QUALITY_STD);

  module::ScaleBias mountainBaseDef_sb0;
  mountainBaseDef_sb0.SetSourceModule (0, mountainBaseDef_rm0);
  mountainBaseDef_sb0.SetScale (0.5);
  mountainBaseDef_sb0.SetBias (0.375);

  module::RidgedMulti mountainBaseDef_rm1;
  mountainBaseDef_rm1.SetSeed (CUR_SEED + 31);
  mountainBaseDef_rm1.SetFrequency (367.0);
  mountainBaseDef_rm1.SetLacunarity (MOUNTAIN_LACUNARITY);
  mountainBaseDef_rm1.SetOctaveCount (1);
  mountainBaseDef_rm1.SetNoiseQuality (QUALITY_BEST);

  module::ScaleBias mountainBaseDef_sb1;
  mountainBaseDef_sb1.SetSourceModule (0, mountainBaseDef_rm1);
  mountainBaseDef_sb1.SetScale (-2.0);
  mountainBaseDef_sb1.SetBias (-0.5);

  module::Const mountainBaseDef_co;
  mountainBaseDef_co.SetConstValue (-1.0);

  module::Blend mountainBaseDef_bl;
  mountainBaseDef_bl.SetSourceModule (0, mountainBaseDef_co);
  mountainBaseDef_bl.SetSourceModule (1, mountainBaseDef_sb0);
  mountainBaseDef_bl.SetControlModule (mountainBaseDef_sb1);

  module::Turbulence mountainBaseDef_tu0;
  mountainBaseDef_tu0.SetSourceModule (0, mountainBaseDef_bl);
  mountainBaseDef_tu0.SetSeed (CUR_SEED + 32);
  mountainBaseDef_tu0.SetFrequency (1337.0);
  mountainBaseDef_tu0.SetPower (1.0 / 6730.0 * MOUNTAINS_TWIST);
  mountainBaseDef_tu0.SetRoughness (4);

  module::Turbulence mountainBaseDef_tu1;
  mountainBaseDef_tu1.SetSourceModule (0, mountainBaseDef_tu0);
  mountainBaseDef_tu1.SetSeed (CUR_SEED + 33);
  mountainBaseDef_tu1.SetFrequency (21221.0);
  mountainBaseDef_tu1.SetPower (1.0 / 120157.0 * MOUNTAINS_TWIST);
  mountainBaseDef_tu1.SetRoughness (6);

  module::Cache mountainBaseDef;
  mountainBaseDef.SetSourceModule (0, mountainBaseDef_tu1);

  module::RidgedMulti mountainousHigh_rm0;
  mountainousHigh_rm0.SetSeed (CUR_SEED + 40);
  mountainousHigh_rm0.SetFrequency (2371.0);
  mountainousHigh_rm0.SetLacunarity (MOUNTAIN_LACUNARITY);
  mountainousHigh_rm0.SetOctaveCount (3);
  mountainousHigh_rm0.SetNoiseQuality (QUALITY_BEST);

  module::RidgedMulti mountainousHigh_rm1;
  mountainousHigh_rm1.SetSeed (CUR_SEED + 41);
  mountainousHigh_rm1.SetFrequency (2341.0);
  mountainousHigh_rm1.SetLacunarity (MOUNTAIN_LACUNARITY);
  mountainousHigh_rm1.SetOctaveCount (3);
  mountainousHigh_rm1.SetNoiseQuality (QUALITY_BEST);

  module::Max mountainousHigh_ma;
  mountainousHigh_ma.SetSourceModule (0, mountainousHigh_rm0);
  mountainousHigh_ma.SetSourceModule (1, mountainousHigh_rm1);

  module::Turbulence mountainousHigh_tu;
  mountainousHigh_tu.SetSourceModule (0, mountainousHigh_ma);
  mountainousHigh_tu.SetSeed (CUR_SEED + 42);
  mountainousHigh_tu.SetFrequency (31511.0);
  mountainousHigh_tu.SetPower (1.0 / 180371.0 * MOUNTAINS_TWIST);
  mountainousHigh_tu.SetRoughness (4);

  module::Cache mountainousHigh;
  mountainousHigh.SetSourceModule (0, mountainousHigh_tu);

  module::RidgedMulti mountainousLow_rm0;
  mountainousLow_rm0.SetSeed (CUR_SEED + 50);
  mountainousLow_rm0.SetFrequency (1381.0);
  mountainousLow_rm0.SetLacunarity (MOUNTAIN_LACUNARITY);
  mountainousLow_rm0.SetOctaveCount (8);
  mountainousLow_rm0.SetNoiseQuality (QUALITY_BEST);

  module::RidgedMulti mountainousLow_rm1;
  mountainousLow_rm1.SetSeed (CUR_SEED + 51);
  mountainousLow_rm1.SetFrequency (1427.0);
  mountainousLow_rm1.SetLacunarity (MOUNTAIN_LACUNARITY);
  mountainousLow_rm1.SetOctaveCount (8);
  mountainousLow_rm1.SetNoiseQuality (QUALITY_BEST);

  module::Multiply mountainousLow_mu;
  mountainousLow_mu.SetSourceModule (0, mountainousLow_rm0);
  mountainousLow_mu.SetSourceModule (1, mountainousLow_rm1);

  module::Cache mountainousLow;
  mountainousLow.SetSourceModule (0, mountainousLow_mu);
  
  module::ScaleBias mountainousTerrain_sb0;
  mountainousTerrain_sb0.SetSourceModule (0, mountainousLow);
  mountainousTerrain_sb0.SetScale (0.03125);
  mountainousTerrain_sb0.SetBias (-0.96875);

  module::ScaleBias mountainousTerrain_sb1;
  mountainousTerrain_sb1.SetSourceModule (0, mountainousHigh);
  mountainousTerrain_sb1.SetScale (0.25);
  mountainousTerrain_sb1.SetBias (0.25);

  module::Add mountainousTerrain_ad;
  mountainousTerrain_ad.SetSourceModule (0, mountainousTerrain_sb1);
  mountainousTerrain_ad.SetSourceModule (1, mountainBaseDef);

  module::Select mountainousTerrain_se;
  mountainousTerrain_se.SetSourceModule (0, mountainousTerrain_sb0);
  mountainousTerrain_se.SetSourceModule (1, mountainousTerrain_ad);
  mountainousTerrain_se.SetControlModule (mountainBaseDef);
  mountainousTerrain_se.SetBounds (-0.5, 999.5);
  mountainousTerrain_se.SetEdgeFalloff (0.5);

  module::ScaleBias mountainousTerrain_sb2;
  mountainousTerrain_sb2.SetSourceModule (0, mountainousTerrain_se);
  mountainousTerrain_sb2.SetScale (0.8);
  mountainousTerrain_sb2.SetBias (0.0);

  module::Exponent mountainousTerrain_ex;
  mountainousTerrain_ex.SetSourceModule (0, mountainousTerrain_sb2);
  mountainousTerrain_ex.SetExponent (MOUNTAIN_GLACIATION);

  module::Cache mountainousTerrain;
  mountainousTerrain.SetSourceModule (0, mountainousTerrain_ex);

  module::Billow hillyTerrain_bi;
  hillyTerrain_bi.SetSeed (CUR_SEED + 60);
  hillyTerrain_bi.SetFrequency (1663.0);
  hillyTerrain_bi.SetPersistence (0.5);
  hillyTerrain_bi.SetLacunarity (HILLS_LACUNARITY);
  hillyTerrain_bi.SetOctaveCount (6);
  hillyTerrain_bi.SetNoiseQuality (QUALITY_BEST);

  module::ScaleBias hillyTerrain_sb0;
  hillyTerrain_sb0.SetSourceModule (0, hillyTerrain_bi);
  hillyTerrain_sb0.SetScale (0.5);
  hillyTerrain_sb0.SetBias (0.5);

  module::RidgedMulti hillyTerrain_rm;
  hillyTerrain_rm.SetSeed (CUR_SEED + 61);
  hillyTerrain_rm.SetFrequency (367.5);
  hillyTerrain_rm.SetLacunarity (HILLS_LACUNARITY);
  hillyTerrain_rm.SetNoiseQuality (QUALITY_BEST);
  hillyTerrain_rm.SetOctaveCount (1);

  module::ScaleBias hillyTerrain_sb1;
  hillyTerrain_sb1.SetSourceModule (0, hillyTerrain_rm);
  hillyTerrain_sb1.SetScale (-2.0);
  hillyTerrain_sb1.SetBias (-0.5);

  // 5: [Low-flat module]: This low constant value is used by step 6.
  module::Const hillyTerrain_co;
  hillyTerrain_co.SetConstValue (-1.0);

  // 6: [Mountains-and-valleys module]: This blender module merges the
  //    scaled-hills module and the scaled-river-valley module together.  It
  //    causes the low-lying areas of the terrain to become smooth, and causes
  //    the high-lying areas of the terrain to contain hills.  To do this, it
  //    uses the scaled-hills module as the control module, causing the low-
  //    flat module to appear in the lower areas and causing the scaled-river-
  //    valley module to appear in the higher areas.
  module::Blend hillyTerrain_bl;
  hillyTerrain_bl.SetSourceModule (0, hillyTerrain_co);
  hillyTerrain_bl.SetSourceModule (1, hillyTerrain_sb1);
  hillyTerrain_bl.SetControlModule (hillyTerrain_sb0);

  // 7: [Scaled-hills-and-valleys module]: This scale/bias module slightly
  //    reduces the range of the output value from the hills-and-valleys
  //    module, decreasing the heights of the hilltops.
  module::ScaleBias hillyTerrain_sb2;
  hillyTerrain_sb2.SetSourceModule (0, hillyTerrain_bl);
  hillyTerrain_sb2.SetScale (0.75);
  hillyTerrain_sb2.SetBias (-0.25);

  // 8: [Increased-slope-hilly-terrain module]: To increase the hill slopes at
  //    higher elevations, this exponential-curve module applies an
  //    exponential curve to the output value the scaled-hills-and-valleys
  //    module.  This exponential-curve module expects the input value to
  //    range from -1.0 to 1.0.
  module::Exponent hillyTerrain_ex;
  hillyTerrain_ex.SetSourceModule (0, hillyTerrain_sb2);
  hillyTerrain_ex.SetExponent (1.375);

  // 9: [Coarse-turbulence module]: This turbulence module warps the output
  //    value from the increased-slope-hilly-terrain module, adding some
  //    coarse detail to it.
  module::Turbulence hillyTerrain_tu0;
  hillyTerrain_tu0.SetSourceModule (0, hillyTerrain_ex);
  hillyTerrain_tu0.SetSeed (CUR_SEED + 62);
  hillyTerrain_tu0.SetFrequency (1531.0);
  hillyTerrain_tu0.SetPower (1.0 / 16921.0 * HILLS_TWIST);
  hillyTerrain_tu0.SetRoughness (4);

  // 10: [Warped-hilly-terrain module]: This turbulence module warps the
  //     output value from the coarse-turbulence module.  This turbulence has
  //     a higher frequency, but lower power, than the coarse-turbulence
  //     module, adding some fine detail to it.
  module::Turbulence hillyTerrain_tu1;
  hillyTerrain_tu1.SetSourceModule (0, hillyTerrain_tu0);
  hillyTerrain_tu1.SetSeed (CUR_SEED + 63);
  hillyTerrain_tu1.SetFrequency (21617.0);
  hillyTerrain_tu1.SetPower (1.0 / 117529.0 * HILLS_TWIST);
  hillyTerrain_tu1.SetRoughness (6);

  // 11: [Hilly-terrain group]: Caches the output value from the warped-hilly-
  //     terrain module.  This is the output value for the entire hilly-
  //     terrain group.
  module::Cache hillyTerrain;
  hillyTerrain.SetSourceModule (0, hillyTerrain_tu1);


  ////////////////////////////////////////////////////////////////////////////
  // Module group: plains terrain
  ////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  // Module subgroup: plains terrain (7 noise modules)
  //
  // This subgroup generates the plains terrain.
  //
  // Because this subgroup will eventually be flattened considerably, the
  // types and combinations of noise modules that generate the plains are not
  // really that important; they only need to "look" interesting.
  //
  // -1.0 represents the lowest elevations and +1.0 represents the highest
  // elevations.
  //

  // 1: [Plains-basis-0 module]: This billow-noise module, along with the
  //    plains-basis-1 module, produces the plains.
  module::Billow plainsTerrain_bi0;
  plainsTerrain_bi0.SetSeed (CUR_SEED + 70);
  plainsTerrain_bi0.SetFrequency (1097.5);
  plainsTerrain_bi0.SetPersistence (0.5);
  plainsTerrain_bi0.SetLacunarity (PLAINS_LACUNARITY);
  plainsTerrain_bi0.SetOctaveCount (8);
  plainsTerrain_bi0.SetNoiseQuality (QUALITY_BEST);

  // 2: [Positive-plains-basis-0 module]: This scale/bias module makes the
  //    output value from the plains-basis-0 module positive since this output
  //    value will be multiplied together with the positive-plains-basis-1
  //    module.
  module::ScaleBias plainsTerrain_sb0;
  plainsTerrain_sb0.SetSourceModule (0, plainsTerrain_bi0);
  plainsTerrain_sb0.SetScale (0.5);
  plainsTerrain_sb0.SetBias (0.5);

  // 3: [Plains-basis-1 module]: This billow-noise module, along with the
  //    plains-basis-2 module, produces the plains.
  module::Billow plainsTerrain_bi1;
  plainsTerrain_bi1.SetSeed (CUR_SEED + 71);
  plainsTerrain_bi1.SetFrequency (1319.5);
  plainsTerrain_bi1.SetPersistence (0.5);
  plainsTerrain_bi1.SetLacunarity (PLAINS_LACUNARITY);
  plainsTerrain_bi1.SetOctaveCount (8);
  plainsTerrain_bi1.SetNoiseQuality (QUALITY_BEST);

  // 4: [Positive-plains-basis-1 module]: This scale/bias module makes the
  //    output value from the plains-basis-1 module positive since this output
  //    value will be multiplied together with the positive-plains-basis-0
  //    module.
  module::ScaleBias plainsTerrain_sb1;
  plainsTerrain_sb1.SetSourceModule (0, plainsTerrain_bi1);
  plainsTerrain_sb1.SetScale (0.5);
  plainsTerrain_sb1.SetBias (0.5);

  // 5: [Combined-plains-basis module]: This multiplication module combines
  //    the two plains basis modules together.
  module::Multiply plainsTerrain_mu;
  plainsTerrain_mu.SetSourceModule (0, plainsTerrain_sb0);
  plainsTerrain_mu.SetSourceModule (1, plainsTerrain_sb1);

  // 6: [Rescaled-plains-basis module]: This scale/bias module maps the output
  //    value that ranges from 0.0 to 1.0 back to a value that ranges from
  //    -1.0 to +1.0.
  module::ScaleBias plainsTerrain_sb2;
  plainsTerrain_sb2.SetSourceModule (0, plainsTerrain_mu);
  plainsTerrain_sb2.SetScale (2.0);
  plainsTerrain_sb2.SetBias (-1.0);

  // 7: [Plains-terrain group]: Caches the output value from the rescaled-
  //    plains-basis module.  This is the output value for the entire plains-
  //    terrain group.
  module::Cache plainsTerrain;
  plainsTerrain.SetSourceModule (0, plainsTerrain_sb2);


  ////////////////////////////////////////////////////////////////////////////
  // Module group: badlands terrain
  ////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  // Module subgroup: badlands sand (6 noise modules)
  //
  // This subgroup generates the sandy terrain for the badlands.
  //
  // -1.0 represents the lowest elevations and +1.0 represents the highest
  // elevations.
  //

  // 1: [Sand-dunes module]: This ridged-multifractal-noise module generates
  //    sand dunes.  This ridged-multifractal noise is generated with a single
  //    octave, which makes very smooth dunes.
  module::RidgedMulti badlandsSand_rm;
  badlandsSand_rm.SetSeed (CUR_SEED + 80);
  badlandsSand_rm.SetFrequency (6163.5);
  badlandsSand_rm.SetLacunarity (BADLANDS_LACUNARITY);
  badlandsSand_rm.SetNoiseQuality (QUALITY_BEST);
  badlandsSand_rm.SetOctaveCount (1);

  // 2: [Scaled-sand-dunes module]: This scale/bias module shrinks the dune
  //    heights by a small amount.  This is necessary so that the subsequent
  //    noise modules in this subgroup can add some detail to the dunes.
  module::ScaleBias badlandsSand_sb0;
  badlandsSand_sb0.SetSourceModule (0, badlandsSand_rm);
  badlandsSand_sb0.SetScale (0.875);
  badlandsSand_sb0.SetBias (0.0);

  // 3: [Dune-detail module]: This noise module uses Voronoi polygons to
  //    generate the detail to add to the dunes.  By enabling the distance
  //    algorithm, small polygonal pits are generated; the edges of the pits
  //    are joined to the edges of nearby pits.
  module::Voronoi badlandsSand_vo;
  badlandsSand_vo.SetSeed (CUR_SEED + 81);
  badlandsSand_vo.SetFrequency (16183.25);
  badlandsSand_vo.SetDisplacement (0.0);
  badlandsSand_vo.EnableDistance ();

  // 4: [Scaled-dune-detail module]: This scale/bias module shrinks the dune
  //    details by a large amount.  This is necessary so that the subsequent
  //    noise modules in this subgroup can add this detail to the sand-dunes
  //    module.
  module::ScaleBias badlandsSand_sb1;
  badlandsSand_sb1.SetSourceModule (0, badlandsSand_vo);
  badlandsSand_sb1.SetScale (0.25);
  badlandsSand_sb1.SetBias (0.25);

  // 5: [Dunes-with-detail module]: This addition module combines the scaled-
  //    sand-dunes module with the scaled-dune-detail module.
  module::Add badlandsSand_ad;
  badlandsSand_ad.SetSourceModule (0, badlandsSand_sb0);
  badlandsSand_ad.SetSourceModule (1, badlandsSand_sb1);

  // 6: [Badlands-sand subgroup]: Caches the output value from the dunes-with-
  //    detail module.
  module::Cache badlandsSand;
  badlandsSand.SetSourceModule (0, badlandsSand_ad);


  ////////////////////////////////////////////////////////////////////////////
  // Module subgroup: badlands cliffs (7 noise modules)
  //
  // This subgroup generates the cliffs for the badlands.
  //
  // -1.0 represents the lowest elevations and +1.0 represents the highest
  // elevations.
  //

  // 1: [Cliff-basis module]: This Perlin-noise module generates some coherent
  //    noise that will be used to generate the cliffs.
  module::Perlin badlandsCliffs_pe;
  badlandsCliffs_pe.SetSeed (CUR_SEED + 90);
  badlandsCliffs_pe.SetFrequency (CONTINENT_FREQUENCY * 839.0);
  badlandsCliffs_pe.SetPersistence (0.5);
  badlandsCliffs_pe.SetLacunarity (BADLANDS_LACUNARITY);
  badlandsCliffs_pe.SetOctaveCount (6);
  badlandsCliffs_pe.SetNoiseQuality (QUALITY_STD);

  // 2: [Cliff-shaping module]: Next, this curve module applies a curve to the
  //    output value from the cliff-basis module.  This curve is initially
  //    very shallow, but then its slope increases sharply.  At the highest
  //    elevations, the curve becomes very flat again.  This produces the
  //    stereotypical Utah-style desert cliffs.
  module::Curve badlandsCliffs_cu;
  badlandsCliffs_cu.SetSourceModule (0, badlandsCliffs_pe);
  badlandsCliffs_cu.AddControlPoint (-2.0000, -2.0000);
  badlandsCliffs_cu.AddControlPoint (-1.0000, -1.2500);
  badlandsCliffs_cu.AddControlPoint (-0.0000, -0.7500);
  badlandsCliffs_cu.AddControlPoint ( 0.5000, -0.2500);
  badlandsCliffs_cu.AddControlPoint ( 0.6250,  0.8750);
  badlandsCliffs_cu.AddControlPoint ( 0.7500,  1.0000);
  badlandsCliffs_cu.AddControlPoint ( 2.0000,  1.2500);

  // 3: [Clamped-cliffs module]: This clamping module makes the tops of the
  //    cliffs very flat by clamping the output value from the cliff-shaping
  //    module so that the tops of the cliffs are very flat.
  module::Clamp badlandsCliffs_cl;
  badlandsCliffs_cl.SetSourceModule (0, badlandsCliffs_cu);
  badlandsCliffs_cl.SetBounds (-999.125, 0.875);

  // 4: [Terraced-cliffs module]: Next, this terracing module applies some
  //    terraces to the clamped-cliffs module in the lower elevations before
  //    the sharp cliff transition.
  module::Terrace badlandsCliffs_te;
  badlandsCliffs_te.SetSourceModule (0, badlandsCliffs_cl);
  badlandsCliffs_te.AddControlPoint (-1.0000);
  badlandsCliffs_te.AddControlPoint (-0.8750);
  badlandsCliffs_te.AddControlPoint (-0.7500);
  badlandsCliffs_te.AddControlPoint (-0.5000);
  badlandsCliffs_te.AddControlPoint ( 0.0000);
  badlandsCliffs_te.AddControlPoint ( 1.0000);

  // 5: [Coarse-turbulence module]: This turbulence module warps the output
  //    value from the terraced-cliffs module, adding some coarse detail to
  //    it.
  module::Turbulence badlandsCliffs_tu0;
  badlandsCliffs_tu0.SetSeed (CUR_SEED + 91);
  badlandsCliffs_tu0.SetSourceModule (0, badlandsCliffs_te);
  badlandsCliffs_tu0.SetFrequency (16111.0);
  badlandsCliffs_tu0.SetPower (1.0 / 141539.0 * BADLANDS_TWIST);
  badlandsCliffs_tu0.SetRoughness (3);

  // 6: [Warped-cliffs module]: This turbulence module warps the output value
  //    from the coarse-turbulence module.  This turbulence has a higher
  //    frequency, but lower power, than the coarse-turbulence module, adding
  //    some fine detail to it.
  module::Turbulence badlandsCliffs_tu1;
  badlandsCliffs_tu1.SetSeed (CUR_SEED + 92);
  badlandsCliffs_tu1.SetSourceModule (0, badlandsCliffs_tu0);
  badlandsCliffs_tu1.SetFrequency (36107.0);
  badlandsCliffs_tu1.SetPower (1.0 / 211543.0 * BADLANDS_TWIST);
  badlandsCliffs_tu1.SetRoughness (3);

  // 7: [Badlands-cliffs subgroup]: Caches the output value from the warped-
  //    cliffs module.
  module::Cache badlandsCliffs;
  badlandsCliffs.SetSourceModule (0, badlandsCliffs_tu1);


  ////////////////////////////////////////////////////////////////////////////
  // Module subgroup: badlands terrain (3 noise modules)
  //
  // Generates the final badlands terrain.
  //
  // Using a scale/bias module, the badlands sand is flattened considerably,
  // then the sand elevations are lowered to around -1.0.  The maximum value
  // from the flattened sand module and the cliff module contributes to the
  // final elevation.  This causes sand to appear at the low elevations since
  // the sand is slightly higher than the cliff base.
  //
  // -1.0 represents the lowest elevations and +1.0 represents the highest
  // elevations.
  //

  // 1: [Scaled-sand-dunes module]: This scale/bias module considerably
  //    flattens the output value from the badlands-sands subgroup and lowers
  //    this value to near -1.0.
  module::ScaleBias badlandsTerrain_sb;
  badlandsTerrain_sb.SetSourceModule (0, badlandsSand);
  badlandsTerrain_sb.SetScale (0.25);
  badlandsTerrain_sb.SetBias (-0.75);

  // 2: [Dunes-and-cliffs module]: This maximum-value module causes the dunes
  //    to appear in the low areas and the cliffs to appear in the high areas.
  //    It does this by selecting the maximum of the output values from the
  //    scaled-sand-dunes module and the badlands-cliffs subgroup.
  module::Max badlandsTerrain_ma;
  badlandsTerrain_ma.SetSourceModule (0, badlandsCliffs);
  badlandsTerrain_ma.SetSourceModule (1, badlandsTerrain_sb);

  // 3: [Badlands-terrain group]: Caches the output value from the dunes-and-
  //    cliffs module.  This is the output value for the entire badlands-
  //    terrain group.
  module::Cache badlandsTerrain;
  badlandsTerrain.SetSourceModule (0, badlandsTerrain_ma);


  ////////////////////////////////////////////////////////////////////////////
  // Module group: river positions
  ////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  // Module subgroup: river positions (7 noise modules)
  //
  // This subgroup generates the river positions.
  //
  // -1.0 represents the lowest elevations and +1.0 represents the highest
  // elevations.
  //

  // 1: [Large-river-basis module]: This ridged-multifractal-noise module
  //    creates the large, deep rivers.
  module::RidgedMulti riverPositions_rm0;
  riverPositions_rm0.SetSeed (CUR_SEED + 100);
  riverPositions_rm0.SetFrequency (18.75);
  riverPositions_rm0.SetLacunarity (CONTINENT_LACUNARITY);
  riverPositions_rm0.SetOctaveCount (1);
  riverPositions_rm0.SetNoiseQuality (QUALITY_BEST);

  // 2: [Large-river-curve module]: This curve module applies a curve to the
  //    output value from the large-river-basis module so that the ridges
  //    become inverted.  This creates the rivers.  This curve also compresses
  //    the edge of the rivers, producing a sharp transition from the land to
  //    the river bottom.
  module::Curve riverPositions_cu0;
  riverPositions_cu0.SetSourceModule (0, riverPositions_rm0);
  riverPositions_cu0.AddControlPoint (-2.000,  2.000);
  riverPositions_cu0.AddControlPoint (-1.000,  1.000);
  riverPositions_cu0.AddControlPoint (-0.125,  0.875);
  riverPositions_cu0.AddControlPoint ( 0.000, -1.000);
  riverPositions_cu0.AddControlPoint ( 1.000, -1.500);
  riverPositions_cu0.AddControlPoint ( 2.000, -2.000);

  /// 3: [Small-river-basis module]: This ridged-multifractal-noise module
  //     creates the small, shallow rivers.
  module::RidgedMulti riverPositions_rm1;
  riverPositions_rm1.SetSeed (CUR_SEED + 101);
  riverPositions_rm1.SetFrequency (43.25);
  riverPositions_rm1.SetLacunarity (CONTINENT_LACUNARITY);
  riverPositions_rm1.SetOctaveCount (1);
  riverPositions_rm1.SetNoiseQuality (QUALITY_BEST);

  // 4: [Small-river-curve module]: This curve module applies a curve to the
  //    output value from the small-river-basis module so that the ridges
  //    become inverted.  This creates the rivers.  This curve also compresses
  //    the edge of the rivers, producing a sharp transition from the land to
  //    the river bottom.
  module::Curve riverPositions_cu1;
  riverPositions_cu1.SetSourceModule (0, riverPositions_rm1);
  riverPositions_cu1.AddControlPoint (-2.000,  2.0000);
  riverPositions_cu1.AddControlPoint (-1.000,  1.5000);
  riverPositions_cu1.AddControlPoint (-0.125,  1.4375);
  riverPositions_cu1.AddControlPoint ( 0.000,  0.5000);
  riverPositions_cu1.AddControlPoint ( 1.000,  0.2500);
  riverPositions_cu1.AddControlPoint ( 2.000,  0.0000);

  // 5: [Combined-rivers module]: This minimum-value module causes the small
  //    rivers to cut into the large rivers.  It does this by selecting the
  //    minimum output values from the large-river-curve module and the small-
  //    river-curve module.
  module::Min riverPositions_mi;
  riverPositions_mi.SetSourceModule (0, riverPositions_cu0);
  riverPositions_mi.SetSourceModule (1, riverPositions_cu1);

  // 6: [Warped-rivers module]: This turbulence module warps the output value
  //    from the combined-rivers module, which twists the rivers.  The high
  //    roughness produces less-smooth rivers.
  module::Turbulence riverPositions_tu;
  riverPositions_tu.SetSourceModule (0, riverPositions_mi);
  riverPositions_tu.SetSeed (CUR_SEED + 102);
  riverPositions_tu.SetFrequency (9.25);
  riverPositions_tu.SetPower (1.0 / 57.75);
  riverPositions_tu.SetRoughness (6);

  // 7: [River-positions group]: Caches the output value from the warped-
  //    rivers module.  This is the output value for the entire river-
  //    positions group.
  module::Cache riverPositions;
  riverPositions.SetSourceModule (0, riverPositions_tu);


  ////////////////////////////////////////////////////////////////////////////
  // Module group: scaled mountainous terrain
  ////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  // Module subgroup: scaled mountainous terrain (6 noise modules)
  //
  // This subgroup scales the output value from the mountainous-terrain group
  // so that it can be added to the elevation defined by the continent-
  // definition group.
  //
  // This subgroup scales the output value such that it is almost always
  // positive.  This is done so that a negative elevation does not get applied
  // to the continent-definition group, preventing parts of that group from
  // having negative terrain features "stamped" into it.
  //
  // The output value from this module subgroup is measured in planetary
  // elevation units (-1.0 for the lowest underwater trenches and +1.0 for the
  // highest mountain peaks.)
  //

  // 1: [Base-scaled-mountainous-terrain module]: This scale/bias module
  //    scales the output value from the mountainous-terrain group so that the
  //    output value is measured in planetary elevation units.
  module::ScaleBias scaledMountainousTerrain_sb0;
  scaledMountainousTerrain_sb0.SetSourceModule (0, mountainousTerrain);
  scaledMountainousTerrain_sb0.SetScale (0.125);
  scaledMountainousTerrain_sb0.SetBias (0.125);

  // 2: [Base-peak-modulation module]: At this stage, most mountain peaks have
  //    roughly the same elevation.  This Perlin-noise module generates some
  //    random values that will be used by subsequent noise modules to
  //    randomly change the elevations of the mountain peaks.
  module::Perlin scaledMountainousTerrain_pe;
  scaledMountainousTerrain_pe.SetSeed (CUR_SEED + 110);
  scaledMountainousTerrain_pe.SetFrequency (14.5);
  scaledMountainousTerrain_pe.SetPersistence (0.5);
  scaledMountainousTerrain_pe.SetLacunarity (MOUNTAIN_LACUNARITY);
  scaledMountainousTerrain_pe.SetOctaveCount (6);
  scaledMountainousTerrain_pe.SetNoiseQuality (QUALITY_STD);

  // 3: [Peak-modulation module]: This exponential-curve module applies an
  //    exponential curve to the output value from the base-peak-modulation
  //    module.  This produces a small number of high values and a much larger
  //    number of low values.  This means there will be a few peaks with much
  //    higher elevations than the majority of the peaks, making the terrain
  //    features more varied.
  module::Exponent scaledMountainousTerrain_ex;
  scaledMountainousTerrain_ex.SetSourceModule (0, scaledMountainousTerrain_pe);
  scaledMountainousTerrain_ex.SetExponent (1.25);

  // 4: [Scaled-peak-modulation module]: This scale/bias module modifies the
  //    range of the output value from the peak-modulation module so that it
  //    can be used as the modulator for the peak-height-multiplier module.
  //    It is important that this output value is not much lower than 1.0.
  module::ScaleBias scaledMountainousTerrain_sb1;
  scaledMountainousTerrain_sb1.SetSourceModule (0,
    scaledMountainousTerrain_ex);
  scaledMountainousTerrain_sb1.SetScale (0.25);
  scaledMountainousTerrain_sb1.SetBias (1.0);

  module::Multiply scaledMountainousTerrain_mu;
  scaledMountainousTerrain_mu.SetSourceModule (0,
    scaledMountainousTerrain_sb0);
  scaledMountainousTerrain_mu.SetSourceModule (1,
    scaledMountainousTerrain_sb1);

  module::Cache scaledMountainousTerrain;
  scaledMountainousTerrain.SetSourceModule (0, scaledMountainousTerrain_mu);

  module::ScaleBias scaledHillyTerrain_sb0;
  scaledHillyTerrain_sb0.SetSourceModule (0, hillyTerrain);
  scaledHillyTerrain_sb0.SetScale (0.0625);
  scaledHillyTerrain_sb0.SetBias (0.0625);

  module::Perlin scaledHillyTerrain_pe;
  scaledHillyTerrain_pe.SetSeed (CUR_SEED + 120);
  scaledHillyTerrain_pe.SetFrequency (13.5);
  scaledHillyTerrain_pe.SetPersistence (0.5);
  scaledHillyTerrain_pe.SetLacunarity (HILLS_LACUNARITY);
  scaledHillyTerrain_pe.SetOctaveCount (6);
  scaledHillyTerrain_pe.SetNoiseQuality (QUALITY_STD);

  module::Exponent scaledHillyTerrain_ex;
  scaledHillyTerrain_ex.SetSourceModule (0, scaledHillyTerrain_pe);
  scaledHillyTerrain_ex.SetExponent (1.25);

  module::ScaleBias scaledHillyTerrain_sb1;
  scaledHillyTerrain_sb1.SetSourceModule (0, scaledHillyTerrain_ex);
  scaledHillyTerrain_sb1.SetScale (0.5);
  scaledHillyTerrain_sb1.SetBias (1.5);

  module::Multiply scaledHillyTerrain_mu;
  scaledHillyTerrain_mu.SetSourceModule (0, scaledHillyTerrain_sb0);
  scaledHillyTerrain_mu.SetSourceModule (1, scaledHillyTerrain_sb1);

  module::Cache scaledHillyTerrain;
  scaledHillyTerrain.SetSourceModule (0, scaledHillyTerrain_mu);

  module::ScaleBias scaledPlainsTerrain_sb;
  scaledPlainsTerrain_sb.SetSourceModule (0, plainsTerrain);
  scaledPlainsTerrain_sb.SetScale (0.00390625);
  scaledPlainsTerrain_sb.SetBias (0.0078125);

  module::Cache scaledPlainsTerrain;
  scaledPlainsTerrain.SetSourceModule (0, scaledPlainsTerrain_sb);

  module::ScaleBias scaledBadlandsTerrain_sb;
  scaledBadlandsTerrain_sb.SetSourceModule (0, badlandsTerrain);
  scaledBadlandsTerrain_sb.SetScale (0.0625);
  scaledBadlandsTerrain_sb.SetBias (0.0625);

  module::Cache scaledBadlandsTerrain;
  scaledBadlandsTerrain.SetSourceModule (0, scaledBadlandsTerrain_sb);

  module::Terrace continentalShelf_te;
  continentalShelf_te.SetSourceModule (0, continentDef);
  continentalShelf_te.AddControlPoint (-1.0);
  continentalShelf_te.AddControlPoint (-0.75);
  continentalShelf_te.AddControlPoint (SHELF_LEVEL);
  continentalShelf_te.AddControlPoint (1.0);

  module::RidgedMulti continentalShelf_rm;
  continentalShelf_rm.SetSeed (CUR_SEED + 130);
  continentalShelf_rm.SetFrequency (CONTINENT_FREQUENCY * 4.375);
  continentalShelf_rm.SetLacunarity (CONTINENT_LACUNARITY);
  continentalShelf_rm.SetOctaveCount (16);
  continentalShelf_rm.SetNoiseQuality (QUALITY_BEST);

  module::ScaleBias continentalShelf_sb;
  continentalShelf_sb.SetSourceModule (0, continentalShelf_rm);
  continentalShelf_sb.SetScale (-0.125);
  continentalShelf_sb.SetBias (-0.125);

  module::Clamp continentalShelf_cl;
  continentalShelf_cl.SetSourceModule (0, continentalShelf_te);
  continentalShelf_cl.SetBounds (-0.75, SEA_LEVEL);

  module::Add continentalShelf_ad;
  continentalShelf_ad.SetSourceModule (0, continentalShelf_sb);
  continentalShelf_ad.SetSourceModule (1, continentalShelf_cl);

  module::Cache continentalShelf;
  continentalShelf.SetSourceModule (0, continentalShelf_ad);

  module::ScaleBias baseContinentElev_sb;
  baseContinentElev_sb.SetSourceModule (0, continentDef);
  baseContinentElev_sb.SetScale (CONTINENT_HEIGHT_SCALE);
  baseContinentElev_sb.SetBias (0.0);

  module::Select baseContinentElev_se;
  baseContinentElev_se.SetSourceModule (0, baseContinentElev_sb);
  baseContinentElev_se.SetSourceModule (1, continentalShelf);
  baseContinentElev_se.SetControlModule (continentDef);
  baseContinentElev_se.SetBounds (SHELF_LEVEL - 1000.0, SHELF_LEVEL);
  baseContinentElev_se.SetEdgeFalloff (0.03125);

  module::Cache baseContinentElev;
  baseContinentElev.SetSourceModule (0, baseContinentElev_se);

  module::Add continentsWithPlains_ad;
  continentsWithPlains_ad.SetSourceModule (0, baseContinentElev);
  continentsWithPlains_ad.SetSourceModule (1, scaledPlainsTerrain);

  module::Cache continentsWithPlains;
  continentsWithPlains.SetSourceModule (0, continentsWithPlains_ad);

  module::Add continentsWithHills_ad;
  continentsWithHills_ad.SetSourceModule (0, baseContinentElev);
  continentsWithHills_ad.SetSourceModule (1, scaledHillyTerrain);

  module::Select continentsWithHills_se;
  continentsWithHills_se.SetSourceModule (0, continentsWithPlains);
  continentsWithHills_se.SetSourceModule (1, continentsWithHills_ad);
  continentsWithHills_se.SetControlModule (terrainTypeDef);
  continentsWithHills_se.SetBounds (1.0 - HILLS_AMOUNT, 1001.0 - HILLS_AMOUNT);
  continentsWithHills_se.SetEdgeFalloff (0.25);

  module::Cache continentsWithHills;
  continentsWithHills.SetSourceModule (0, continentsWithHills_se);

  module::Add continentsWithMountains_ad0;
  continentsWithMountains_ad0.SetSourceModule (0, baseContinentElev);
  continentsWithMountains_ad0.SetSourceModule (1, scaledMountainousTerrain);

  module::Curve continentsWithMountains_cu;
  continentsWithMountains_cu.SetSourceModule (0, continentDef);
  continentsWithMountains_cu.AddControlPoint (                  -1.0, -0.0625);
  continentsWithMountains_cu.AddControlPoint (                   0.0,  0.0000);
  continentsWithMountains_cu.AddControlPoint (1.0 - MOUNTAINS_AMOUNT,  0.0625);
  continentsWithMountains_cu.AddControlPoint (                   1.0,  0.2500);

  module::Add continentsWithMountains_ad1;
  continentsWithMountains_ad1.SetSourceModule (0, continentsWithMountains_ad0);
  continentsWithMountains_ad1.SetSourceModule (1, continentsWithMountains_cu);

  module::Select continentsWithMountains_se;
  continentsWithMountains_se.SetSourceModule (0, continentsWithHills);
  continentsWithMountains_se.SetSourceModule (1, continentsWithMountains_ad1);
  continentsWithMountains_se.SetControlModule (terrainTypeDef);
  continentsWithMountains_se.SetBounds (1.0 - MOUNTAINS_AMOUNT,
    1001.0 - MOUNTAINS_AMOUNT);
  continentsWithMountains_se.SetEdgeFalloff (0.25);

  module::Cache continentsWithMountains;
  continentsWithMountains.SetSourceModule (0, continentsWithMountains_se);

  module::Perlin continentsWithBadlands_pe;
  continentsWithBadlands_pe.SetSeed (CUR_SEED + 140);
  continentsWithBadlands_pe.SetFrequency (16.5);
  continentsWithBadlands_pe.SetPersistence (0.5);
  continentsWithBadlands_pe.SetLacunarity (CONTINENT_LACUNARITY);
  continentsWithBadlands_pe.SetOctaveCount (2);
  continentsWithBadlands_pe.SetNoiseQuality (QUALITY_STD);

  module::Add continentsWithBadlands_ad;
  continentsWithBadlands_ad.SetSourceModule (0, baseContinentElev);
  continentsWithBadlands_ad.SetSourceModule (1, scaledBadlandsTerrain);

  module::Select continentsWithBadlands_se;
  continentsWithBadlands_se.SetSourceModule (0, continentsWithMountains);
  continentsWithBadlands_se.SetSourceModule (1, continentsWithBadlands_ad);
  continentsWithBadlands_se.SetControlModule (continentsWithBadlands_pe);
  continentsWithBadlands_se.SetBounds (1.0 - BADLANDS_AMOUNT,
    1001.0 - BADLANDS_AMOUNT);
  continentsWithBadlands_se.SetEdgeFalloff (0.25);

  module::Max continentsWithBadlands_ma;
  continentsWithBadlands_ma.SetSourceModule (0, continentsWithMountains);
  continentsWithBadlands_ma.SetSourceModule (1, continentsWithBadlands_se);

  module::Cache continentsWithBadlands;
  continentsWithBadlands.SetSourceModule (0, continentsWithBadlands_ma);

  module::ScaleBias continentsWithRivers_sb;
  continentsWithRivers_sb.SetSourceModule (0, riverPositions);
  continentsWithRivers_sb.SetScale (RIVER_DEPTH / 2.0);
  continentsWithRivers_sb.SetBias (-RIVER_DEPTH / 2.0);

  module::Add continentsWithRivers_ad;
  continentsWithRivers_ad.SetSourceModule (0, continentsWithBadlands);
  continentsWithRivers_ad.SetSourceModule (1, continentsWithRivers_sb);

  module::Select continentsWithRivers_se;
  continentsWithRivers_se.SetSourceModule (0, continentsWithBadlands);
  continentsWithRivers_se.SetSourceModule (1, continentsWithRivers_ad);
  continentsWithRivers_se.SetControlModule (continentsWithBadlands);
  continentsWithRivers_se.SetBounds (SEA_LEVEL,
    CONTINENT_HEIGHT_SCALE + SEA_LEVEL);
  continentsWithRivers_se.SetEdgeFalloff (CONTINENT_HEIGHT_SCALE - SEA_LEVEL);

  module::Cache continentsWithRivers;
  continentsWithRivers.SetSourceModule (0, continentsWithRivers_se);

  module::Cache unscaledFinalPlanet;
  unscaledFinalPlanet.SetSourceModule (0, continentsWithRivers);

  module::ScaleBias finalPlanet_sb;
  finalPlanet_sb.SetSourceModule (0, unscaledFinalPlanet);
  finalPlanet_sb.SetScale ((MAX_ELEV - MIN_ELEV) / 2.0);
  finalPlanet_sb.SetBias (MIN_ELEV + ((MAX_ELEV - MIN_ELEV) / 2.0));

  module::Cache finalPlanet;
  finalPlanet.SetSourceModule (0, finalPlanet_sb);

  assert (SOUTH_COORD < NORTH_COORD);
  assert (WEST_COORD < EAST_COORD);
  assert (GRID_WIDTH > 0);
  assert (GRID_HEIGHT > 0);
  assert (PLANET_CIRCUMFERENCE >= 1.0);
  assert (MIN_ELEV < MAX_ELEV);
  assert (CONTINENT_FREQUENCY >= 1.0);
  assert (CONTINENT_LACUNARITY >= 1.0);
  assert (CONTINENT_LACUNARITY <= 4.0);
  assert (MOUNTAIN_LACUNARITY >= 1.0);
  assert (MOUNTAIN_LACUNARITY <= 4.0);
  assert (HILLS_LACUNARITY >= 1.0);
  assert (HILLS_LACUNARITY <= 4.0);
  assert (PLAINS_LACUNARITY >= 1.0);
  assert (PLAINS_LACUNARITY <= 4.0);
  assert (BADLANDS_LACUNARITY >= 1.0);
  assert (BADLANDS_LACUNARITY <= 4.0);
  assert (MOUNTAINS_TWIST >= 0.0);
  assert (HILLS_TWIST >= 0.0);
  assert (BADLANDS_TWIST >= 0.0);
  assert (SEA_LEVEL >= -1.0);
  assert (SEA_LEVEL <=  1.0);
  assert (SHELF_LEVEL >= -1.0);
  assert (SHELF_LEVEL <=  1.0);
  assert (SEA_LEVEL > SHELF_LEVEL);
  assert (MOUNTAINS_AMOUNT >=  0.0);
  assert (MOUNTAINS_AMOUNT <=  1.0);
  assert (HILLS_AMOUNT >= 0.0);
  assert (HILLS_AMOUNT <= 1.0);
  assert (BADLANDS_AMOUNT >= 0.0);
  assert (BADLANDS_AMOUNT <= 1.0);
  assert (MOUNTAINS_AMOUNT < HILLS_AMOUNT);
  assert (MOUNTAIN_GLACIATION >= 1.0);
  assert (RIVER_DEPTH >= 0.0);

  planet.SetBounds (SOUTH_COORD, NORTH_COORD, WEST_COORD, EAST_COORD);
  planet.SetDestSize (GRID_WIDTH, GRID_HEIGHT);

  planet.SetSourceModule (finalPlanet);
  planet.SetDestNoiseMap (elevGrid);
  planet.Build ();

  double degExtent = EAST_COORD - WEST_COORD;
  double gridExtent = (double)GRID_WIDTH;
  double metersPerDegree = (PLANET_CIRCUMFERENCE / 360.0);
  resInMeters = (degExtent / gridExtent) * metersPerDegree;
}

void getTopography() {
  uint32_t toposize = GRID_WIDTH*GRID_HEIGHT*2;
  fwrite(&toposize, sizeof(toposize), 1, stdout);

  uint8* pLineBuffer = new uint8[GRID_WIDTH * 2];
  std::ofstream os;
  char fullpath[1024];
  sprintf("%s/%s-topo.raw", fullpath, cachedir, outfile);
  os.open (fullpath, std::ios::out | std::ios::binary);
  for (int y = 0; y < GRID_HEIGHT; y++) {
    float* pSource = elevGrid.GetSlabPtr (y);
    uint8* pDest = pLineBuffer;
    for (int x = 0; x < GRID_WIDTH; x++) {
      int16 elev = (int16)(floor (*pSource));
      *pDest++ = (uint8)(((uint16)elev & 0xff00) >> 8);
      *pDest++ = (uint8)(((uint16)elev & 0x00ff)     );
      ++pSource;
    }
    //fwrite(pLineBuffer, GRID_WIDTH*2, 1, stdout);
    os.write ((char*)pLineBuffer, GRID_WIDTH * 2);
  }
  os.close ();
  delete[] pLineBuffer;
}

void getTexture() {
  seaLevelInMeters = (((SEA_LEVEL + 1.0) / 2.0)
    * (MAX_ELEV - MIN_ELEV)) + MIN_ELEV;

  const double DEEP_SEA_LEVEL = -256.0;
  for (int y = 0; y < GRID_HEIGHT; y++) {
    float* pCur = elevGrid.GetSlabPtr (y);
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (*pCur < (SEA_LEVEL + DEEP_SEA_LEVEL)) {
        *pCur = (SEA_LEVEL + DEEP_SEA_LEVEL);
      }
      ++pCur;
    }
  }

  utils::RendererImage surfaceRenderer;
  surfaceRenderer.SetSourceNoiseMap (elevGrid);
  surfaceRenderer.SetDestImage (destImage);
  surfaceRenderer.ClearGradient ();
  surfaceRenderer.AddGradientPoint (-16384.0 + seaLevelInMeters, utils::Color (  3,  29,  63, 255));
  surfaceRenderer.AddGradientPoint (DEEP_SEA_LEVEL + seaLevelInMeters, utils::Color (  3,  29,  63, 255));
  surfaceRenderer.AddGradientPoint (-1.0 + seaLevelInMeters, utils::Color (  7, 106, 127, 255));
  surfaceRenderer.AddGradientPoint (0.0 + seaLevelInMeters, utils::Color ( 62,  86,  30, 255));
  surfaceRenderer.AddGradientPoint (1024.0 + seaLevelInMeters, utils::Color ( 84,  96,  50, 255));
  surfaceRenderer.AddGradientPoint (2048.0 + seaLevelInMeters, utils::Color (130, 127,  97, 255));
  surfaceRenderer.AddGradientPoint (3072.0 + seaLevelInMeters, utils::Color (184, 163, 141, 255));
  surfaceRenderer.AddGradientPoint (4096.0 + seaLevelInMeters, utils::Color (255, 255, 255, 255));
  surfaceRenderer.AddGradientPoint (6144.0 + seaLevelInMeters, utils::Color (128, 255, 255, 255));
  surfaceRenderer.AddGradientPoint (16384.0 + seaLevelInMeters, utils::Color (  0,   0, 255, 255));
  surfaceRenderer.EnableLight (false);
  surfaceRenderer.Render ();
  bitmapWriter.SetSourceImage (destImage);
  char fullpath[1024];
  sprintf("%s/%s-texture.bmp", fullpath, cachedir, outfile);
  bitmapWriter.SetDestFilename (fullpath);
  bitmapWriter.WriteDestFile ();
}

void getSpecular() {
  utils::RendererImage specularityRenderer;
  specularityRenderer.SetSourceNoiseMap (elevGrid);
  specularityRenderer.SetDestImage (destImage);
  specularityRenderer.ClearGradient ();
  specularityRenderer.AddGradientPoint (MIN_ELEV              , utils::Color (255, 255, 255, 255));
  specularityRenderer.AddGradientPoint (seaLevelInMeters      , utils::Color (255, 255, 255, 255));
  specularityRenderer.AddGradientPoint (seaLevelInMeters + 1.0, utils::Color (  0,   0,   0, 255));
  specularityRenderer.AddGradientPoint (MAX_ELEV              , utils::Color (128, 128, 128, 255));
  specularityRenderer.EnableLight (false);
  specularityRenderer.Render ();
  bitmapWriter.SetSourceImage (destImage);
  char fullpath[1024];
  sprintf("%s/%s-specular.bmp", fullpath, cachedir, outfile);
  bitmapWriter.SetDestFilename (fullpath);
  bitmapWriter.WriteDestFile ();
}

void getNormals() {
  utils::RendererNormalMap normalMapRenderer;
  normalMapRenderer.SetSourceNoiseMap (elevGrid);
  normalMapRenderer.SetDestImage (destImage);
  normalMapRenderer.SetBumpHeight (1.0 / resInMeters);
  normalMapRenderer.Render ();
  bitmapWriter.SetSourceImage (destImage);
    char fullpath[1024];
  sprintf("%s/%s-normal.bmp", fullpath, cachedir, outfile);
  bitmapWriter.SetDestFilename (fullpath);
  bitmapWriter.WriteDestFile ();
}
