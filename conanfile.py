from conan import ConanFile
from conan.tools.cmake import CMakeToolchain
from conan.tools.env import VirtualBuildEnv

class StudyCppConan(ConanFile):
    name = "study_cpp"
    version = "0.1"
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps"
    default_options = {
        "boost/*:without_cobalt": False,
    }

    def build_requirements(self):
        self.tool_requires("cmake/3.30.0")
        self.tool_requires("ninja/1.11.1")
        self.tool_requires("ccache/4.11")

    def requirements(self):
        self.requires("gtest/1.16.0")
        self.requires("boost/1.88.0")
        self.requires("benchmark/1.9.4")

    def generate(self):
        env = VirtualBuildEnv(self)
        env.generate()
        
        tc = CMakeToolchain(self)
        tc.generator = "Ninja"
        tc.variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.generate()
