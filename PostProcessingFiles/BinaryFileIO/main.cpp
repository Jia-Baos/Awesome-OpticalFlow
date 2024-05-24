#include <iostream>
#include <cstdio>
#include <cstdlib>

int main(int argc, char *argv[])
{
    const int width = 10;
    FILE *stream_write = fopen("demo.txt", "wb");
    if (stream_write == nullptr)
    {
        std::cout << "Failed to open files..." << std::endl;
    }

    std::cout << "malloc..." << std::endl;
    float *arr_write = (float *)malloc(sizeof(float) * width);
    for (int i = 0; i < width; i++)
    {
        *(arr_write + i) = i * i;
    }

    fwrite(arr_write, sizeof(float), width, stream_write);
    fclose(stream_write);
    free(arr_write);
    arr_write = nullptr;
    std::cout << "free..." << std::endl;

    FILE *stream_read = fopen("demo.txt", "rb");
    if (stream_read == nullptr)
    {
        std::cout << "Failed to open files..." << std::endl;
    }

    std::cout << "malloc..." << std::endl;
    float *arr_read = (float *)malloc(sizeof(float) * width);
    // rewind(stream_read);
    fread(arr_read, sizeof(float), width, stream_read);
    for (int i = 0; i < width; i++)
    {
        std::cout << *(arr_read + i) << std::endl;
    }
    fclose(stream_read);
    free(arr_read);
    arr_read = nullptr;
    std::cout << "free..." << std::endl;

    return 0;
}