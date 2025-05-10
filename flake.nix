{
  description = "A very basic synthesizer";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }: {

    packages.x86_64-linux.hello = nixpkgs.legacyPackages.x86_64-linux.hello;

    packages.x86_64-linux.default = nixpkgs.legacyPackages.x86_64-linux.stdenv.mkDerivation {
      name = "synthesizer";
      src = ./.;
      nativeBuildInputs = with nixpkgs.legacyPackages.x86_64-linux; [
        pkg-config
      ];
      buildInputs = with nixpkgs.legacyPackages.x86_64-linux; [
        pipewire
      ];
      buildPhase = ''
        $CC main.c -o main $(pkg-config --cflags --libs libpipewire-0.3) -lm -lpthread
      '';
      installPhase = ''
        mkdir -p $out/bin
        cp main $out/bin/
      '';
    };

    # Add development shell with gcc and pipewire
    devShells.x86_64-linux.default = nixpkgs.legacyPackages.x86_64-linux.mkShell {
      nativeBuildInputs = with nixpkgs.legacyPackages.x86_64-linux; [
        pkg-config
      ];
      buildInputs = with nixpkgs.legacyPackages.x86_64-linux; [
        gcc
        pipewire
      ];
      shellHook = ''
        export PKG_CONFIG_PATH="${nixpkgs.legacyPackages.x86_64-linux.pipewire}/lib/pkgconfig:$PKG_CONFIG_PATH"
      '';
    };

    # Add run command
    apps.x86_64-linux.default = {
      type = "app";
      program = "${self.packages.x86_64-linux.default}/bin/main";
    };
  };
}