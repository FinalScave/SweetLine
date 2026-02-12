// C++ sample code
#include <vector>
#include <map>

#define VERSION 2

#ifndef NDEBUG
#pragma once
#endif

namespace Graphics {

template<typename T, int N>
class Matrix {
public:
    explicit Matrix(T val = T()) : data_(N, val) {}

    T get(int i, T fallback = T()) const { return i < N ? data_[i] : fallback; }
    void set(int i, T val) { data_[i] = val; }

    template<typename U>
    Matrix<U, N> cast() const;

private:
    std::vector<T> data_;
};

enum class LogLevel : int { Debug, Info, Warning, Error };

} // namespace Graphics

class Renderer : public Graphics::Matrix<float, 4> {
public:
    void draw(Graphics::ShaderType type);
    std::vector<int> getIndices() const;
};

// namespace-qualified type usage
std::vector<int> indices = {1, 2, 3};
std::map<std::string, int> lookup;
std::shared_ptr<Renderer> renderer;
std::unique_ptr<Graphics::Vertex> vertex;

// smart pointer and template
auto createMatrix() -> std::shared_ptr<Graphics::Matrix<double, 4>> {
    return std::make_shared<Graphics::Matrix<double, 4>>(1.0);
}

void processData(std::vector<double> &data, const std::string &name) {
    for (auto &val : data) {
        val *= 2.0;
    }
}

// function parameter default value
void render(int width = 800, int height = 600, bool fullscreen = false);
void log(const std::string &msg, Graphics::LogLevel level = Graphics::LogLevel::Info);
void initBuffer(size_t size = 1024, unsigned char fill = 0xFF);

int main() {
    auto mat = createMatrix();
    mat->set(0, 3.14);
    std::cout << mat->get(0) << std::endl;

    int hex = 0xDEAD'BEEF;
    int bin = 0b1100'1010;
    bool ok = true;
    char ch = '\n';
    auto raw = R"(raw string content)";

    constexpr int MAX_VAL = 100;

    Graphics::ShaderType shader = Graphics::ShaderType::Vertex;
    std::vector<double> values = {1.0, 2.0, 3.0};

    if (ok && nullptr != mat) {
        for (auto i = 0; i < MAX_VAL; ++i) {
            if (i == 10) break;
        }
    }

    /* multi-line comment
       can span multiple lines */
    return false ? 1 : 0;
}
