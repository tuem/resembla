# Resembla: Word-based Japanese similar sentence search library

## Requirements
- [MeCab](http://taku910.github.io/mecab/)
- [ICU](http://site.icu-project.org/)

### Other included libraries
- [Catch](https://github.com/philsquared/Catch)
- [cmdline](https://github.com/tanakh/cmdline)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [SimString](https://github.com/chokkan/simstring)
- [paramset](https://github.com/tuem/paramset)

## Install on CentOS 7

- install g++ 5.x
  - http://linuxpitstop.com/install-and-use-red-hat-developer-toolset-4-1-on-centos-7/
```sh
sudo yum install centos-release-scl
sudo yum install devtoolset-4
scl enable devtoolset-4 bash
```

- prepare to build Resembla
```sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
sudo /sbin/ldconfig
```

- install MeCab
  - URL: http://qiita.com/ikenyal/items/275ca3096002822e8cd6
```sh
sudo rpm -ivh http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
sudo yum makecache
sudo yum install mecab mecab-ipadic mecab-devel
```

- install ICU
```sh
cd /var/tmp
wget 'https://downloads.sourceforge.net/project/icu/ICU4C/59.1/icu4c-59_1-src.tgz?r=http%3A%2F%2Fapps.icu-project.org%2Ficu-jsp%2FdownloadSection.jsp%3Fver%3D59.1%26base%3Dcs%26svn%3Drelease-59-1&ts=1497872621&use_mirror=jaist' -O icu4c-59_1-src.tgz
tar zxf icu4c-59_1-src.tgz
cd icu/source
./configure
make
sudo make install
sudo /sbin/ldconfig
```

- clone, build and install Resembla
```sh
cd /var/tmp
git clone https://github.com/tuem/resembla.git
cd resembla/src
make
sudo make install
cd executable
make
sudo make install
#optional
cd /var/tmp/resembla/misc/mecab_dic/unidic/
./install-unidic.sh
cd /var/tmp/resembla/misc/mecab_dic/mecab-unidic-neologd/
./install-mecab-unidic-neologd.sh
```

- run sample program
```sh
resembla_index /var/tmp/resembla/misc/sample/sample.tsv
resembla_cli /var/tmp/resembla/misc/sample/sample.tsv
# input some text like '東京都'
```
