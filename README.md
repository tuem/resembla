# Resembla: Word-based Japanese similar sentence search library

## Features
- Candidate elimination using N-gram index and bit-parallel edit distance computation
- Word, kana and romaji-based edit distance variables and their ensemble
- Support vector regression with linguistic features

## Requirements
- [MeCab](http://taku910.github.io/mecab/)
- [LIBSVM](https://www.csie.ntu.edu.tw/~cjlin/libsvm/)
- [ICU](http://site.icu-project.org/)
- C++11 compiler

### Other included libraries
- [Catch](https://github.com/philsquared/Catch)
- [cmdline](https://github.com/tanakh/cmdline)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [SimString](https://github.com/chokkan/simstring)
- [paramset](https://github.com/tuem/paramset)

## Running example

- install MeCab, LIBSVM, ICU and a C++11 compiler
  - if you use CentOS 7, see [Wiki](../../wiki/Installation-on-CentOS-7)

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

- run with example files
```sh
# on src/executable
./resembla_index -c ../../example/conf/name.json
./resembla_cli -c ../../example/conf/name.json
# input some names like 'タケダ'
./resembla_index -c ../../example/conf/address.json
./resembla_cli -c ../../example/conf/address.json
# input some addresses like '京都北区'
# you may need to run install-unidic.sh or edit configuration file
./resembla_index -c ../../example/conf/apple.json
./resembla_cli -c ../../example/conf/apple.json
# input some sentences like 'りんごおいしくねえ'
```
