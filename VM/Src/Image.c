#include "Image.h"
#include "VM.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Object MakeInt(int i)
{
    Object obj;
    obj.type = PrimType(INT);
    obj.i = i;
    return obj;
}

Object MakeArray(int size, Object *arr)
{
    Object obj;
    obj.type = PrimType(ARRAY);
    obj.p = AllocPointer(arr);
    arr[0] = MakeInt(size);
    return obj;
}

Object LoadImage(const char *path, int *width, int *height)
{
    int w, h, comp;
    stbi_uc *img = stbi_load(path, &w, &h, &comp, 0);
    if (!img)
    {
        printf("Error: could not open image '%s'\n", path);
        return MakeInt(-1);
    }

    int i, j, k;
    Object *pixels = (Object*)malloc(sizeof(Object) * (w * h * 4 + 1));
    for (i = 0; i < w * h * 4; i++)
        pixels[i] = MakeInt(img[i]);

    stbi_image_free(img);
    *width = w;
    *height = h;
    return MakeArray(w * h * 4, pixels);
}

void Image_Image(Object *stack, int *sp)
{
    Object *img_obj = &stack[(*sp)-2];
    Object path_obj = stack[(*sp)-1];
    char *path = path_obj.p->str;

    int width, height;
    Object *attrs = (Object*)malloc(sizeof(Object) * 3);
    attrs[0] = LoadImage(path, &width, &height);
    attrs[1] = MakeInt(width);
    attrs[2] = MakeInt(height);
    img_obj->p = AllocPointer(attrs);

    stack[(*sp)++] = MakeInt(0);
}

void Image_Pixels(Object *stack, int *sp)
{
    Object img_obj = stack[(*sp)-1];
    Object pixels = img_obj.p->attrs[0];
    stack[(*sp)++] = pixels;
}

void Image_Width(Object *stack, int *sp)
{
    Object img_obj = stack[(*sp)-1];
    Object width = img_obj.p->attrs[1];
    stack[(*sp)++] = width;
}

void Image_Height(Object *stack, int *sp)
{
    Object img_obj = stack[(*sp)-1];
    Object height = img_obj.p->attrs[2];
    stack[(*sp)++] = height;
}

void RegisterImage()
{
    RegisterFunc((char*)"Image:Image", Image_Image);
    RegisterFunc((char*)"Image:pixels", Image_Pixels);
    RegisterFunc((char*)"Image:width", Image_Width);
    RegisterFunc((char*)"Image:height", Image_Height);
}
