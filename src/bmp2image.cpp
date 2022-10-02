#include "./slim/platforms/win32_bitmap.h"
#include "./slim/serialization/image.h"

int main(int argc, char *argv[]) {
    Image image;
    bool flip = false;

    char* bitmap_file_path = argv[1];
    char* image_file_path = argv[2];

    for (u8 i = 3; i < (u8)argc; i++)
        if (     argv[i][0] == '-' && argv[i][1] == 'f') flip = true;
        else if (argv[i][0] == '-' && argv[i][1] == 'l') image.gamma = 1.0f;
        else return 0;

    u8* components = loadBitmap(bitmap_file_path, image, flip);
    image.pixels = new Pixel[image.width * image.height];

    componentsToPixels(components, image, image.pixels);
    save(image, image_file_path);

    return 0;
}