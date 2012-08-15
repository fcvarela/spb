#include <noise/noise.h>
#include "noiseutils.h"

using namespace noise;

int main(int argc, char** argv) {
    module::Perlin myModule;

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderSphere heightMapBuilder;

    heightMapBuilder.SetSourceModule(myModule);
    myModule.SetOctaveCount (15);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(1024, 512);
    heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
    heightMapBuilder.Build();
    utils::RendererImage renderer;
    utils::Image image;
    renderer.SetSourceNoiseMap(heightMap);
    renderer.SetDestImage(image);
    renderer.ClearGradient();
    renderer.AddGradientPoint(-1.0000, utils::Color(0,   0, 128, 255)); // deeps
    renderer.AddGradientPoint(-0.2500, utils::Color(0,   0, 255, 255)); // shallow
    renderer.AddGradientPoint(0.0000, utils::Color(0, 128, 255, 255)); // shore
    renderer.AddGradientPoint(0.0625, utils::Color(240, 240,  64, 255)); // sand
    renderer.AddGradientPoint(0.1250, utils::Color(32, 160,   0, 255)); // grass
    renderer.AddGradientPoint(0.3750, utils::Color(224, 224,   0, 255)); // dirt
    renderer.AddGradientPoint(0.7500, utils::Color(128, 128, 128, 255)); // rock
    renderer.AddGradientPoint(1.0000, utils::Color(255, 255, 255, 255)); // snow
    renderer.EnableLight();
    renderer.SetLightContrast(3.0);
    renderer.SetLightBrightness(2.0);
    renderer.Render();

    utils::WriterBMP writer;
    writer.SetSourceImage(image);
    writer.SetDestFilename("tutorial.bmp");
    writer.WriteDestFile();

    return 0;
}

