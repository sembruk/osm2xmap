# **Osm2xmap**

Converter [OpenSteerMap](http://www.openstreetmap.org) data to [OpenOrienteering Mapper](https://github.com/OpenOrienteering/mapper) format.

## Getting

See [**Downloads**](https://github.com/sembruk/osm2xmap/releases) or build from source

```bash
git clone https://github.com/sembruk/osm2xmap.git
cd osm2xmap
make
```

## Dependencies

* [Libroxml](http://www.libroxml.net/)
* [Proj.4](https://github.com/OSGeo/proj.4)
* [YAML-CPP-0.3](https://github.com/jbeder/yaml-cpp)

### Installing dependencies on Debian or Ubuntu

```bash
sudo sh -c "echo 'deb http://debian.libroxml.net/mirror/ debian contrib' > /etc/apt/sources.list.d/libroxml.list"
sudo apt-get update
sudo apt-get install libroxml0 libroxml-dev libproj0 libproj-dev libyaml-cpp0.3 libyaml-cpp0.3-dev
```

## Usage

```bash
./osm2xmap --help
Usage:
   ./osm2xmap [options]
   Options:
      -i filename - input OSM filename ('in.osm' as default);
      -o filename - output XMAP filename ('out.xmap' as default);
      -s filename - symbol set XMAP or OMAP filename ('symbols.xmap' as default)
                    (see /usr/share/openorienteering-mapper/symbol\ sets/);
      -r filename - YAML rules filename ('rules.yaml' as default);
      -v or --version - print software version;
      --help, -h or help - this usage.
```

## Todo

* Replace Libroxml
* Rules for other symbol sets
