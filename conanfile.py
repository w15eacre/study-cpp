from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps

class StudyCppConan(ConanFile):
    name = "study_cpp"
    version = "0.1"
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps"
    requires = ["gtest/1.16.0"]
    tool_requires = ["cmake/3.30.0", "ninja/1.11.1", "ccache/4.11"]

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generator = "Ninja"
        tc.variables["CMAKE_C_COMPILER"] = "clang"
        tc.variables["CMAKE_CXX_COMPILER"] = "clang++"
        tc.variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.generate()
