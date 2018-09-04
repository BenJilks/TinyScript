#include "TinyScript.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Object MakeInt(int i, VM vm)
{
    Object obj;
    obj.type = vm.PrimType(INT);
    obj.i = i;
    return obj;
}

Object MakeArray(int size, Object *arr, VM vm)
{
    Object obj;
    obj.type = vm.PrimType(ARRAY);
    obj.p = vm.AllocPointer(arr);
    arr[0] = MakeInt(size, vm);
    return obj;
}

Object LoadImage(const char *path, int *width, int *height, VM vm)
{
    int w, h, comp;
    stbi_uc *img = stbi_load(path, &w, &h, &comp, 0);
    if (!img)
    {
        printf("Error: could not open image '%s'\n", path);
        return MakeInt(-1, vm);
    }

    int i, j, k;
    Object *pixels = (Object*)malloc(sizeof(Object) * (w * h * 4 + 1));
    for (i = 0; i < w * h * 4; i++)
        pixels[i] = MakeInt(img[i], vm);

    stbi_image_free(img);
    *width = w;
    *height = h;
    return MakeArray(w * h * 4, pixels, vm);
}

Object Image_Image(Object *args, int arg_size, VM vm)
{
    Object *img_obj = &args[0];
    Object path_obj = args[1];
    char *path = path_obj.p->str;

    int width, height;
    Object *attrs = (Object*)malloc(sizeof(Object) * 3);
    attrs[0] = LoadImage(path, &width, &height, vm);
    attrs[1] = MakeInt(width, vm);
    attrs[2] = MakeInt(height, vm);
    img_obj->p = vm.AllocPointer(attrs);

    return MakeInt(0, vm);
}

Object Image_pixels(Object *args, int arg_size, VM vm)
{
    Object img_obj = args[0];
    Object pixels = img_obj.p->attrs[0];
    return pixels;
}

Object Image_width(Object *args, int arg_size, VM vm)
{
    Object img_obj = args[0];
    Object width = img_obj.p->attrs[1];
    return width;
}

Object Image_height(Object *args, int arg_size, VM vm)
{
    Object img_obj = args[0];
    Object height = img_obj.p->attrs[2];
    return height;
}
