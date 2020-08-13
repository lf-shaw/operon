let
  pkgs_stable = import <nixos> {};
  pkgs = import <nixos-unstable> {};
  cxxopts = import ./cxxopts.nix; 
  tracy = import ./tracy.nix;
  qcachegrind = pkgs.libsForQt5.callPackage ./qcachegrind.nix {};
  tbb = pkgs.tbb.overrideAttrs (old: rec {
    version = "2020_U3";

    src = pkgs.fetchFromGitHub {
      owner = "01org";
      repo = "tbb";
      rev = version;
      sha256 = "0r9axsdlmacjlcnax4vkzg86nwf8lsx7wbqdi3wnryaxk0xvdcx6";
    };

    installPhase = old.installPhase + ''
        ${pkgs.cmake}/bin/cmake \
            -DINSTALL_DIR="$out"/lib/cmake/TBB \
            -DSYSTEM_NAME=Linux -DTBB_VERSION_FILE="$out"/include/tbb/tbb_stddef.h \
            -P cmake/tbb_config_installer.cmake
    '';
  });
  eigen_trunk = pkgs.eigen.overrideAttrs (old: rec {
    stdenv = pkgs.gcc10Stdenv;
    src = pkgs.fetchgit {
      url             = "https://gitlab.com/libeigen/eigen.git";
      #rev             = "f566724023e1a82be7fecfe0639e908772d3cea6";
      #sha256          = "055x45z5nh14kh7vig8kl23mp3zmsm3c6924hnlghia6qpmm4vc1";
      rev             = "46f8a18567731925e06a7389a6c611e1dc420ea8";
      sha256          = "1j7jxyis1idykj3smzqy4i7zq9g9c1lil5rdnkdbikbbvljbq1ir";
      fetchSubmodules = false;
    };
    patches = [ ./eigen_include_dir.patch ];
    #patches = [ ./eigen_include_dir_old.patch ];
    #patches = [ ./eigen_include_dir_oldest.patch ];
  });
  pybind11_trunk = pkgs.python38Packages.pybind11.overrideAttrs (old: rec {
    stdenv = pkgs.gcc10Stdenv;
    buildInputs = [ eigen_trunk ];
    src = pkgs.fetchgit {
      url           = "https://github.com/pybind/pybind11.git";
      rev           = "3e448c0b5e3abcd179781dd718df2bd2340ddb06";
      sha256        = "15q9761xgg1p5z0xx47l9hh0qh2bzq6l6fyjivcym1rnl25qd43k";
      fetchSubmodules = false;
    };
    patches = [ ./pybind11_include.patch ./pybind11_cxx_standard.patch ];
  });
  ceres_trunk = pkgs.ceres-solver.overrideAttrs (old: rec {
    CFLAGS = (old.CFLAGS or "") + "-march=znver2 -O3";
    stdenv = pkgs.gcc10Stdenv;
    buildInputs = [ eigen_trunk pkgs.glog ];
    src = pkgs.fetchgit {
      url             = "https://github.com/ceres-solver/ceres-solver.git";
      #rev             = "242c703b501ffd64d645f4016d63c8b41c381038";
      #sha256          = "0ffgj18dhlgvq8y9gskw0ydl7jpk5z46vrcz59jwnqmi0lzjjrlf";
      rev             = "242c703b501ffd64d645f4016d63c8b41c381038";
      sha256          = "0ffgj18dhlgvq8y9gskw0ydl7jpk5z46vrcz59jwnqmi0lzjjrlf";
      fetchSubmodules = false;
    };
    enableParallelBuilding = true;
    cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" "-DCXX11=ON" "-DTBB=ON" "-DOPENMP=OFF" "-DBUILD_SHARED_LIBS=ON -DBUILD_EXAMPLES=FALSE -DBUILD_TESTING=FALSE" ];
  });
  fmt = pkgs.fmt.overrideAttrs(old: { outputs = [ "out" ]; });
  python_native = pkgs.python38.overrideAttrs (old: rec {
    CFLAGS = (old.CFLAGS or "") + "-march=znver2 -O3";
    stdenv = pkgs.gcc10Stdenv;
  });
in
#unstable.llvmPackages_10.stdenv.mkDerivation {
pkgs.gcc10Stdenv.mkDerivation {
    name = "operon-env";
    hardeningDisable = [ "all" ]; 

    buildInputs = with pkgs; [
        # python environment for bindings and scripting
        python_native
        pybind11_trunk
        (pkgs_stable.python38.withPackages (ps: with ps; [ pip numpy pandas pyperf colorama coloredlogs seaborn sphinx recommonmark sphinx_rtd_theme jupyterlab ]))
        # Project dependencies
        ccls # completion vim
        bear # generate compilation database
        # profiling and debugging
        gdb
        valgrind
        linuxPackages.perf
        #tracy
        #bloaty
        #heaptrack
        #hotspot
        git
        cmake
        tbb
        #eigen
        eigen_trunk
        ceres_trunk
        openlibm
        gperftools
        jemalloc
        fmt
        glog
        doctest
        llvm_10 # code generation
        clang_10
        # visualize profile results
        qcachegrind
        #massif-visualizer
        graphviz
        cxxopts
      ];
    }
