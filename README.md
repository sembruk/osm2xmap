# **Osm2xmap**

Converter from [OpenStreetMap](http://www.openstreetmap.org) data to [OpenOrienteering Mapper](https://github.com/OpenOrienteering/mapper) format.

## Getting

See [**Downloads**](https://github.com/sembruk/osm2xmap/releases) or build from source

```bash
git clone https://github.com/sembruk/osm2xmap.git
cd osm2xmap
make && make install
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

Download OSM data from http://openstreetmap.org/export.
You can also use [JOSM](https://josm.openstreetmap.de/) editor for download data.

Convert downloaded file with the following command

```bash
osm2xmap map.osm
```

For more options see `osm2xmap --help`.

You can open and edit resulting file in [OpenOrienteering Mapper](http://www.openorienteering.org/apps/mapper/) v0.6.6 or newer.

## Features

* Supported points, lines and multipolygons.
* Copies all OSM tags (press Ctrl+Shift+6 in OOM to see).
* You can configure converting rules (see `*_rules.yaml` file)

## Todo

* Replace Libroxml
* Rules for other symbol sets
