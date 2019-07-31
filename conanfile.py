from conans import ConanFile, CMake


class UbitrackCoreConan(ConanFile):
    name = "ubitrack_visualization"
    version = "1.3.0"

    description = "Ubitrack Visualization Library and Components"
    url = "https://github.com/Ubitrack/utvisualization.git"
    license = "GPL"

    short_paths = True
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    options = {"shared": [True, False],
               "workspaceBuild" : [True, False],
               "enable_glfwconsole": [True, False],
               }
  

    default_options = {
        "shared":True,
        "workspaceBuild" : True,
        "enable_glfwconsole":True,
        }

    # all sources are deployed with the package
    exports_sources = "apps/*", "cmake/*", "components/*", "doc/*", "src/*", "CMakeLists.txt", "utvisualizationConfig.cmake"

    def configure(self):
        if self.options.shared:
            self.options['ubitrack_core'].shared = True
            self.options['ubitrack_vision'].shared = True
            self.options['ubitrack_dataflow'].shared = True

    def requirements(self):
        userChannel = "ubitrack/stable"
        if self.options.workspaceBuild:
            userChannel = "local/dev"

        self.requires("ubitrack_core/%s@%s" % (self.version, userChannel))
        self.requires("ubitrack_vision/%s@%s" % (self.version, userChannel))
        self.requires("ubitrack_dataflow/%s@%s" % (self.version, userChannel))

        if self.options.enable_glfwconsole:
            self.requires("glfw/3.2.1@camposs/stable")
            self.requires("ubitrack_facade/%s@%s" % (self.version, userChannel))
        


    # def imports(self):
    #     self.copy(pattern="*.dll", dst="bin", src="bin") # From bin to bin
    #     self.copy(pattern="*.dylib*", dst="lib", src="lib") 
       
    def build(self):
        cmake = CMake(self)
        cmake.definitions['BUILD_SHARED_LIBS'] = self.options.shared
        cmake.definitions['ENABLE_GLFWCONSOLE'] = self.options.enable_glfwconsole

        cmake.configure()
        cmake.build()
        cmake.install()

    def package(self):
        # self.copy("*.h", dst="include", src="src")
        # self.copy("*.lib", dst="lib", excludes="*/lib/ubitrack/*.lib", keep_path=False)
        # self.copy("*.dll", dst="bin", excludes="*/lib/ubitrack/*.dll", keep_path=False)
        # self.copy("*.dylib*", dst="lib", excludes="*/lib/ubitrack/*.dylib", keep_path=False)
        # self.copy("*.so", dst="lib", excludes="*/lib/ubitrack/*.so", keep_path=False)
        # self.copy("*.a", dst="lib", keep_path=False)
        # self.copy("*", dst="bin", src="bin", keep_path=False)

        # # components
        # self.copy("ubitrack/*.lib", dst="lib/ubitrack", keep_path=False)
        # self.copy("ubitrack/*.dll", dst="bin/ubitrack", keep_path=False)
        # self.copy("ubitrack/*.dylib*", dst="lib/ubitrack", keep_path=False)
        # self.copy("ubitrack/*.so", dst="lib/ubitrack", keep_path=False)
        pass

    def package_info(self):
        suffix = ""
        if self.settings.os == "Windows":
            suffix += self.version.replace(".", "")
            if self.settings.build_type == "Debug":
                suffix += "d"
        self.cpp_info.libs.append("utvisualization%s" % (suffix))


    def package_id(self):
        self.info.requires["ubitrack_vision"].full_package_mode()
