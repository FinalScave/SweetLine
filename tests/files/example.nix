# Nix sample file
# Nix package manager configuration

{ config, pkgs, lib, ... }:

let
  # Variable definitions
  myVariable = "hello";
  myNumber = 42;
  myList = [ 1 2 3 4 5 ];
  myAttrs = {
    key1 = "value1";
    key2 = "value2";
  };

  # Function definition
  myFunction = x: x * 2;
  
  # Function with multiple arguments
  add = a: b: a + b;
  
  # Function with attribute set argument
  greet = { name, age ? 30 }: "Hello, ${name}! You are ${toString age} years old.";
  
  # Recursive attribute set
  recursiveAttrs = rec {
    a = 1;
    b = a + 2;
    c = b + 3;
  };

  # Import expression
  importedConfig = import ./config.nix;
  
  # With expression
  nestedValue = with myAttrs; key1;

  # Let expression
  computedValue = let
    x = 10;
    y = 20;
  in x + y;

  # Conditional expression
  conditional = if myNumber > 40 then "big" else "small";

  # Assert expression
  asserted = assert myNumber > 0; "positive";

  # String interpolation
  interpolated = "The value is ${myVariable} and the number is ${toString myNumber}";

  # Multi-line string
  multiLine = ''
    This is a
    multi-line
    string
  '';

  # Path expressions
  relativePath = ./relative/path;
  absolutePath = /absolute/path;
  homePath = ~/.config/nix;

  # URL
  url = https://example.com/package.tar.gz;

  # List operations
  listLength = builtins.length myList;
  listHead = builtins.head myList;
  listTail = builtins.tail myList;
  
  # Map and filter
  mappedList = map (x: x * 2) myList;
  filteredList = builtins.filter (x: x > 2) myList;
  
  # Fold operation
  sum = builtins.foldl' (acc: x: acc + x) 0 myList;

  # Attribute set operations
  attrValues = builtins.attrValues myAttrs;
  attrNames = builtins.attrNames myAttrs;
  hasKey = builtins.hasAttr "key1" myAttrs;
  getKeyValue = builtins.getAttr "key1" myAttrs;

  # Derivation
  myPackage = derivation {
    name = "my-package";
    builder = "${pkgs.bash}/bin/bash";
    args = [ ./builder.sh ];
    system = builtins.currentSystem;
  };

in
{
  # Package configuration
  environment.systemPackages = with pkgs; [
    vim
    git
    curl
    wget
    htop
    tree
  ];

  # Service configuration
  services = {
    openssh = {
      enable = true;
      ports = [ 22 ];
      permitRootLogin = "no";
    };
    
    nginx = {
      enable = true;
      virtualHosts = {
        "example.com" = {
          root = "/var/www/example.com";
          index = "index.html";
        };
      };
    };
  };

  # User configuration
  users.users.myuser = {
    isNormalUser = true;
    home = "/home/myuser";
    shell = pkgs.zsh;
    extraGroups = [ "wheel" "networkmanager" "docker" ];
  };

  # File system configuration
  fileSystems = {
    "/" = {
      device = "/dev/sda1";
      fsType = "ext4";
    };
    
    "/home" = {
      device = "/dev/sda2";
      fsType = "ext4";
    };
  };

  # Networking configuration
  networking = {
    hostName = "my-machine";
    firewall = {
      enable = true;
      allowedTCPPorts = [ 80 443 22 ];
      allowedUDPPorts = [ 53 ];
    };
  };

  # Boot configuration
  boot = {
    loader = {
      systemd-boot = {
        enable = true;
        configurationLimit = 10;
      };
    };
    
    kernelModules = [ "kvm-intel" "snd-hda-intel" ];
    extraModulePackages = [ ];
  };

  # Nix configuration
  nix = {
    package = pkgs.nixFlakes;
    
    settings = {
      experimental-features = [ "nix-command" "flakes" ];
      auto-optimise-store = true;
    };
    
    gc = {
      automatic = true;
      dates = "weekly";
      options = "--delete-older-than 7d";
    };
  };

  # Environment variables
  environment.variables = {
    EDITOR = "vim";
    BROWSER = "firefox";
    TERMINAL = "alacritty";
  };

  # Programs configuration
  programs = {
    zsh = {
      enable = true;
      enableCompletion = true;
      autosuggestions.enable = true;
      syntaxHighlighting.enable = true;
    };
    
    git = {
      enable = true;
      lfs.enable = true;
    };
  };

  # System state version
  system.stateVersion = "23.11";
}
