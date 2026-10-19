from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout


class MatrixCompConan(ConanFile):
    name = "matrix_comp"
    version = "0.1.0"
    package_type = "application"

    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "CMakeLists.txt", "main.cpp", "include/*", "src/*"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        CMakeToolchain(self).generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
