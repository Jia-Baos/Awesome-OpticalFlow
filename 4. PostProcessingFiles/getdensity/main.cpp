#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <typeinfo>
#include <filesystem>

struct Point
{
    int x_;
    int y_;
    Point() : x_(0), y_(0) {}
    Point(const int x, const int y) : x_(x), y_(y) {}
};

int main(int argc, char *argv[])
{
    const int w = 1372;        // 图像的宽度
    const int h = 452;         // 图像的高度
    const int step = 11;       // 方格的间距
    const int radius = 11 / 2; // 种子点所在patch的半径
    const int gridw = w / step;
    const int gridh = h / step;
    const int ofsx = (w - (gridw - 1) * step) / 2;
    const int ofsy = (h - (gridh - 1) * step) / 2;
    const int seeds_num = gridw * gridh;
    std::vector<Point> seeds(seeds_num);

    for (size_t i = 0; i < seeds_num; i++)
    {
        const int x = i % gridw;
        const int y = i / gridw;

        // 保证 seed 不会出现在图像边缘上
        const float seedx = static_cast<float>(x * step + ofsx);
        const float seedy = static_cast<float>(y * step + ofsy);

        seeds[i] = Point(seedx, seedy);
    }

    int img_num = 0;
    double matches_avg = 0.0;
    double density_avg = 0.0;
    const std::filesystem::path root_path = "D:/DataSet/dataset2/matches-dm/";
    for (auto const &iter : std::filesystem::directory_iterator(root_path))
    {
        const std::string match_path = iter.path().string();
        std::cout << match_path << std::endl;

        std::ifstream infile;
        infile.open(match_path, std::ios::in);

        // 获取每一行的内容
        int x1 = 0;
        int y1 = 0;
        int x2 = 0;
        int y2 = 0;
        std::vector<Point> matches;
        std::string line_content;
        while (std::getline(infile, line_content))
        {
            // 将每一行的坐标按空格进行分割
            sscanf(line_content.c_str(), "%d %d %d %d/n", &x1, &y1, &x2, &y2);
            matches.emplace_back(Point(x1, y1));
        }
        infile.close();

        std::cout << "seeds size: " << seeds_num << std::endl;
        std::cout << "matches size: " << matches.size() << std::endl;

        // 计算Density
        std::vector<int> flags(seeds_num, 0);
        for (size_t i = 0; i < matches.size(); i++)
        {
            const Point match_curr = matches[i];
            for (size_t j = 0; j < seeds_num; j++)
            {
                Point seed_curr = seeds[j];
                if (match_curr.x_ >= seed_curr.x_ - radius &&
                    match_curr.x_ <= seed_curr.x_ + radius &&
                    match_curr.y_ >= seed_curr.y_ - radius &&
                    match_curr.y_ <= seed_curr.y_ + radius)
                {
                    flags[j] = 1;
                }
            }
        }

        const int patches_good = std::count(flags.begin(), flags.end(), 1);
        double density = static_cast<double>(patches_good) / seeds_num;
        std::cout << "matches desity: " << density << std::endl;

        matches_avg = matches_avg * img_num + matches.size();
        density_avg = density_avg * img_num + density;
        img_num += 1;
        matches_avg /= img_num;
        density_avg /= img_num;
    }

    std::cout << "matches avg: " << matches_avg << std::endl;
    std::cout << "density avg: " << density_avg << std::endl;
    return 0;
}